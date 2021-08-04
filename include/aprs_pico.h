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

#include <pico/audio_pwm.h> // For 'audio_buffer_pool_t'


/** \brief Initializes the APRS Pico library
 *
 * \return A pool of audio buffers to be used for rendering any audio signal
 */
audio_buffer_pool_t* aprs_pico_init();


/** \brief Generates the analog AFSK signal for a given APRS message at GPIO-pin 'GP0'
 *
 * \param[in, out] audio_buffer_pool  The pool of audio buffers to be used for rendering the APRS audio signal
 * \param[in]      call_sign_src      The source      call sign
 * \param[in]      call_sign_dst      The destination call sign
 * \param[in]      aprs_path_1        The first  APRS path
 * \param[in]      aprs_path_2        The second APRS path
 * \param[in]      aprs_message       The APRS message text (may be 'NULL' pointer)
 * \param[in]      latitude_in_deg    The latitude  of the geo-location (in degrees)
 * \param[in]      longitude_in_deg   The longitude of the geo-location (in degrees)
 * \param[in]      altitude_in_m      The altitude  of the geo-location (in meters)
 * \param[in]      volume             The volume level of the generated AFSK signal (0 ... 256)
 *
 * \retval         'true'  - Successful operation
 * \retval         'false' - An error occurred
 *
 */
bool aprs_pico_sendAPRS(audio_buffer_pool_t* audio_buffer_pool,
                        const char*          call_sign_src,
                        const char*          call_sign_dst,
                        const char*          aprs_path_1,
                        const char*          aprs_path_2,
                        const char*          aprs_message,
                        double               latitude_in_deg,
                        double               longitude_in_deg,
                        double               altitude_in_m,
                        uint16_t             volume);


/** \brief Generates a 1 KHz sine wave signal at GPIO-pin 'GP0'
 *
 * \param[in, out] audio_buffer_pool  The pool of audio buffers to be used for rendering the sine audio signal
 * \param[in]      volume             The volume level of the generated AFSK signal (0 ... 256)
 */
void aprs_pico_send1kHz(audio_buffer_pool_t* audio_buffer_pool, uint16_t volume);

#endif // APRS_PICO_H
