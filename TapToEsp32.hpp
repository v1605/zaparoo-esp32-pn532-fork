#ifndef TAP_TO_ESP32_H
#define TAP_TO_ESP32_H

//Reguired Configuration-------------------------------------------------

//The SS/SDA and Reset pins of the MFRC522
#define SS_PIN 12
#define RST_PIN 27

//Wifi and mister information
const char* ssid = "NetworkName";
const char* password = "Password";
const String tapToUrl = "http://mister.local:7497";

//-----------------------------------------------------------------------

//Optional Configurations------------------------------------------------

//Uncomment if using a vibration motor for haptic feedback
//#define MOTOR_PIN 25

//Uncomment if using a led to display wifi status
//#define WIFI_LED_PIN 16

//Uncomment if using a led to display launch status
//#define LAUNCH_LED_PIN 16

//Uncomment if using a led to display power indicator
//#define EXTERNAL_POWER_LED 16

//Uncomment if using a I2S module to produce audio
//#define I2S_DOUT 26
//#define I2S_BCLK 17
//#define I2S_LRC 21

//-----------------------------------------------------------------------
#endif
