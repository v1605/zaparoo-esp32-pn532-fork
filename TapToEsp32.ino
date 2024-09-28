#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include "NfcAdapter.h"
#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "launchAudio.h"
#include "TapToEsp32.hpp"

//Config found in ReadTag.hpp

MFRC522 mfrc522(SS_PIN, RST_PIN);
NfcAdapter nfc = NfcAdapter(&mfrc522);
AudioOutputI2S* out;

void setup(void) {
    Serial.begin(9600);
    setupPins();
    initWiFi();
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
  #endif
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
          sendTapTo(payloadAsString);
          nfc.haltTag();
          delay(1000);
        }
      }
    delay(200);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    triggerWifiLed(HIGH, 0);
    delay(500);
    triggerWifiLed(LOW, 500);
  }
  Serial.println(WiFi.localIP());
  triggerWifiLed(HIGH, 0);
  triggerMotor(250, 2, 100);
}

void sendTapTo(String gamePath){
  HTTPClient http;
  http.begin(tapToUrl + "/api/v1/launch/" + urlEncode(gamePath));
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    Serial.println("Launched");
    triggerLaunchLed(HIGH, 0);
    triggerMotor(200, 1, 0);
    playAudio();
    triggerLaunchLed(LOW, 2000);
  }else{
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    expressError(2);
  }
  http.end();
}

void triggerMotor(int time, int loopCount, int loopDelay){
  #ifdef MOTOR_PIN 
  int last = loopCount - 1;
  for(int i=0; i <= last; i++){
    digitalWrite(MOTOR_PIN, HIGH);
    delay(time);
    digitalWrite(MOTOR_PIN, LOW);
    if(loopDelay > 0 && i != last){
      delay(loopDelay);
    }
  }
  #endif
}

void triggerLaunchLed(int state, int preDelay){
  #ifdef LAUNCH_LED_PIN
  if(preDelay > 0){
    delay(preDelay);
  }
  digitalWrite(LAUNCH_LED_PIN, state);
  #endif
}

void triggerWifiLed(int state, int preDelay){
  #ifdef WIFI_LED_PIN
  if(preDelay > 0){
    delay(preDelay);
  }
  digitalWrite(WIFI_LED_PIN, state);
  #endif
}

void expressError(int code){
  int motorPin = -1;
  int launchPin= -1;
  #ifdef MOTOR_PIN 
  motorPin = MOTOR_PIN;
  #endif
  #ifdef LAUNCH_LED_PIN
  launchPin = LAUNCH_LED_PIN;
  #endif
  if(motorPin == -1 && launchPin == -1) return;
  for(int i=0; i < code; i++){
    if(motorPin >= 0) digitalWrite(motorPin, HIGH);
    if(launchPin >= 0) digitalWrite(launchPin, HIGH);
    delay(800);
    if(motorPin >= 0) digitalWrite(motorPin, LOW);
    if(launchPin >= 0) digitalWrite(launchPin, LOW);
    delay(400);
  }
}

void playAudio(){
  #ifdef I2S_DOUT
  AudioFileSourcePROGMEM* file = new AudioFileSourcePROGMEM(launchAudio, sizeof(launchAudio));
  AudioGeneratorWAV* wav = new AudioGeneratorWAV();
  wav->begin(file, out);
  delay(50); //No delay, no sound
  if (wav->isRunning()) {
    if (!wav->loop()) wav->stop();
  } 
  delete file;
  delete wav;
  #endif
}
