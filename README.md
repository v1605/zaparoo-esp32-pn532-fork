# Zaparoo Esp32

The goal of this project is to launch games via the [Zaparoo Service](https://github.com/TapToCommunity/tapto) on the a Mister over Wifi. It also supports launching games via [Simple Serial](https://tapto.wiki/Reader_Drivers#Simple_Serial).

The project is now supporting the intial release of the v2 api (which uses websockets). Further updates are required once the security layer is implemented in the api.

## Hardare
* An Esp32
* A MFRC522
* (Optional) A MAX98357a board and speaker
* (Optional) PWM Rumble motor. You can use a preconstructed board or build your own using a transistor and motor.

## Required Libaries (Which may have their own dependencies)
* MFRC55
* ESP8266Audio
* [NDEF](https://github.com/don/NDEF/tree/master)
* [Zaparoo Esp32 Launch Api](https://github.com/v1605/Zaparoo-esp32-launch-api)

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
1. SERIAL_ONLY: Uncommenting this setting will disable wifi. Only Simple Serial will be supported in this mode.
2. MOTOR_PIN: Sends a pulse to this pin when WiFi is connected, a game is launched, or some error has occurred (see Error Feedback). It is intended for a small vibration motor to provide haptic feedback.
3. WIFI_LED_PIN: This pin will enter a high state once a WiFi connection has been established.
4. LAUNCH_LED_PIN: This pin mimics the behaviors the MOTOR_PIN, with a slightly different pulse behavior more suitable for an LED.
5. EXTERNAL_POWER_LED: This pin will enter a High state as soon as the unit boots. This feature is useful if your enclousre blocks any built in LEDs.
6. I2S_*: These pins are for a I2S module (such as a max98357a) to produce a launch sound. The audio file must be a mp3 file uploaded via LittleFs as described above.
7. launchAudio: The name of the mp3 file that plays when a tag is launched.
8. AUDIO_GAIN: How loud the speaker should be.

## Error Feedback
If you choose to enable the Motor or Launch pins in the config file, you can get additional feedback when scanning a card:
* 2 Pulses: Could not connect to TapTo
* 3 Pulses: TapTo could not read the text, check your tag
* 4 Pulses: TapTo sent a payload that could not be parsed to JSON.

## Portable Options
It is possible to incorporate a battery to make a wireless NFC adapter. There are a two options to consider.
1. An Adadfruit Feather Esp32: it has all the required hardware to run off a lipo battery. The downside is that even with a power switch to ground the enable pin, you can still get battery drain. If you add a swich on the battery, you can only charge when its powered on. It's also possible adding a motor and audio will be too much for the regulator to handle.
2. Combine a lipo charging board, 5v buck converter, and a switch to power the board. This option require more assembly and reasearch but avoids the downsides of option one.
