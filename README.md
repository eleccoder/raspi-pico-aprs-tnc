<img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/eleccoder/raspi-pico-aprs-tnc">

# raspi-pico-aprs-tnc
A TX-only [TNC](https://en.wikipedia.org/wiki/Terminal_node_controller) (Terminal Node Controller) to generate the [AFSK](https://en.wikipedia.org/wiki/Frequency-shift_keying#Audio_FSK) (Audio Frequency-Shift Keying) audio tones for [APRS](https://en.wikipedia.org/wiki/Automatic_Packet_Reporting_System) (Automatic Packet Reporting System) messages using a [RP2040](https://en.wikipedia.org/wiki/RP2040) microcontroller on a [Raspberry Pi Pico](https://en.wikipedia.org/wiki/Raspberry_Pi) board.

![Signal Flow](https://github.com/eleccoder/raspi-pico-aprs-tnc/blob/main/doc/img/signal_flow.png)

Block diagram showing the signal flow

An analog line-out audio signal will be generated by a band-pass filter connected to GPIO-pin 'GP0' which provides the binary PWM signal. You can probe it by a scope, listen to it by using an audio amp, or connect it to any RF transceiver to send it on the air (ham radio license required).

![AFSK scope screenshot](https://github.com/eleccoder/raspi-pico-aprs-tnc/blob/main/doc/img/afsk_scope.png)

Image: Line-out signal (see [below](#Hardware)) probed by a DSO. We clearly see the 1200 Hz and 2200 Hz tones of the 1200 Bd 2-AFSK.

Both a static library `libaprs_pico.a` and an example application will be generated by the build.


## Prerequisites

Your host platform for cross-compilation is assumed to be LINUX.

1. Install the Pico-SDK following the instructions given in the [Raspberry Pi 'Getting Started' Guide](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) (pdf)
1. Set the `PICO_SDK_PATH` environment variable to point to your Pico-SDK installation directory
1. Install the [pico-extras](https://github.com/raspberrypi/pico-extras) libraries:

```
  cd $PICO_SDK_PATH/..
  git clone -b master https://github.com/raspberrypi/pico-extras.git
  cd pico-extras
  git submodule update --init
```

## Hardware

We just need a simple band-pass filter to extract the analog AFSK-signal from the binary PWM signal:

![band-pass filter](https://github.com/eleccoder/raspi-pico-aprs-tnc/blob/main/doc/img/band_pass_filter.png)

The line-out voltage can be as high as 2.7 V<sub>pp</sub> (~1 V<sub>rms</sub>) (at full-scale volume setting in the software and high-impedance load).

## Build the library and the example application

```
git clone https://github.com/eleccoder/raspi-pico-aprs-tnc.git
cd raspi-pico-aprs-tnc
cmake -S . -B build
cmake --build build
```

`build/lib/libaprs_pico.a` and `build/aprs_pico_example[.uf2|.elf|.bin]` will be generated.

## Run the example application

```
cd build
Flash 'aprs_pico_example[.uf2|.elf|.bin]' to the Pico board as usual
```

The analog AFSK audio signal will be available at the filter's line-out. You can probe it by a scope, listen to it by using an audio amp, or connect it to any RF transceiver to send it on the air (ham radio license required).

## Test the example application using *Dire Wolf* (on LINUX)

We can use the famous [Dire Wolf](https://github.com/wb2osz/direwolf) CLI software to decode the APRS data after sampling our APRS audio signal by means of a soundcard.

1. Connect the line-out of our circuit above to the microphone input of your soundcard of your (ALSA-supported) LINUX system.
2. Check if you can hear the typical APRS 'modem-sound' on your audio output device by monitoring the input signal:

```
arecord -f cd -c 1 -t raw - | aplay -f cd -c 1 -t raw
```

3. Install [Dire Wolf](https://github.com/wb2osz/direwolf) on your system. Probably, you just have to run:

```
sudo apt install direwolf
```

4. Let's sample the APRS audio signal fed to the soundcard and forward the audio stream to *Dire Wolf*:

```
arecord -f cd -c 1 -t raw - | direwolf
```

5. Enjoy the decoded APRS message:

![Decoded APRS message by Dire Wolf](https://github.com/eleccoder/raspi-pico-aprs-tnc/blob/main/doc/img/direwolf_decoding.png)

## TODO (Dec. 2022)

- [x] Send the APRS message on the console (USB or UART) rather than hard-coding
- [x] PTT control for RF transceivers
- [x] Show how to physically connect to a Baofeng HT

## Ingredients / Acknowledgements

- For APRS => AX.25 => PCM conversion I'm using [my modified version](https://github.com/eleccoder/ax25-aprs-lib) of [fsphil's ax25beacon](https://github.com/fsphil/ax25beacon)
- For PCM => PWM conversion I'm using the `pico_audio_pwm` library from [pico-extras](https://github.com/raspberrypi/pico-extras) (NOTE: ATTOW, maturity seems to be rather alpha/beta)
