/*
* Project 'raspi-pico-aprs-tnc'
* Copyright (C) 2021 Thomas Glau, DL3TG
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <aprs_pico.h>

#include <stdio.h>
#include <math.h>

#include <pico/stdlib.h>
#include <pico/audio_pwm.h>

#include <ax25beacon.h>


typedef struct AudioCallBackUserData
{
  unsigned int aprs_sample_freq_in_hz;
  bool         is_loop_forever;
  uint16_t     volume; // Valid range: 0 ... 256

} AudioCallBackUserData_t;


/** \brief Init function for the Pico audio PWM library
 *
 * \param sample_freq_in_hz   The sampling frequency to be used for audio signals
 * \param audio_buffer_format The format of the audio buffers to be created, representing
 *                            the data format of the audio samples
 *
 * \return                    A pool of audio buffers to be used for rendering an audio signal
 */
static audio_buffer_pool_t* init_audio(unsigned int sample_freq_in_hz, uint16_t audio_buffer_format)
{
  const int NUM_AUDIO_BUFFERS  = 3;
  const int SAMPLES_PER_BUFFER = 256;

  const audio_format_t audio_format = {.format        = audio_buffer_format,
                                       .sample_freq   = sample_freq_in_hz,
                                       .channel_count = 1};

  audio_buffer_format_t producer_format = {.format        = &audio_format,
                                           .sample_stride = 2};

  audio_buffer_pool_t* producer_pool = audio_new_producer_pool(&producer_format, NUM_AUDIO_BUFFERS, SAMPLES_PER_BUFFER);

  if (!audio_pwm_setup(&audio_format, -1, &default_mono_channel_config))
    {
      panic("PicoAudio: Unable to open audio device.\n");
    }

  bool __unused is_ok = audio_pwm_default_connect(producer_pool, false);
  assert(is_ok);

  audio_pwm_set_enabled(true);

  return producer_pool;
}


/** \brief Init function for the Pico's clock system and the Pico's standard library
 *
 * \param sample_freq_in_hz  The sampling frequency to be used for rendering audio signals
 */
static void init_system(unsigned int sample_freq_in_hz)
{
  // WARNING: ATTOW, the pico audio PWM lib worked only @ 22050 Hz sampling frequency and 48 MHz system clock
  //          This is documented here: https://github.com/raspberrypi/pico-extras

  if (sample_freq_in_hz == APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB_FIXED_SAMPLE_FREQ_IN_HZ)
    {
      // This is the safe case, see the comment above
      set_sys_clock_48mhz();
    }
  else
    {
      // Compensate a non-'PICO_EXTRA_AUDIO_PWM_LIB_FIXED_SAMPLE_FREQ_IN_HZ' sampling frequency
      // by a adapting the system clock accordingly

      const float SYS_CLOCK_FREQ_OF_PICO_EXTRA_AUDIO_PWM_LIB_IN_MHZ = 48.0f;

      float sys_clock_in_mhz = SYS_CLOCK_FREQ_OF_PICO_EXTRA_AUDIO_PWM_LIB_IN_MHZ * (float)sample_freq_in_hz /
                               (float)APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB_FIXED_SAMPLE_FREQ_IN_HZ;

      // Round to full Mhz to increase the chance that 'set_sys_clock_khz()' can exactly realize this frequency
      sys_clock_in_mhz = round(sys_clock_in_mhz);

      set_sys_clock_khz(1000u * (uint32_t)sys_clock_in_mhz, false);
    }

  stdio_init_all();
}


/** \brief Renders given PCM audio samples
 *
 * \param audio_pool       The pool of audio buffers to be used for rendering an audio signal
 * \param pcm_data         The PCM audio samples to be rendered
 * \param num_samples      The number of samples to be rendered
 * \param volume           The volume level of the generated AFSK signal (0 ... 256)
 * \param is_loop_forever  If 'true', the rendering of the audio samples will be continuously repeated
 */
static void render_audio_samples(audio_buffer_pool_t* audio_pool, const int16_t* pcm_data,
                                 unsigned int num_samples, uint16_t volume, bool is_loop_forever)
{
  unsigned int pos   = 0u;
  bool is_keep_going = true;

  while (is_keep_going)
    {
      audio_buffer_t* buffer  = take_audio_buffer(audio_pool, true);
      int16_t*        samples = (int16_t*)buffer->buffer->bytes;

      for (unsigned int i = 0u; i < buffer->max_sample_count; i++)
        {
          samples[i] = ((int32_t)volume * (int32_t)pcm_data[pos]) >> 8u;
          pos++;

          if (pos == num_samples)
            {
              if (is_loop_forever)
                {
                  pos = 0u;
                }
              else
                {
                  is_keep_going = false;
                  break;
                }
            }
        }

      buffer->sample_count = buffer->max_sample_count;
      give_audio_buffer(audio_pool, buffer);
    }
}


/** \brief The callback function to render the generated PCM audio samples of an APRS message
 *
 * \param callback_user_data  User data provided by the caller function of this callback
 * \param pcm_data            The PCM audio samples to be rendered
 * \param num_samples         The number of samples the PCM data consist of
 */
static void sendAPRS_audioCallback(void* callback_user_data, int16_t* pcm_data, size_t num_samples)
{
  const AudioCallBackUserData_t user_data = *((const AudioCallBackUserData_t*)callback_user_data);

  audio_buffer_pool_t* audio_pool = init_audio(user_data.aprs_sample_freq_in_hz, AUDIO_BUFFER_FORMAT_PCM_S16);

  render_audio_samples(audio_pool, pcm_data, num_samples, user_data.volume, user_data.is_loop_forever);
}


// See the header file for documentation
void send1kHz( unsigned int sample_freq_in_hz, uint16_t volume)
{
  init_system(sample_freq_in_hz);

  const unsigned int TONE_FREQ_IN_HZ = 1000u;
  const unsigned int num_samples     = sample_freq_in_hz / TONE_FREQ_IN_HZ;

  int16_t* sine_wave_table = malloc(num_samples * sizeof(int16_t));

  if (!sine_wave_table)
    {
      panic("Out of memory: malloc() failed.\n");
    }

  for (unsigned int i = 0u; i < num_samples; i++)
    {
      sine_wave_table[i] = (int16_t)(32767.0f * sinf(2.0f * (float)M_PI * (float)i / (float)num_samples));
    }

  audio_buffer_pool_t* audio_pool = init_audio(sample_freq_in_hz, AUDIO_BUFFER_FORMAT_PCM_S16);

  render_audio_samples(audio_pool, sine_wave_table, num_samples, volume, true);

  free(sine_wave_table);
}


// See the header file for documentation
void sendAPRS(const char*    call_sign_src,
              const char*    call_sign_dst,
              const char*    aprs_path_1,
              const char*    aprs_path_2,
              const char*    aprs_message,
              const double   latitude_in_deg,
              const double   longitude_in_deg,
              const double   altitude_in_m,
              const uint16_t volume,
              const bool     is_loop_forever)
{
  static AudioCallBackUserData_t callback_user_data;

  callback_user_data.aprs_sample_freq_in_hz = 48000u; // Known from the 'ax25beacon' library
  callback_user_data.is_loop_forever        = is_loop_forever;
  callback_user_data.volume                 = volume;

  init_system(callback_user_data.aprs_sample_freq_in_hz);

  ax25_beacon((void*)&callback_user_data,
              sendAPRS_audioCallback,
              call_sign_src,
              call_sign_dst,
              aprs_path_1,
              aprs_path_2,
              latitude_in_deg,
              longitude_in_deg,
              altitude_in_m,
              aprs_message,
              '/', 'O');
}
