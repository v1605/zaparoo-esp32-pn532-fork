#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <NfcAdapter.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <LittleFS.h>
#include <TapToLaunchApi.h>
#include "TapToEsp32.hpp"

//Config found in ReadTag.hpp

MFRC522 mfrc522(SS_PIN, RST_PIN);
NfcAdapter nfc = NfcAdapter(&mfrc522);
TapToLaunchApi client;
AudioOutputI2S* out;
boolean wifiEnabled = false;

void setup() {
    Serial.begin(115200);
    setupPins();
    #ifndef SERIAL_ONLY
    initWiFi();
    client.url(tapToUrl);
    #endif
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
    nfc.begin();
}

void setupPins(){
  #ifdef MOTOR_PIN
    pinMode(MOTOR_PIN, OUTPUT);
  #endif
  #ifdef WIFI_LED_PIN
    pinMode(WIFI_LED_PIN, OUTPUT);
  #endif
  #ifdef LAUNCH_LED_PIN
    pinMode(LAUNCH_LED_PIN, OUTPUT);
  #endif
  #ifdef EXTERNAL_POWER_LED
    pinMode(EXTERNAL_POWER_LED, OUTPUT);
    digitalWrite(EXTERNAL_POWER_LED, HIGH);
  #endif
  #ifdef I2S_DOUT
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetChannels(1);
    out->SetGain(AUDIO_GAIN);
    if (!LittleFS.begin(true)) {
      Serial.println("An error has occurred while mounting LittleFS. No launch audio");
    }
    else if (!LittleFS.exists(launchAudio)) {
      Serial.println("Launch audio file not found");
    }
  #endif
}

void motorOn(int predelay=0){
  #ifdef MOTOR_PIN
  delay(predelay);
  analogWrite(MOTOR_PIN, 255);
  #endif
}

void motorOff(int predelay=0){
  #ifdef MOTOR_PIN 
  delay(predelay);
  analogWrite(MOTOR_PIN, 0);
  #endif
}

void launchLedOn(int predelay=0){
  #ifdef LAUNCH_LED_PIN
  delay(predelay);
  digitalWrite(LAUNCH_LED_PIN, HIGH);
  #endif
}

void launchLedOff(int predelay=0, int postDelay=0){
  #ifdef LAUNCH_LED_PIN
  delay(predelay);
  digitalWrite(LAUNCH_LED_PIN, LOW);
  delay(postDelay);
  #endif
}

void wifiLedOn(){
  #ifdef WIFI_LED_PIN
  digitalWrite(WIFI_LED_PIN, HIGH);
  #endif
}

void wifiLedOff(){
  #ifdef WIFI_LED_PIN
  digitalWrite(WIFI_LED_PIN, LOW);
  #endif
}

void expressError(int code){
  for(int i=0; i < code; i++){
    motorOn();
    launchLedOn();
    motorOff(800);
    launchLedOff(0, 400);
  }
}

void playAudio(){
  #ifdef I2S_DOUT
  AudioFileSourceLittleFS* file = new AudioFileSourceLittleFS(launchAudio);
  AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
  mp3->begin(file, out);
  while(mp3->loop()){}
  mp3->stop();
  delete file;
  delete mp3;
  #else
  delay(1000);
  #endif
}

bool sendTapTo(String& gamePath){
  if(!wifiEnabled) return true;
  int code = client.launch(gamePath);
  if(code > 0){
    expressError(code);
  }
  return code == 0;
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    wifiLedOn();
    delay(500);
    wifiLedOff();
  }
  wifiEnabled =true;
  Serial.println(WiFi.localIP());
  wifiLedOn();
  motorOn();
  motorOff(250);
  motorOn(100);
  motorOff(250);
}

void loop(void) {
    if (nfc.tagPresent()) {
        NfcTag tag = nfc.read();       
        if(tag.hasNdefMessage()){
          NdefMessage message = tag.getNdefMessage();
          int recordCount = message.getRecordCount();
          NdefRecord record = message.getRecord(0);
          int payloadLength = record.getPayloadLength();
          const byte *payload = record.getPayload();
          String payloadAsString = "";
          for (int i = 3; i < payloadLength; i++) {
                payloadAsString += (char)payload[i];
          }
          if(!payloadAsString.equalsIgnoreCase("")){
            if(sendTapTo(payloadAsString)){
              Serial.print("SCAN\t" + payloadAsString + "\n");
              Serial.flush();
              launchLedOn(0);
              motorOn(0);
              playAudio();
              motorOff(0);
              launchLedOff();
            }
            nfc.haltTag();
            delay(1000);
          }
        }
      }
}
