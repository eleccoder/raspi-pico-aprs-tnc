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

#define SINE_WAVE_TEST (0) // For testing & debugging


int main()
{
#if (SINE_WAVE_TEST == 1)

  const uint8_t VOLUME = 128u;
  send1kHz(APRS_PICO__PICO_EXTRA_AUDIO_PWM_LIB_FIXED_SAMPLE_FREQ_IN_HZ, VOLUME);

#else // !SINE_WAVE_TEST

  // Send an APRS test message
  sendAPRS("SRC",  // Src call sign
           "DST",  // Dst call sign
           "PATH1",
           "PATH2",
           "Test message",
           10.0,   // Latitude in deg
           20.0,   // Longitude in deg
           100.0,  // Altitude in m
           128u,   // Volume (0 ... 255)
           false); // Loop forever

#endif // SINE_WAVE_TEST, !SINE_WAVE_TEST

  return 0;
}
