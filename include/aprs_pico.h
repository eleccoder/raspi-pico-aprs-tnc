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

#ifndef APRS_PICO_H
#define APRS_PICO_H

#include <stdint.h>
#include <stdbool.h>

// WARNING: ATTOW, the pico audio PWM lib worked only @ 22050 Hz sampling frequency and 48 MHz system clock
//          This is documented here: https://github.com/raspberrypi/pico-extras
#define APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB_FIXED_SAMPLE_FREQ_IN_HZ  (22050)


void sendAPRS(const char*   call_sign_src,
              const char*   call_sign_dst,
              const char*   aprs_path_1,
              const char*   aprs_path_2,
              const char*   aprs_message,
              const double  latitude_in_deg,
              const double  longitude_in_deg,
              const double  altitude_in_m,
              const uint8_t volume,
              const bool    is_loop);


void send1kHz(unsigned int sample_freq_in_hz, uint8_t volume);

#endif // APRS_PICO_H
