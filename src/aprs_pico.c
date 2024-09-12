/*
* Project 'raspi-pico-aprs-tnc'
* Copyright (C) 2021-2023 Thomas Glau, DL3TG
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
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "aprs_pico.h"

#include "ax25beacon.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include <math.h>


// NOTE: ATTOW, the pico-extra audio PWM lib worked only at a fixed 22050 Hz sampling frequency, while the
//       system clock runs at 48 MHz. This is documented here: https://github.com/raspberrypi/pico-extras
#define APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB__FIXED_SAMPLE_FREQ_IN_HZ  (22050)
#define APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB__SYS_CLOCK_FREQ_OF_IN_MHZ (48)


typedef struct AudioCallBackUserData
{
  audio_buffer_pool_t* audio_buffer_pool;      // The pool of audio buffers to be used for rendering an audio signal
  uint16_t             volume;                 // Valid range: 0 ... 256

} AudioCallBackUserData_t;


/** \brief Init function for the Pico audio PWM library
 *
 * \param[in] sample_freq_in_hz    The sampling frequency to be used for audio signals
 * \param[in] audio_buffer_format  The format of the audio buffers to be created, representing
 *                                 the data format of the audio samples
 *
 * \return                         A pool of audio buffers to be used for rendering any audio signal
 */
static audio_buffer_pool_t* aprs_pico_initAudio(unsigned int sample_freq_in_hz, uint16_t audio_buffer_format)
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


/** \brief Init function for the Pico's clock system
 *
 * \param[in] sample_freq_in_hz  The sampling frequency to be used for rendering audio signals
 */
static void aprs_pico_initClock(unsigned int sample_freq_in_hz)
{
  // NOTE: ATTOW, the pico-extra audio PWM lib worked only at a fixed 22050 Hz sampling frequency, while the
  //       system clock runs at 48 MHz. This is documented here: https://github.com/raspberrypi/pico-extras

  // Compensate a non-'APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB__FIXED_SAMPLE_FREQ_IN_HZ' sampling frequency
  // by adapting the system clock frequency accordingly.

  float sys_clock_in_mhz = (float)APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB__SYS_CLOCK_FREQ_OF_IN_MHZ *
                           ((float)sample_freq_in_hz / (float)APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB__FIXED_SAMPLE_FREQ_IN_HZ);

  if (!set_sys_clock_khz((uint32_t)(1000.0f * sys_clock_in_mhz), false))
    {
      // Round to full MHz to increase the chance that 'set_sys_clock_khz()' can exactly attain this frequency
      sys_clock_in_mhz = round(sys_clock_in_mhz);

      // With the second parameter set 'true', the function will assert if the frequency is not attainable
      set_sys_clock_khz(1000u * (uint32_t)sys_clock_in_mhz, true);
    }
}


/** \brief Renders given PCM audio samples
 *
 * \param[in, out] audio_buffer_pool  The pool of audio buffers to be used for rendering any audio signal
 * \param[in]      pcm_data           The PCM audio samples to be rendered
 * \param[in]      num_samples        The number of PCM audio samples to be rendered
 * \param[in]      volume             The volume level of the generated AFSK signal (0 ... 256)
 * \param[in]      num_loops          For 'num_loops' >= 0  rendering of the audio samples will be repeated 'num_loops' times
 *                                    For 'num_loops'  < 0, rendering of the audio samples will be endlessly repeated
 */
static void aprs_pico_renderAudioSamples(audio_buffer_pool_t* audio_buffer_pool, const int16_t* pcm_data,
                                         unsigned int num_samples, uint16_t volume, int num_loops)
{
  assert(audio_buffer_pool != NULL);
  assert(pcm_data          != NULL);

  bool do_loop_forever          = num_loops < 0;
  bool is_all_samples_processed = (num_samples == 0u) || (num_loops == 0);

  unsigned int idx_src = 0u;

  // Write the PCM sample data into the next audio buffer while applying the 'volume' value

  // Loop over audio buffers
  while (!is_all_samples_processed)
    {
      audio_buffer_t* audio_buffer          = take_audio_buffer(audio_buffer_pool, true);
      int16_t*        audio_buffer_pcm_data = (int16_t*)audio_buffer->buffer->bytes;

      unsigned int idx_dst = 0u;

      // Fill the current audio buffer
      while (!is_all_samples_processed && (idx_dst < audio_buffer->max_sample_count))
        {
          audio_buffer_pcm_data[idx_dst] = ((int32_t)volume * (int32_t)pcm_data[idx_src]) >> 8u;

          idx_src++;
          idx_dst++;

          if (idx_src == num_samples)
            {
              if (!do_loop_forever)
                {
                  num_loops--;
                }

              if (num_loops == 0)
                {
                  is_all_samples_processed = true;
                }
              else
                {
                  idx_src = 0u;
                }
            }
        }

      assert(idx_src <= num_samples);
      assert(idx_dst <= audio_buffer->max_sample_count);

      audio_buffer->sample_count = idx_dst;
      give_audio_buffer(audio_buffer_pool, audio_buffer);
    }
}


