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
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <aprs_pico.h>
#include <pico/stdlib.h>

#define SINE_WAVE_TEST (0) // For testing & debugging



int main()
{
  stdio_init_all();

  audio_buffer_pool_t* audio_buffer_pool = aprs_pico_init();

#if (SINE_WAVE_TEST == 1)

  const unsigned int FREQ_IN_HZ = 1000u;
  const uint16_t     VOLUME     = 128u;

  aprs_pico_send_sine_wave(audio_buffer_pool, FREQ_IN_HZ, VOLUME);

#else // !SINE_WAVE_TEST

  while (true) // Loop forever
    {
      // Send an APRS test message
      aprs_pico_sendAPRS(audio_buffer_pool,
                         "SRC",          // Source call sign
                         "DST",          // Destination call sign
                         "PATH1",        // APRS path #1
                         "PATH2",        // APRS path #2
                         "Test message", // APRS message
                         10.0,           // Latitude  (in deg)
                         20.0,           // Longitude (in deg)
                         100.0,          // Altitude  (in m)
                         128u);          // Volume    (0 ... 256)
    }

#endif // SINE_WAVE_TEST, !SINE_WAVE_TEST

  return 0;
}
