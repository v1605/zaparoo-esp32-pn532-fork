# TapTo Esp32

The goal of this project is to launch a game via the [TapTo Service](https://github.com/TapToCommunity/tapto) on the a Mister over Wifi.

As of now, this code should be considered alpha, as it is not using the TapTo api. Instead it parses the game from the TapTo card and launches via the [Mister Remote](https://github.com/wizzomafizzo/mrext?tab=readme-ov-file#remote). Once TapTo 2.0 is released, I plan on revisiting and using that latest api.

## Required Parts
An Esp32
A MFRC522

## Required Libaries
ArduinoJson
MFRC55
[NDEF](https://github.com/don/NDEF/tree/master)
