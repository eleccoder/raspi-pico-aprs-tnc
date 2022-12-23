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

#include <stdbool.h>


int main()
{
  stdio_init_all();

  audio_buffer_pool_t* audio_buffer_pool = aprs_pico_init();

  // Let the altitude run over time
  double alt_in_m = 0.0;

  while (true) // Loop forever
    {
      // Send an APRS test message
      aprs_pico_sendAPRS(audio_buffer_pool,
                         "DL3TG",  // Source call sign
                         "DL3TG",  // Destination call sign
                         "PATH1",  // APRS path #1
                         "PATH2",  // APRS path #2
                         "APRS by RPi-Pico - https://github.com/eleccoder/raspi-pico-aprs-tnc", // Text message
                         10.0,     // Latitude  (in deg)
                         20.0,     // Longitude (in deg)
                         alt_in_m, // Altitude  (in m)
                         128u);    // Volume    (0 ... 256)

      // Don't raise too high ...
      alt_in_m = (alt_in_m < 1000.0) ? alt_in_m + 100.0 : 0.0;
    }

  return 0;
}
