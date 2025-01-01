# Zaparoo ESP32

The goal of this project is to launch games via the [Zaparoo Service](https://wiki.zaparoo.org/Main_Page) on the a Mister over Wifi. It also supports launching games via [Simple Serial](https://wiki.zaparoo.org/Reader_Drivers#Simple_Serial).

The project is now supporting the initial release of the v2 api (which uses websockets). Further updates are required once the security layer is implemented in the api.

## This fork has been enanched with new features & added support for both the the RC522 & PN532 rfid modules.  
New features:  
- Supports Universal launching for both MiSTer and SteamOS
- Play Card Detected, Game Launched, and Card Remove Audio files  
- Play game specific Game Launched and Card Removed Audio files  
- SD Card Module support (Only Supported with PN532 Module installed) to store more audio files.
- A Simple Web UI to
  - configure the ESP32's LED's, Rumble Motor, & Audio Module's GPIO pins (if installed),
  - change Zaparoo MiSTer and SteamOS settings.
  - upload audio files to SD Card (if available), or SPIFFS partition.
  - Search for games & write NFC tag/Cards with extended information.
  - Set default detect, launch, and remove audio files
  - Configure unique launch audio and card removal audio sounds per game/card.

Short Video  
[![Zaparoo-esp32](https://img.youtube.com/vi/U0me8gvgdw8/0.jpg)](https://youtu.be/U0me8gvgdw8)
  
## Hardware
* An Esp32
* A PN532 (in I2C mode) or RC522 Module
* (Optional) A SD Card module (Note: Only supported with PN532 Module installed)
* (Optional) A MAX98357a board and speaker
* (Optional) PWM Rumble motor. You can use a preconstructed board or build your own using a transistor and motor.
* (Optional) LED's for Pwr, Wifi, & Game Launch indication.  

## Required Libaries (Which may have their own dependencies)
* ESP8266Audio  
* ArdunioJSON
* Arduino
* Preferences
* AsyncTCP
* [ESPAsyncWebServer](https://github.com/MintyTrebor/ESPWebFileManager)
* [Zaparoo-esp32-launch-api](https://github.com/ZaparooProject/zaparoo-esp32-launch-api)  
* ArduinoWebSockets
* SPI
* LittleFS
* AudioFileSourceLittleFS
* AudioOutputI2S
* AudioGeneratorMP3  
  
## Required Libraries for PN532 Module Only:  
* [PN532 & NDEF](https://github.com/MintyTrebor/PN532)  
* Wire

## Required Libraries for RC522 Module Only:  
* [NDEF](https://github.com/TheNitek/NDEF)
* [MFRC522](https://github.com/MintyTrebor/rfid)  
  
## Setup
1. If you haven't already, downalod and install Arduino Studio.
2. Install the above Libaries, selecting the approriate libraries for the RFID module chosen.
3. Clone/Download the repository and copy all the files in this folder into a folder called ZaparooEsp32. Open ZaparooEsp32.ino in Arunido Studio.
4. Select your board and port, located in the Tools menu. If you do not have options for an Esp32, follow these [instructions](https://docs.sunfounder.com/projects/umsk/en/latest/03_esp32/esp32_start/03_install_esp32.html) for installing the board configurations.
5. Edit the ZaparooEsp32.hpp file to define your your Wifi credentials, RFID module type, & addtional options.
6. (Optional) If you are planning to use a battery to power the Esp32, it might help to extend battery life by decreasing the CPU frequency under tools. If using the audio configuration, you need a min cpu of 160mhz.
7. Make sure your "Partion Scheme" under tools is set to "No OTA(2MB APP/xMB SPIFFS)". xMB will vary based on your board, but 2MB is common. 
8. Compile and upload the project your esp32.

## Web Setup
WebUI: Open your browser and enter the IP address of your ESP32 (see serial console of the IDE or your router to get IP address - to access serial console click the magnifiying glass in the top left corner of the IDE, and set baud to 115200 - connect or reset the esp32 to see info and IP address).
1. Zaparoo Settings: Enter IP Address of MiSTer &/or Steamdeck - can also choose to return to MiSTer menu on card removal.
2. ESP32 Settings: Toggle LED's/Audio/Rumble motor and set GPIO Pins. Also toggle SD Card module
3. Audio Settings: Set Audio Gain (Volume : 0.1 - 4), and files names of audio files for Detect, Launch, & Remove Tag.
4. Audio File Manager: Upload and delete files to the ESP32 device SD Card Module (when Enabled in Settings)
5. Toggle NFC Write Mode: Search for games and write data to NFC card.  

## Error Feedback
If you choose to enable the Motor or Launch LED in the Web UI, you can get additional feedback when scanning a card:
* 2 Pulses: Could not connect to Zaparoo
* 3 Pulses: Zaparoo could not read the text, check your tag
* 4 Pulses: Zaparoo sent a payload that could not be parsed to JSON.

## Portable Options
It is possible to incorporate a battery to make a wireless NFC adapter. There are a two options to consider.
1. An Adadfruit Feather Esp32: it has all the required hardware to run off a lipo battery. The downside is that even with a power switch to ground the enable pin, you can still get battery drain. If you add a swich on the battery, you can only charge when its powered on. It's also possible adding a motor and audio will be too much for the regulator to handle.
2. Combine a lipo charging board, 5v buck converter, and a switch to power the board. This option require more assembly and reasearch but avoids the downsides of option one.

## 3D Printed Case  
A basic case example is provided in the stl folder of the repo.  

## ESP32 Default GPIO Pins  

### PN532 Module
(Check your ESP32's default I2C pins as the code uses them by default)    
SDA             21  
SCL             22  
RSTPDN          13  

### SDCARD OR RC522 Module
(Check your ESP32's default vspi pins as the code uses them by default)    
SDA     5  
SCK     18  
MOSI    23  
MISO    19  
RST     4  

### LED's
WIFI    2  
PWR     15  
Launch  33  

### Rumble Motor
Motor   32  

### Audio MAX98357
DOUT    25   
BCLK    27  
LRC     26  
GAIN    GND
