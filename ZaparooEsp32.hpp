#ifndef ZAPAROO_ESP32_H
  #define ZAPAROO_ESP32_H

  //*********************Reguired Configuration******************************
  
  //uncomment for RC522----------------------------------------------------
  //#define RC522
  //-----------------------------------------------------------------------


  //uncomment for PN532----------------------------------------------------
  //#define PN532
  //The GPIO pin the PN532 RSTPDN Pin is connected to ((***NOT THE PN532 RSTO PIN***)). 
  //This is essential for stability and recovery after an esp32 soft reset.
  //#define PN532_RST_PIN 13
  //------------------------------------------------------------------------

  //Common Config----------------------------------------------------------
  //The SS/SDA and Reset pins of the RC522 or SD Card using the default SPI GPIO pins of your ESP32
  #define SS_PIN 5
  #define RST_PIN 4
  //------------------------------------------------------------------------


  //Wifi information--------------------------------------------------------
  const char* ssid = "******";
  const char* password = "*******";
  //------------------------------------------------------------------------

  //Set True for serial only-----------------------------------------------
  //wifi Game Launch & Reset API calls will not be used - webUI features will still be available
  #define SERIAL_ONLY false
  //-----------------------------------------------------------------------

#endif


//INFO ------------------------------------------------------------------------
//DEFAULT GPIO PIN ASSIGNMENTS
// PN532 Module
// (Check your ESP32's default I2C pins as the code uses them by default)
// SDA            21
// SCL            22
// RSTPDN         13 (Can be reassigned above with PN532_RST_PIN)

// SDCARD or RC522 Module
// (Check your ESP32's default vspi pins as the code uses them by default)
// SDA            5 (Can be reassigned above with SS_PIN)
// SCK            18
// MOSI           23
// MISO           19
// RST            4 (Can be reassigned above with RST_PIN)

// LED's
// WIFI           2 (This is normally replicates the cards wifi led, but it may be a different gpio on your esp32)
// PWR            15
// Launch         33

// Rumble Motor
// Motor          32

// Audio MAX98357 Module
// DOUT           25
// BCLK           27
// LRC            26
// GAIN           GND