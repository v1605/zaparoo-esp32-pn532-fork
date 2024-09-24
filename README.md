# TapTo Esp32

The goal of this project is to launch a game via the [TapTo Service](https://github.com/TapToCommunity/tapto) on the a Mister over Wifi. It should work with any Ardunio compatible board but only Esp32 models are official supported.

As of now, this code should be considered alpha, as it is not using the TapTo api. Instead it parses the game from the TapTo card and launches via the [Mister Remote](https://github.com/wizzomafizzo/mrext?tab=readme-ov-file#remote). Once TapTo 2.0 is released, I plan on revisiting and using that latest api.

## Required Parts
* An Esp32
* A MFRC522

## Required Libaries
* ArduinoJson
* MFRC55
* [NDEF](https://github.com/don/NDEF/tree/master)

## Setup
1. If you haven't already, downalod and install Ardunio Studio.
2. Install the above Libaries
3. Clone/Download the repository and open Tap2Esp32.ino in Arunido Studio.
4. Select your board and port, located in the Tools menu. If you do not have options for an Esp32, follow these [instrustions](https://docs.sunfounder.com/projects/umsk/en/latest/03_esp32/esp32_start/03_install_esp32.html) for installing the board configurations.
5. Edit the ReadTag.hpp file to define your pins for the MFRC522, the Mister url (using the Remote port until TapTo 2.0, default 8182), and your Wifi credentials.
6. (Optional) Enable any of the settings in the optional section of the config by uncommenting the line and setting the correct value for your setup. See "Optional Setup" for more info.

## Optional Setup
These descriptions are for the optional config options found in the ReadTag.hpp file. Uncommment and edit the lines in the config if you want to enable them.
1. MOTOR_PIN: Sends a pulse to this pin when WiFi is connected, a game is launched, or some error has occurred (see Error Feedback). It is intended for a small vibration motor to provide haptic feedback.
2. WIFI_LED_PIN: This pin will enter a high state once a WiFi connection has been established.
3. LAUNCH_LED_PIN: This pin mimics the behaviors the MOTOR_PIN, with a slightly different pulse behavior more suitable for an LED.
4. EXTERNAL_POWER_LED: This pin will enter a High state as soon as the unit boots. This feature is useful if your enclousre blocks any built in LEDs.

## Error Feedback
If you choose to enable the Motor or Launch pins in the config file, you can get additional feedback when scanning a card:
* 1 Pulse: Game was launched successfully.
* 3 Pulses: Game failed to launch via TapTo.
* 4 Pulses: Game could not be found on your Mster.
* 5 Pulses: Response for searching the game could not parsed (Probably a Mister Remote error).
