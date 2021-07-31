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

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include <pico/stdlib.h>
#include <pico/audio_pwm.h>

#include <ax25beacon.h>


// Do not change: ATTOW, the pico audio lib worked only @ 22050 Hz sampling frequency
#define SAMPLE_FREQ_IN_HZ  (22050)
#define SEND_LOOP_DEBUG    (1) // For debug



audio_buffer_pool_t* init_audio(uint sample_freq_in_hz, uint16_t audio_buffer_format)
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


static void init(uint sample_freq_in_hz)
{
  // NOTE: ATTOW, the pico audio lib worked only @ 22050 Hz sampling frequency,
  //       related to 48 MHz system clock

  const uint SAMPLING_FREQ_REF_IN_HZ = 22050;
  const uint SYSTEM_CLOCK_REF_IN_KHZ = 48000;

  if (sample_freq_in_hz == SAMPLING_FREQ_REF_IN_HZ)
    {
      set_sys_clock_48mhz();
    }
  else
    {
      // Compensate a non-reference sampling frequency by a different system clock frequency
      // To be improved: System clock may be imprecise

      // set_sys_clock_khz((uint32_t)((float)SYSTEM_CLOCK_REF_IN_KHZ * (float)sample_freq_in_hz / (float)SAMPLING_FREQ_REF_IN_HZ), false);
      set_sys_clock_khz(105000, false); // HACK
    }

  stdio_init_all();
}


static void fill_audio_buffer(audio_buffer_pool_t* audio_pool, const int16_t* pcm_data, uint num_samples)
{
  uint          pos    = 0u;
  const uint8_t volume = 128u;

  while (true)
    {
      audio_buffer_t* buffer = take_audio_buffer(audio_pool, true);
      int16_t*       samples = (int16_t*)buffer->buffer->bytes;

      for (uint i = 0u; i < buffer->max_sample_count; i++)
        {
          samples[i] = (volume * pcm_data[pos]) >> 8u;
          pos++;

          if (pos == num_samples)
            {
              pos = 0u;
            }
        }

      buffer->sample_count = buffer->max_sample_count;
      give_audio_buffer(audio_pool, buffer);
    }
}


// Test tone: 1 kHz sine wave
static void send1kHz(uint sample_freq_in_hz)
{
  init(sample_freq_in_hz);

  const uint tone_freq_in_hz = 1000u;
  const uint num_samples     = sample_freq_in_hz / tone_freq_in_hz;

  int16_t* sine_wave_table = malloc(num_samples * sizeof(int16_t));

  if (!sine_wave_table)
    {
      panic("Out of memory: malloc() failed.\n");
    }

  for (uint i = 0u; i < num_samples; i++)
    {
      sine_wave_table[i] = 32767 * sinf(2.0f * (float)M_PI * (float)i / (float)num_samples);
    }

  audio_buffer_pool_t* audio_pool = init_audio(sample_freq_in_hz, AUDIO_BUFFER_FORMAT_PCM_S16);

  while (true)
    {
      fill_audio_buffer(audio_pool, sine_wave_table, num_samples);
    }

  free(sine_wave_table);
}


static void sendAPRS_audioCallback(void* user_data, int16_t* pcm_data, size_t num_samples)
{
  const uint aprs_sampl_freq_in_hz = *((const uint*)user_data);

  audio_buffer_pool_t* audio_pool = init_audio(aprs_sampl_freq_in_hz, AUDIO_BUFFER_FORMAT_PCM_S16);

#if (SEND_LOOP_DEBUG == 1)
  while (true)
    {
#endif // SEND_LOOP_DEBUG

      fill_audio_buffer(audio_pool, pcm_data, num_samples);

#if (SEND_LOOP_DEBUG == 1)
    }
#endif // SEND_LOOP_DEBUG
}


static void sendAPRS(const char* call_sign_src,
                     const char* call_sign_dst,
                     const char* aprs_path_1,
                     const char* aprs_path_2,
                     const char* aprs_message,
                     const double latitude_in_deg,
                     const double longitude_in_deg,
                     const double altitude_in_m)
{
  static const uint APRS_SAMPL_FREQ_IN_HZ = 48000u;

  init(APRS_SAMPL_FREQ_IN_HZ);

  ax25_beacon((void*)&APRS_SAMPL_FREQ_IN_HZ, // User data
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


int main()
{
  // send1kHz(SAMPLE_FREQ_IN_HZ); // For test & debug

  // Send an APRS test message
  sendAPRS("SRC",  // Src call sign
           "DST",  // Dst call sign
           "PATH1",
           "PATH2",
           "Test message",
           10.0,   // Lat in deg
           20.0,   // Long in deg
           100.0   // Alt in m
  );

  return 0;
}
