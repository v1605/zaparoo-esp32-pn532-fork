#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "NfcAdapter.h"

//------------------Config
#define SS_PIN 12
#define RST_PIN 27
const char* ssid = "NetworkName";
const char* password = "Password";
const String misterRemoteUrl = "http://mister.local:8182";
//------------------EndConfig

MFRC522 mfrc522(SS_PIN, RST_PIN);
NfcAdapter nfc = NfcAdapter(&mfrc522);

void setup(void) {
    Serial.begin(9600);
    initWiFi();
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
    nfc.begin();
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
          Serial.println("Game: "+ payloadAsString);
          sendTapTo(payloadAsString);
          nfc.haltTag();
          delay(4000);
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
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void sendTapTo(String gamePath){ // Waiting for 2.0 TapTo, for now mister remote
  HTTPClient http;
  http.begin(misterRemoteUrl + "/api/games/launch");
  String payload = buildRequest(gamePath);
  Serial.println(payload);
  int httpResponseCode = http.POST(payload);
  if (httpResponseCode == 200) {
    Serial.print("Launched");
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
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
  sendDoc["query"] = game.substring(game.lastIndexOf('/') + 1, game.lastIndexOf('.'));
  sendDoc["system"] = system;
  String req;
  serializeJson(sendDoc, req);
  int responseCode = http.POST(req);
  if (responseCode >= 300 || responseCode < 0) {
    Serial.print("Error code: ");
    Serial.println(responseCode);
    return "";
  }
  DynamicJsonDocument doc(2048);
  if(deserializeJson(doc, http.getStream())){
    return "";
  }
  http.end();
  JsonObject data = doc["data"][0];
  const char* path = data["path"];
  return String(path);
}
