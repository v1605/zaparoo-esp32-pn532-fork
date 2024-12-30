# Zaparoo ESP32

The goal of this project is to launch games via the [Zaparoo Service](https://wiki.zaparoo.org/Main_Page) on the a Mister over Wifi. It also supports launching games via [Simple Serial](https://wiki.zaparoo.org/Reader_Drivers#Simple_Serial).

The project is now supporting the initial release of the v2 api (which uses websockets). Further updates are required once the security layer is implemented in the api.

## Hardware
* An Esp32
* A [MFRC522](https://github.com/MintyTrebor/zaparoo-esp32/tree/main/RC522) or [PN532](https://github.com/MintyTrebor/zaparoo-esp32/tree/main/PN532) Module
* (Optional) A MAX98357a board and speaker
* (Optional) PWM Rumble motor. You can use a preconstructed board or build your own using a transistor and motor.
* (Optional) SD Card Module (PN532 version only)

## For MFRC522 Module use the version in the [RC522 Folder](https://github.com/MintyTrebor/zaparoo-esp32/tree/main/RC522)  
Detailed instructions are in the included readme.md.  
## For PN532 Module use the version in the [PN532 Folder](https://github.com/MintyTrebor/zaparoo-esp32/tree/main/PN532)  
Detailed instructions are in the included readme.md.  
