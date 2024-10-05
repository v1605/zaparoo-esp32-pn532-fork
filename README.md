# TapTo Esp32

The goal of this project is to launch a game via the [TapTo Service](https://github.com/TapToCommunity/tapto) on the a Mister over Wifi. It should work with any Ardunio compatible board but only Esp32 models are official supported.

The project is now supporting the intial release of the v2 api (which uses websockets). Further updates are required once the security layer is implemented in the api.

## Required Parts
* An Esp32
* A MFRC522

## Required Libaries
* ArduinoJson
* UUID
* MFRC55
* ESP8266Audio
* ArdunioWebsockets
* [NDEF](https://github.com/don/NDEF/tree/master)

## Setup
1. If you haven't already, downalod and install Ardunio Studio.
2. Install the above Libaries.
3. Clone/Download the repository and open Tap2Esp32.ino in Arunido Studio.
4. Select your board and port, located in the Tools menu. If you do not have options for an Esp32, follow these [instrustions](https://docs.sunfounder.com/projects/umsk/en/latest/03_esp32/esp32_start/03_install_esp32.html) for installing the board configurations.
5. Edit the ReadTag.hpp file to define your pins for the MFRC522, the Mister url (using the Remote port until TapTo 2.0, default 8182), and your Wifi credentials.
6. (Optional) If you are planning to use a battery to power the Esp32, it might help to extend battery life by decreasing the CPU frequency under tools. If using the audio configuration, you need a min cpu of 160mhz.
7. (Optional) Enable any of the settings in the optional section of the config by uncommenting the line and setting the correct value for your setup. See "Optional Setup" for more info.
8. (Optional) If using audio options, make sure your "Partion Scheme" under tools is set to "Default xMB with spiffs". xMB will vary based on your board, but 4MB is common. After you upload the project, follow the instructons [here](https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/) to upload a mp3 (test with [this gb sound effect](https://tuna.voicemod.net/sound/e4674ff7-386c-4932-9faf-e50c82d45099)).
9. Compile and upload the project your esp32.

## Optional Setup
These descriptions are for the optional config options found in the ReadTag.hpp file. Uncommment and edit the lines in the config if you want to enable them.
1. MOTOR_PIN: Sends a pulse to this pin when WiFi is connected, a game is launched, or some error has occurred (see Error Feedback). It is intended for a small vibration motor to provide haptic feedback.
2. WIFI_LED_PIN: This pin will enter a high state once a WiFi connection has been established.
3. LAUNCH_LED_PIN: This pin mimics the behaviors the MOTOR_PIN, with a slightly different pulse behavior more suitable for an LED.
4. EXTERNAL_POWER_LED: This pin will enter a High state as soon as the unit boots. This feature is useful if your enclousre blocks any built in LEDs.
5. I2S_*: These pins are for a I2S module (such as a max98357a) to produce a launch sound. The audio file must be a mp3 file uploaded via LittleFs as described above.
6. launchAudio: The name of the mp3 file that plays when a tag is launched.
7. AUDIO_GAIN: How loud the speaker should be.

## Error Feedback
If you choose to enable the Motor or Launch pins in the config file, you can get additional feedback when scanning a card:
* 2 Pulses: Could not connect to TapTo
* 3 Pulses: TapTo could not read the text, check your tag
* 4 Pulses: TapTo sent a payload that could not be parsed to JSON.
