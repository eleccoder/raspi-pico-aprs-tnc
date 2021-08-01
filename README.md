# raspi-pico-aprs-tnc
A TX-only [TNC](https://en.wikipedia.org/wiki/Terminal_node_controller) (Terminal Network Controller) to generate the [AFSK](https://en.wikipedia.org/wiki/Frequency-shift_keying#Audio_FSK) audio tones for [APRS](https://en.wikipedia.org/wiki/Automatic_Packet_Reporting_System) (Automatic Packet Reporting System) messages using a [Raspberry Pi Pico](https://en.wikipedia.org/wiki/Raspberry_Pi) microcontroller board.

An analog line-out audio signal will be produced at GPIO-pin 'GP0'. You can observe it by using a scope, listen to it by using an audio amp, or connect it to any RF transceiver to send it on the air (ham radio license required).

Basically, this is the data/signal flow:

```
APRS (text msg + geo-coordinates + meta-data) -> AX.25 -> PCM -> PWM -> Low-Pass filtering -> AFSK signal @ GPIO 'GP0'
```

## Preliminaries

Your host platform is assumed to be LINUX.
If you have already installed the Pico-SDK, set the `PICO_SDK_PATH` environment variable accordingly to avoid installing the SDK twice.

## Hardware

We just need a simple band-pass filter to extract the AFSK-signal from the PWM signal:

![band-pass](https://github.com/eleccoder/raspi-pico-aprs-tnc/doc/schematic/band_pass_filter.png)


## Build the application

```
(cd into the cloned dir)
cmake -S . -B build
cmake --build build
```

## Run the application

```
cd build
(flash 'aprs_pico.uf2' or 'aprs_pico.elf' to the Pico board as usual)
```

The GPIO-pin 'GP0' is the line-out for the analog AFSK-signal. You can observe it by using a scope, listen to it by using an audio amp, or connect it to any RF transceiver to send it on the air (ham radio license required).

## Modify the application

To send an APRS message of your choice, you have to modify the *main()* function in `src/aprs_pico.c`.

## TODO (Aug 2021)

- [x] Thorough evaluation
- [x] Send the APRS message on the console (USB or UART) rather than hard-coding
- [x] Code documentation
- [x] Show how to connect to a Baofeng HT
- [x] PTT control for HTs

