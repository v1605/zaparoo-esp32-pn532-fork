# TapTo Esp32

The goal of this project is to launch a game via the [TapTo Service](https://github.com/TapToCommunity/tapto) on the a Mister over Wifi. It should work with any Ardunio compatible board but only Esp32 models are official supported.

As of now, this code should be considered alpha, as it is not using the TapTo 2.0 api (instead using v1 endpoints). Once TapTo 2.0 is released I will rework the api calls.

## Required Parts
* An Esp32
* A MFRC522

## Required Libaries
* ArduinoJson (future TapTo 2.0)
* UrlEncode
* MFRC55
* ESP8266Audio
* [NDEF](https://github.com/don/NDEF/tree/master)

## Setup
1. If you haven't already, downalod and install Ardunio Studio.
2. Install the above Libaries.
3. Clone/Download the repository and open Tap2Esp32.ino in Arunido Studio.
4. Select your board and port, located in the Tools menu. If you do not have options for an Esp32, follow these [instrustions](https://docs.sunfounder.com/projects/umsk/en/latest/03_esp32/esp32_start/03_install_esp32.html) for installing the board configurations.
5. Edit the ReadTag.hpp file to define your pins for the MFRC522, the Mister url (using the Remote port until TapTo 2.0, default 8182), and your Wifi credentials.
6. (Optional) If you are planning to use a battery to power the Esp32, it might help to extend battery life by decreasing the CPU frequency under tools.
7. (Optional) Enable any of the settings in the optional section of the config by uncommenting the line and setting the correct value for your setup. See "Optional Setup" for more info.

## Optional Setup
These descriptions are for the optional config options found in the ReadTag.hpp file. Uncommment and edit the lines in the config if you want to enable them.
1. MOTOR_PIN: Sends a pulse to this pin when WiFi is connected, a game is launched, or some error has occurred (see Error Feedback). It is intended for a small vibration motor to provide haptic feedback.
2. WIFI_LED_PIN: This pin will enter a high state once a WiFi connection has been established.
3. LAUNCH_LED_PIN: This pin mimics the behaviors the MOTOR_PIN, with a slightly different pulse behavior more suitable for an LED.
4. EXTERNAL_POWER_LED: This pin will enter a High state as soon as the unit boots. This feature is useful if your enclousre blocks any built in LEDs.
5. I2S_*: These pins are for a I2S module (such as a max98357a) to produce a launch sound. You can replace the sound in the launchAudio.h file by converting a 8-bit unsigned pcm audio file to hex.

## Error Feedback
If you choose to enable the Motor or Launch pins in the config file, you can get additional feedback when scanning a card:
* 2 Pulses: Game failed to launch via TapTo.
