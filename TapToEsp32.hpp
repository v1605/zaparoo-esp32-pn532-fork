#ifndef READ_TAG_H
#define READ_TAG_H

//Reguired Configuration-------------------------------------------------

//The SS/SDA and Reset pins of the MFRC522
#define SS_PIN 12
#define RST_PIN 27

//Wifi and mister information
const char* ssid = "NetworkName";
const char* password = "Password";
const String misterRemoteUrl = "http://mister.local:8182";

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

//-----------------------------------------------------------------------
#endif
