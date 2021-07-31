# raspi-pico-aprs-tnc
A TX-only [TNC](https://en.wikipedia.org/wiki/Terminal_node_controller) (Terminal Network Controller) to generate the AFSK audio tones for [APRS](https://en.wikipedia.org/wiki/Automatic_Packet_Reporting_System) (Automatic Packet Reporting System) messages using a Raspberry Pi Pico microcontroller board.

Basically, this is the data/signal flow:

```
APRS (text msg + geo-coordinates + meta-data) -> AX.25 -> PCM -> PWM -> Low-Pass filtering -> AFSK signal @ GPIO 'GP0'
```

## Preliminaries

Your host platform is assumed to be LINUX.
If you have already installed the Pico-SDK, set the `PICO_SDK_PATH` environment variable accordingly to avoid installing the SDK it twice.

## Hardware

We just need a simple low-pass filter to extract the AFSK-signal from the PWM signal. Just connect GPIO-pin 'GP0' to a 1k resistor and the latter to a 47 nF cap.
The junction between the resistor and the cap is our signal output.

## Build the application
```
(cd into the clone dir)
cmake -S . -B build
cmake --build build
```

## Run the application
```
cd build
(copy any of the flash files 'aprs_pico.uf2' or 'aprs_pico.elf' to the Pico board)
```

The GPIO-pin 'GP0' is the line-out for the analog AFSK-signal. You can observe it by using a scope, listen to it by using an audio amp, or connect it to an HT to bring it on the air (ham radio license required).

## Modify the application

To send an APRS message of your choice, you have to modify the *main()* function in `src/aprs_pico.c`.

## TODO (Aug 2021)

- [x] Send the APRS message on the console (USB or UART) rather than hard-coding
- [x] Code documentation
- [x] Show how to connect to a Baofeng HT
- [x] PTT control for HTs
- [x] Scope screenshots
- [x] Schematic & Fritzings
