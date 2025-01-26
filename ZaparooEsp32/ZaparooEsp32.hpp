#ifndef ZAPAROO_ESP32_H
  #define ZAPAROO_ESP32_H

  //*********************Reguired Configuration******************************
  
  //uncomment for RC522----------------------------------------------------
  //#define RC522
  //-----------------------------------------------------------------------


  //uncomment for PN532----------------------------------------------------
  #define PN532
  //The GPIO pin the PN532 RSTPDN Pin is connected to ((***NOT THE PN532 RSTO PIN***)). 
  //This is essential for stability and recovery after an esp32 soft reset.
  #define PN532_RST_PIN 13
  //------------------------------------------------------------------------

  //Common Config----------------------------------------------------------
  //The SS/SDA and Reset pins of the RC522 or SD Card using the default SPI GPIO pins of your ESP32
  #define SS_PIN 10
  #define RST_PIN 4
  //------------------------------------------------------------------------

  //Set True for serial only-----------------------------------------------
  //wifi Game Launch & Reset API calls will not be used - webUI features will still be available
  #define SERIAL_ONLY true
  //-----------------------------------------------------------------------

#endif


//INFO ------------------------------------------------------------------------
//DEFAULT ESP32S3 GPIO PIN ASSIGNMENTS
// PN532 Module
// (Check your ESP32's default I2C pins as the code uses them by default)
// SDA            8
// SCL            9
// RSTPDN         36 (Can be reassigned above with PN532_RST_PIN)

// SDCARD or RC522 Module
// (Check your ESP32's default vspi pins as the code uses them by default)
// SDA            10 (Can be reassigned above with SS_PIN)
// SCK            12
// MOSI           11
// MISO           13
// RST            4 (Can be reassigned above with RST_PIN)

// LED's          !!Configurable in Web UI!!
// WIFI           2 (This is normally replicates the cards wifi led, but it may be a different gpio on your esp32)
// PWR            37
// Launch         39

// Rumble Motor   !!Configurable in Web UI!!
// Motor          35

// Audio MAX98357 Module   !!Configurable in Web UI!!
// DOUT           17
// BCLK           16
// LRC            15
// GAIN           GND

//INFO ------------------------------------------------------------------------
//DEFAULT ESP32S2 Mini GPIO PIN ASSIGNMENTS
// PN532 Module
// (Check your ESP32's default I2C pins as the code uses them by default)
// SDA            8
// SCL            9
// RSTPDN         12 (Can be reassigned above with PN532_RST_PIN)

// SDCARD or RC522 Module
// (Check your ESP32's default vspi pins as the code uses them by default)
// SDA            34 (Can be reassigned above with SS_PIN)
// SCK            36
// MOSI           35
// MISO           34
// RST            4 (Can be reassigned above with RST_PIN)

// LED's          !!Configurable in Web UI!!
// WIFI           11 (This is normally replicates the cards wifi led, but it may be a different gpio on your esp32)
// PWR            6
// Launch         4

// Rumble Motor   !!Configurable in Web UI!!
// Motor          11

// Audio MAX98357 Module   !!Configurable in Web UI!!
// DOUT           17
// BCLK           16
// LRC            15
// GAIN           GND

//INFO ------------------------------------------------------------------------
//DEFAULT ESP32 GPIO PIN ASSIGNMENTS
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

// LED's          !!Configurable in Web UI!!
// WIFI           2 (This is normally replicates the cards wifi led, but it may be a different gpio on your esp32)
// PWR            15
// Launch         33

// Rumble Motor   !!Configurable in Web UI!!
// Motor          32

// Audio MAX98357 Module   !!Configurable in Web UI!!
// DOUT           25
// BCLK           27
// LRC            26
// GAIN           GND
