# Zaparoo ESP32

The goal of this project is to launch games via the [Zaparoo Service](https://wiki.zaparoo.org/Main_Page) on the a Mister over Wifi. 
It provides a simple Web interface to enable configuration of the ESP32/LED/Rumble Motor/Audio, and change settings.

This fork is based on the main branch of the zaparoo-esp32 project.

The project is now supporting the initial release of the v2 api (which uses websockets). Further updates are required once the security layer is implemented in the api.

## Hardware
* An Esp32
* A MFRC522
* (Optional) A MAX98357a board and speaker
* (Optional) PWM Rumble motor. You can use a preconstructed board or build your own using a transistor and motor.

## Required Libaries (Which may have their own dependencies)
* MFRC55  
* ESP8266Audio  
* [NDEF-MFRC522](https://github.com/aroller/NDEF-MFRC522)  
* ArdunioJSON
* Arduino
* Preferences
* AsyncTCP
* ESPAsyncWebServer
* ArduinoWebSockets
* UUID
* SPI
* LittleFS
* AudioFileSourceLittleFS
* AudioOutputI2S
* AudioGeneratorMP3

Code also includes modified versions of the following libraries 
* QRCode.js
* [ESPWebFileManager](https://github.com/jobitjoseph/ESPWebFileManager)


## Setup
1. If you haven't already, downalod and install Arduino Studio.
2. Install the above Libaries.
3. Clone/Download the repository and open ZaparooEsp32.ino in Arunido Studio.
4. Select your board and port, located in the Tools menu. If you do not have options for an Esp32, follow these [instrustions](https://docs.sunfounder.com/projects/umsk/en/latest/03_esp32/esp32_start/03_install_esp32.html) for installing the board configurations.
5. Edit the ZaparooEsp32.ino file to define your your Wifi credentials.
6. Ensure Boards -> Boards Manager -> esp32 is set to version 3.0.7
7. (Optional) If you are planning to use a battery to power the Esp32, it might help to extend battery life by decreasing the CPU frequency under tools. If using the audio configuration, you need a min cpu of 160mhz.
8. Make sure your "Partion Scheme" under tools is set to "No OTA(2MB APP/xMB SPIFFS)". xMB will vary based on your board, but 2MB is common. 
9. Compile and upload the project your esp32.

## Web Setup
WebUI: Open your browser and enter the IP address of your ESP32 (see serial console of the IDE or your router to get IP address - to access serial console click the magnifiying glass in the top left corner of the IDE, and set baud to 115200 - connect or reset the esp32 to see info and IP address).
1. Zaparoo Settings: Enter IP Address of MiSTer &/or Steamdeck - can also choose to return to MiSTer menu on card removal.
2. ESP32 Settings: Toggle LED's/Audio/Rumble motor and set GPIO Pins
3. Audio Settings: Set Audio Gain (Volume : 0.1 - 4), and files names of audio files for Detect, Launch, & Remove Tag.
4. Audio File Manager: Upload and delte files to the ESP32 device - Note space available will be small and dependant on your esp32 device

## Error Feedback
If you choose to enable the Motor or Launch LED in the Web UI, you can get additional feedback when scanning a card:
* 2 Pulses: Could not connect to Zaparoo
* 3 Pulses: Zaparoo could not read the text, check your tag
* 4 Pulses: Zaparoo sent a payload that could not be parsed to JSON.

## Portable Options
It is possible to incorporate a battery to make a wireless NFC adapter. There are a two options to consider.
1. An Adadfruit Feather Esp32: it has all the required hardware to run off a lipo battery. The downside is that even with a power switch to ground the enable pin, you can still get battery drain. If you add a swich on the battery, you can only charge when its powered on. It's also possible adding a motor and audio will be too much for the regulator to handle.
2. Combine a lipo charging board, 5v buck converter, and a switch to power the board. This option require more assembly and reasearch but avoids the downsides of option one.

## ESP32 Default GPIO Pins    

### MFRC522 (Check your ESP32's default vspi pins as the code uses them by default - however SDA & RST must be set to the below GPIO pins unless you change the source code)
SDA     5  
SCK     18  
MOSI    23  
MISO    19  
RST     4  

### LED's
WIFI    2
PWR     22  
Launch  33  

### Rumble Motor
Motor   32  

### Audio MAX98357
DOUT    25   
BCLK    27  
LRC     26  
GAIN    GND
