#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NfcAdapter.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <UUID.h>
#include <atomic>
#include <LittleFS.h>
#include "TapToEsp32.hpp"

using namespace websockets;

//Config found in ReadTag.hpp

MFRC522 mfrc522(SS_PIN, RST_PIN);
NfcAdapter nfc = NfcAdapter(&mfrc522);
boolean requestSent = false;
AudioOutputI2S* out;

void setup() {
    Serial.begin(115200);
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
          if(sendTapTo(payloadAsString)){
            Serial.print("SCAN\t" + payloadAsString + "\n");
            triggerLaunchLed(HIGH, 0);
            triggerMotor(200, 1, 0);
            playAudio();
            triggerLaunchLed(LOW, 2000);
          }
          nfc.haltTag();
          delay(1000);
        }
      }
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

bool sendTapTo(String gamePath){
  WebsocketsClient client;
  std::atomic<bool> complete(false);
  std::atomic<bool> wasError(false);
  JsonDocument doc;
  UUID uuid;
  const char* id = uuid.toCharArray();
  doc["jsonrpc"]= "2.0";
  doc["method"]="launch";
  doc["id"]= uuid.toCharArray();
  doc["params"]["text"] = gamePath;
  doc.shrinkToFit();
  client.onMessage([&complete, &wasError, &id](WebsocketsMessage msg){
    if(complete.load()) return;
    JsonDocument result;
    DeserializationError error = deserializeJson(result, msg.data());
    if (error) {
      Serial.print("Failed to parse json");
      Serial.println(error.c_str());
      expressError(4);
      complete.store(true);
      wasError.store(true);
      return;
    }
    const char* resultId = result["id"];
    if(strcmp(id, resultId) != 0) return;
    complete.store(true);
    if(result.containsKey("result")){
      Serial.print("Error with game path");
      expressError(3);
      wasError.store(true);
      return;
    }
  });
  if(!client.connect(tapToUrl)){
    Serial.println("Unable to connect");
    expressError(2);
    return false;
  }
  String request;
  serializeJson(doc, request);
  client.send(request);
  while(!complete.load()){
    client.poll();
  }
  client.close();
  return !wasError.load();
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
  AudioFileSourceLittleFS* file = new AudioFileSourceLittleFS(launchAudio);
  AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
  mp3->begin(file, out);
  while(mp3->loop()){}
  mp3->stop();
  delete file;
  delete mp3;
  #endif
}
