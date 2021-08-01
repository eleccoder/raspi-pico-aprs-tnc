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


/** \brief Generates the analog AFSK signal for a given APRS message at GPIO-pin 'GP0'
 *
 * \param call_sign_src     The source      call sign
 * \param call_sign_dst     The destination call sign
 * \param aprs_path_1       The first  APRS path
 * \param aprs_path_2       The second APRS path
 * \param aprs_message      The APRS message text
 * \param latitude_in_deg   The latitude  of the geo-location (in degrees)
 * \param longitude_in_deg  The longitude of the geo-location (in degrees)
 * \param altitude_in_m     The altitude  of the geo-location (in meters)
 * \param volume            The volume level of the generated AFSK signal (0 ... 255)
 * \param is_loop_forever   If 'true', the transmission of the signal will be continuously repeated
 */
void sendAPRS(const char*   call_sign_src,
              const char*   call_sign_dst,
              const char*   aprs_path_1,
              const char*   aprs_path_2,
              const char*   aprs_message,
              const double  latitude_in_deg,
              const double  longitude_in_deg,
              const double  altitude_in_m,
              const uint8_t volume,
              const bool    is_loop_forever);


/** \brief Generates a 1 KHz sine wave signal at GPIO-pin 'GP0'
 *
 * \param sample_freq_in_hz  The sampling frequency to be used for the audio signal
 * \param volume             The volume level of the generated AFSK signal (0 ... 255)
 *
 * \warning ATTOW, use 'APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB_FIXED_SAMPLE_FREQ_IN_HZ' as
 *          the only value for the 'sample_freq_in_hz' parameter
 */
void send1kHz(unsigned int sample_freq_in_hz, uint8_t volume);

#endif // APRS_PICO_H
