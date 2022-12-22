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
#include "pico/stdlib.h"


int main()
{
  // 48 kHz is used as a default by the underlying https://github.com/eleccoder/ax25-aprs-lib library
  // The PWM base frequency is expected to be 16 * SAMPLE_FREQ_IN_HZ
  const unsigned int SAMPLE_FREQ_IN_HZ = 48000u;

  const unsigned int TONE_FREQ_IN_HZ   = 1000u;
  const uint16_t     VOLUME            = 128u;   // 0 ... 256

  stdio_init_all();
  audio_buffer_pool_t* audio_buffer_pool = aprs_pico_init();

  aprs_pico_send_sine_wave(audio_buffer_pool, TONE_FREQ_IN_HZ, SAMPLE_FREQ_IN_HZ, VOLUME);

  return 0;
}
