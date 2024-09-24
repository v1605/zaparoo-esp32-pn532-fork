#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ReadTag.hpp"
#include "NfcAdapter.h"

//Config found in ReadTag.hpp

MFRC522 mfrc522(SS_PIN, RST_PIN);
NfcAdapter nfc = NfcAdapter(&mfrc522);

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

void sendTapTo(String gamePath){ // Waiting for 2.0 TapTo, for now mister remote
  HTTPClient http;
  http.begin(misterRemoteUrl + "/api/games/launch");
  String payload = buildRequest(gamePath);
  if(payload.equals("")){
    Serial.print("Error building request for ");
    Serial.println(gamePath);
    return;
  }
  Serial.println(payload);
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == 200) {
    Serial.println("Launched");
    triggerLaunchLed(HIGH, 0);
    triggerMotor(200, 1, 0);
    triggerLaunchLed(LOW, 2000);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    expressError(3);
  }
  http.end();
}

String buildRequest(String fullPath){
  int pathSplit = fullPath.indexOf('/');
  String path = getGamePath(fullPath.substring(0, pathSplit), fullPath.substring(pathSplit+1));
  if(path.equals("")) return "";
  JsonDocument doc;
  doc["path"] = path;
  String output;
  doc.shrinkToFit();
  serializeJson(doc, output);
  return output;
}

String getGamePath(String system, String game){
  HTTPClient http;
  http.begin(misterRemoteUrl + "/api/games/search");
  JsonDocument sendDoc;
  sendDoc["query"] = game.substring(game.lastIndexOf('/') + 1, game.lastIndexOf('.'));;
  sendDoc["system"] = system;
  String req;
  serializeJson(sendDoc, req);
  int responseCode = http.POST(req);
  if (responseCode >= 300 || responseCode < 0) {
    Serial.print("Error code for searching game path: ");
    Serial.println(responseCode);
    expressError(4);
    return "";
  }
  JsonDocument doc;
  if(deserializeJson(doc, http.getStream())){
    Serial.print("Error deserializing game path response");
    expressError(5);
    return "";
  }
  http.end();
  JsonObject data = doc["data"][0];
  const char* path = data["path"];
  return String(path);
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
