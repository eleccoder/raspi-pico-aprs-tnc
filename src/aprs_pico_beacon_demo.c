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


/* This program demonstrates the usage of the 'libaprs_pico.a' library by showing
 * how to send a static APRS beacon.
 *
 * Optionally, PTT control can be enabled (see the #define section down below).
 */

// Define whether/how the RPi Pico should control a transmitter's PTT input
#define PTT_ENABLE                   (false)
#define PTT_GPXX_PIN                 (1)
#define PTT_DELAY_BEFORE_TX_IN_MSEC  (400)
#define PTT_TX_PERIOD_IN_MIN         (10)


#include "aprs_pico.h"
#include "pico/stdlib.h"

#include <stdbool.h>



int main()
{
  stdio_init_all();

#if PTT_ENABLE == true

  gpio_init(PTT_GPXX_PIN);
  gpio_set_dir(PTT_GPXX_PIN, GPIO_OUT);

#endif // PTT_ENABLE

  audio_buffer_pool_t* audio_buffer_pool = aprs_pico_init();

  // Loop forever
  while (true)
    {
#if PTT_ENABLE == true

      gpio_put(PTT_GPXX_PIN, true);
      sleep_ms(PTT_DELAY_BEFORE_TX_IN_MSEC);

#endif // PTT_ENABLE

      // Send an APRS test message
      aprs_pico_sendAPRS(audio_buffer_pool,
                         "DL3TG-9",  // Source call sign
                         "APPIPI",   // Destination call sign
                         "WIDE1-1",  // APRS path #1
                         "WIDE2-2",  // APRS path #2
                         "APRS by RPi-Pico - https://github.com/eleccoder/raspi-pico-aprs-tnc", // Text message
                         48.75588,   // Latitude  (in deg)
                         9.19011,    // Longitude (in deg)
                         483,        // Altitude  (in m)
                         '/',        // APRS symbol table: Primary
                         '>',        // APRS symbol code:  Car
                         128u);      // Volume    (0 ... 256)

#if PTT_ENABLE == true

      gpio_put(PTT_GPXX_PIN, false);
      sleep_ms(PTT_TX_PERIOD_IN_MIN * 60 * 1000);

#endif // PTT_ENABLE
    }

  return 0;
}