/** \brief The callback function to render the generated PCM audio samples of an APRS message
 *
 * \param[in] callback_user_data  User data provided by the caller function of this callback
 * \param[in] pcm_data            The PCM audio samples to be rendered
 * \param[in] num_samples         The number of samples the PCM data consist of
 * \param[in] sample_freq_in_hz   The sample frequency of the PCM data (in Hz)
 */
static void aprs_pico_sendAPRSAudioCallback(const void* callback_user_data, const int16_t* pcm_data, size_t num_samples, uint16_t sample_freq_in_hz)
{
  assert(callback_user_data != NULL);
  assert(pcm_data           != NULL);

  const AudioCallBackUserData_t user_data = *((AudioCallBackUserData_t*)callback_user_data);

  aprs_pico_initClock(sample_freq_in_hz);
  aprs_pico_renderAudioSamples(user_data.audio_buffer_pool, pcm_data, num_samples, user_data.volume, 1);
}


// See the header file for documentation
audio_buffer_pool_t* aprs_pico_init()
{
  audio_buffer_pool_t* audio_buffer_pool = aprs_pico_initAudio(APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB__FIXED_SAMPLE_FREQ_IN_HZ,
                                                               AUDIO_BUFFER_FORMAT_PCM_S16);
  return audio_buffer_pool;
}


// See the header file for documentation
void aprs_pico_play_sine_wave(audio_buffer_pool_t* audio_buffer_pool, unsigned int freq_in_hz,
                              unsigned int sample_freq_in_hz, uint16_t volume, int duration_in_ms)
{
  assert(audio_buffer_pool != NULL);

  typedef int16_t wave_table_value_t;
  const wave_table_value_t WAVE_TABLE_VALUE_MAX = INT16_MAX;


  aprs_pico_initClock(sample_freq_in_hz);

  const unsigned int num_samples_per_period = sample_freq_in_hz / freq_in_hz;

  wave_table_value_t* sine_period_wave_table = malloc(num_samples_per_period * sizeof(wave_table_value_t));

  if (sine_period_wave_table == NULL)
    {
      panic("Out of memory: malloc() failed.\n");
    }

  for (unsigned int i = 0u; i < num_samples_per_period; i++)
    {
      sine_period_wave_table[i] = (wave_table_value_t)((float)WAVE_TABLE_VALUE_MAX * sinf(2.0f * (float)M_PI * (float)i / (float)num_samples_per_period));
    }

  const int num_loops = duration_in_ms < 0 ? -1 : (duration_in_ms * (int)freq_in_hz / 1000);

  aprs_pico_renderAudioSamples(audio_buffer_pool, sine_period_wave_table, num_samples_per_period, volume, num_loops);

  free(sine_period_wave_table);
}


// See the header file for documentation
bool aprs_pico_sendAPRS(audio_buffer_pool_t* audio_buffer_pool,
                        const char*          call_sign_src,
                        const char*          call_sign_dst,
                        const char*          aprs_path_1,
                        const char*          aprs_path_2,
                        const char*          aprs_message,
                        double               latitude_in_deg,
                        double               longitude_in_deg,
                        double               altitude_in_m,
                        char                 sym_table,
                        char                 sym_code,
                        uint16_t             volume)
{
  // NOTE: 'aprs_message' is allowed to be 'NULL'
  assert(audio_buffer_pool != NULL);
  assert(call_sign_src     != NULL);
  assert(call_sign_dst     != NULL);
  assert(aprs_path_1       != NULL);
  assert(aprs_path_2       != NULL);

  static AudioCallBackUserData_t callback_user_data;

  callback_user_data.audio_buffer_pool = audio_buffer_pool;
  callback_user_data.volume            = volume;

  int ret_val = ax25_beacon((void*)&callback_user_data,
                            aprs_pico_sendAPRSAudioCallback,
                            call_sign_src,
                            call_sign_dst,
                            aprs_path_1,
                            aprs_path_2,
                            latitude_in_deg,
                            longitude_in_deg,
                            altitude_in_m,
                            aprs_message,
                            sym_table,
                            sym_code);

  return ret_val == AX25_OK;
}
