#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <SPI.h>
#include <AudioFileSourceLittleFS.h>
#include "AudioFileSourceSD.h"
#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <LittleFS.h>
#include <ESPWebFileManager.h>
#include <NfcAdapter.h>
#include <ESPmDNS.h>
#include <ZaparooLaunchApi.h>
#include <ZaparooRestResult.h>
#include "index.h"
#include "ZaparooEsp32.hpp"
#include "ZaparooScanner.cpp"

#ifdef PN532
#include "scanners/ZaparooPN532Scanner.cpp"
PN532_I2C pn532_i2c(Wire);
#endif

#ifdef RC522
#include "scanners/ZaparooRC522Scanner.cpp"
MFRC522 mfrc522(SS_PIN, RST_PIN);
#endif

//Common Setup
AudioOutputI2S* out;
Preferences preferences;
using namespace websockets;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
ESPWebFileManager fileManager;
ZaparooLaunchApi ZapClient;
ZaparooScanner* tokenScanner = NULL;

//globals
String ZAP_URL = "ws://<replace>:7497" + String(ZaparooLaunchApi::wsPath);
ZaparooToken* token = NULL;
bool inserted = false;
bool isPN532 = false;
int timeoutLoop = 0;
float val_AUDIO_GAIN = 1;
bool isConnected = false;
bool isWebLog = false;
bool softReset = false;
bool wifi_led_enabled = false;
bool motor_enabled = false;
bool launch_led_enabled = false;
bool audio_enabled = false;
bool pwr_led_enabled = false;
bool reset_on_remove_enabled = true;
bool mister_enabled = true;
bool steamOS_enabled = false;
bool sdCard_enabled = false;
bool systemAudio_enabled = false;
bool gameAudio_enabled = false;
bool buzz_on_detect_enabled = true;
bool buzz_on_launch_enabled = true;
bool buzz_on_remove_enabled = true;
bool buzz_on_error_enabled = true;
bool UID_ScanMode_enabled = false;
String SteamIP = "steamOS.local";
String defAudioPath = "";
String defDetectAudioPath = "";
String defRemoveAudioPath = "";
String defErrAudioPath = "";
String ZapIP = "mister.local";
const char* uidAudio = "/uidAudioMappings.csv";
const char* uidExtdRecFile = "/uidExtdRecord.json";

//Prototypes
void playAudio(const String& PrefString);
void notifyClients(const String& txtMsgToSend);
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);


void setPref_Bool(const String& key, bool valBool) {
  preferences.putBool(key.c_str(), valBool);
}

void setPref_Int(const String& key, int valInt) {
  preferences.putInt(key.c_str(), valInt);
}

void setPref_Str(const String& key, const String& valStr) {
  preferences.putString(key.c_str(), valStr);
}

void setPref_Float(const String& key, float valFloat) {
  preferences.putFloat(key.c_str(), valFloat);
}


void setupPins() {
  if (preferences.getBool("En_Motor", false)) {
#define MOTOR_PIN preferences.getInt("MOTOR_PIN", 32)
    pinMode(MOTOR_PIN, OUTPUT);
  }
  if (preferences.getBool("En_Wifi_LED", false)) {
#define WIFI_LED_PIN preferences.getInt("WIFI_LED_PIN", 2)
    pinMode(WIFI_LED_PIN, OUTPUT);
  }
  if (preferences.getBool("En_Lnch_LED", false)) {
#define LAUNCH_LED_PIN preferences.getInt("LAUNCH_LED_PIN", 33)
    pinMode(LAUNCH_LED_PIN, OUTPUT);
  }
  if (preferences.getBool("En_EPwr_LED", false)) {
#define EXTERNAL_POWER_LED preferences.getInt("EXT_PWRLED_PIN", 15)
    pinMode(EXTERNAL_POWER_LED, OUTPUT);
    digitalWrite(EXTERNAL_POWER_LED, HIGH);
  }
  if (preferences.getBool("En_Audio", false)) {
#define I2S_DOUT preferences.getInt("I2S_DOUT", 25)
#define I2S_BCLK preferences.getInt("I2S_BCLK", 27)
#define I2S_LRC preferences.getInt("I2S_LRC", 26)
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetChannels(1);
    out->SetGain(preferences.getFloat("AUDIO_GAIN", 1));
#ifdef PN532
    if (preferences.getBool("En_SDCard", false)) {
      if (!SD.begin(SS_PIN)) {
        Serial.println(F("failed to do SD Card"));
      }
    }
#endif
  }
}

void motorOn(int predelay = 0) {
  if (motor_enabled) {
    delay(predelay);
    analogWrite(MOTOR_PIN, 255);
  }
}

void motorOff(int predelay = 0) {
  if (motor_enabled) {
    delay(predelay);
    analogWrite(MOTOR_PIN, 0);
  }
}

void launchLedOn(int predelay = 0) {
  if (launch_led_enabled) {
    delay(predelay);
    digitalWrite(LAUNCH_LED_PIN, HIGH);
  }
}

void launchLedOff(int predelay = 0, int postDelay = 0) {
  if (launch_led_enabled) {
    delay(predelay);
    digitalWrite(LAUNCH_LED_PIN, LOW);
    delay(postDelay);
  }
}

void wifiLedOn() {
  if (wifi_led_enabled) {
    digitalWrite(WIFI_LED_PIN, HIGH);
  }
}

void wifiLedOff() {
  if (wifi_led_enabled) {
    digitalWrite(WIFI_LED_PIN, LOW);
  }
}

void expressError(int code) {
  for (int i = 0; i < code; i++) {
    launchLedOn();
    if(buzz_on_error_enabled){
        motorOn(0);
        motorOff(800);
    }
    if (defErrAudioPath.length() > 0) {
      playAudio(defErrAudioPath);
    }
    launchLedOff(0, 400);
  }
}

void successActions(const String& audioPath) {
  launchLedOn(0);
  const String& pathToPlay = !audioPath.isEmpty() ? audioPath : defAudioPath;
  if (!pathToPlay.isEmpty()) {
      if(buzz_on_launch_enabled){
        motorOn(0);
        motorOff(100);
      }
      playAudio(pathToPlay);
    }else{
      if(buzz_on_launch_enabled){
        motorOn(0);
        motorOff(1000);
      }
    }    
  launchLedOff(0, 0);
}

void setUidAudioMappings(ZaparooToken* obj) {
  if(!audio_enabled) return;
  File file;
  JsonDocument UIDFile;
  if (sdCard_enabled && SD.exists(uidExtdRecFile)) {
    file = SD.open(uidExtdRecFile, FILE_READ);
  } else if (LittleFS.exists(uidExtdRecFile)) {
    file = LittleFS.open(uidExtdRecFile, "r");
  } else {
    return;
  }
  String uid = String(obj->getId());
  while (file.available()) {
    DeserializationError error = deserializeJson(UIDFile, file);
    if(!error){
      for(JsonObject item : UIDFile["UID_ExtdRecs"].as<JsonArray>()){
        if(item["UID"] == uid){
          obj->setLaunchAudio(item["launchAudio"]);
          obj->setRemoveAudio(item["removeAudio"]);
          file.close();
          return;
        }
      }
    }
  }
  file.close();
  return;
}


void playAudio(const String& PrefString) {
  if (!audio_enabled || PrefString.isEmpty()) {
    delay(1000);
    return;
  }
  const char* launchAudio = PrefString.c_str();
  AudioFileSource* file = nullptr;
  if (sdCard_enabled) {
    AudioFileSourceSD* source = new AudioFileSourceSD(launchAudio);
    file = source;
  } else {
    AudioFileSourceLittleFS* source = new AudioFileSourceLittleFS(launchAudio);
    file = source;
  }
  if (file->getSize() == 0) {
    notifyClients("Audio file did not exist, check path: " + PrefString);
    delete file;
    file = NULL;
    return;
  }
  AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
  mp3->begin(file, out);
  while (mp3->loop()) {}
  mp3->stop();
  delete mp3;
  mp3 = NULL;
  delete file;
  file = NULL;
}

void cardInsertedActions() {
  inserted = true;
  if (defDetectAudioPath.length() > 0) {
    playAudio(defDetectAudioPath);
  }
  if(buzz_on_detect_enabled){
    motorOn(0);
    motorOff(100);
  }
}

void cardRemovedActions(const String& audioRemPath) {
  inserted = false;
  const String& pathToPlay = !audioRemPath.isEmpty() ? audioRemPath : defRemoveAudioPath;
  if (!pathToPlay.isEmpty()) {
    playAudio(pathToPlay);
  }
  if(buzz_on_remove_enabled){
    motorOn(0);
    motorOff(100);
  }
}

void notifyClients(const String& txtMsgToSend) {
  Serial.println(txtMsgToSend);
  if (isWebLog) {
    JsonDocument msgJson;
    msgJson["msgType"] = "notify";
    msgJson["data"] = txtMsgToSend;
    String output;
    serializeJson(msgJson, output);
    delay(200);
    ws.textAll(output.c_str());
    if (txtMsgToSend == "closeWS") {
      delay(100);
      ESP.restart();
    }
  } else {
    if (txtMsgToSend == "closeWS") {
      ESP.restart();
    }
  }
}

void cmdClients(JsonDocument& cmdJson) {
  String output;
  serializeJson(cmdJson, output);
  Serial.print("WS MSG SENT: ");
  Serial.println(output);
  delay(100);
  ws.textAll(output.c_str());
}

void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
             void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      isWebLog = true;
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      setPref_Bool("En_NFC_Wr", false);
      isWebLog = false;
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void startApMode() {
  server.end();
  WiFi.disconnect();
  //WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP("zapesp32", "zapesp32");
  Serial.println("Starting AP Mode:");
  Serial.println(WiFi.softAPIP());
  server.begin();
  initWebSocket();
  MDNS.begin("zapesp");
}

void connectWifi() {
  if (WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_MODE_AP) {
    return;
  }
  WiFi.disconnect();
  MDNS.end();
  //WiFi.mode(WIFI_STA);
  String ssid = preferences.getString("Wifi_SSID", "");
  if(ssid.isEmpty()){
    startApMode();
    return;
  }
  WiFi.begin(ssid, preferences.getString("Wifi_PASS", ""));
  int retries = 30;
  while (WiFi.status() != WL_CONNECTED && retries--) {
    wifiLedOn();
    delay(500);
    wifiLedOff();
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    startApMode();
    return;
  }
  // Initialize mDNS
  retries = 10;
  while (!MDNS.begin("zapesp") && retries--) {
    Serial.println("Error setting up MDNS responder!");
    delay(1000);
  }
  Serial.println("mDNS started - access the web UI @ http://zapesp.local");
  Serial.print("WiFi connected - Zap ESP IP = ");
  Serial.println(WiFi.localIP());
  WiFi.setSleep(false);
  server.begin();
  initWebSocket();
  motorOn();
  motorOff(250);
  motorOn(100);
  motorOff(250);
  wifiLedOn();
}

void writeTagLaunch(String& launchCmd, String& audioLaunchFile, String& audioRemoveFile) {
  String tmpLaunchCmd = launchCmd;
  tmpLaunchCmd.replace("launch_cmd::", "");
  if (tokenScanner->tokenPresent()) {
    bool success = tokenScanner->writeLaunch(launchCmd, audioLaunchFile, audioRemoveFile);
    if (success) {
      notifyClients("Data sucessfully written. Remove the Tag/Card and close 'Creation Mode' before testing.");
    } else {
      notifyClients("Data write failed. Resetting the NFC device! Remove the Tag/Card and try again.");
    }
  } else {
    notifyClients("No NFC Tag/Card detected - Aborting Write - Please insert a Valid NFC Tag/Card");
  }
  tokenScanner->halt();
}

//Save the UIDExtdRec.json received from Web Client
void saveUIDExtdRec(JsonDocument newUIDExtdRec){
  File file;
  if(sdCard_enabled && SD.exists(uidExtdRecFile)){
    file = SD.open(uidExtdRecFile, FILE_WRITE);
  }else if(LittleFS.exists(uidExtdRecFile)){
    file = LittleFS.open(uidExtdRecFile, FILE_WRITE);
  }
  while (file.available()) {
    serializeJson(newUIDExtdRec, file);
    file.close();
    return;
  }
  file.close();
  return;
}

//Load the UIDExtdRec.json and pass to Web Client
void getUIDExtdRec(){
  JsonDocument UIDData;
  JsonDocument UIDFile;
  File file;
  UIDData["msgType"] = "getUIDExtdRec";
  if(sdCard_enabled && SD.exists(uidExtdRecFile)){
    file = SD.open(uidExtdRecFile, FILE_READ);
  }else if(LittleFS.exists(uidExtdRecFile)){
    file = LittleFS.open(uidExtdRecFile, "r");
  }
  while (file.available()) { 
    DeserializationError error = deserializeJson(UIDFile, file);
    if(!error){
      UIDData["data"] = UIDFile;
      cmdClients(UIDData);
      file.close();
      return;
    }
  }
  file.close();
  return;
}


bool send(String& gamePath) {
  String message;
  bool sent = false;
  if (SERIAL_ONLY) {
    Serial.print("SCAN\t");
    Serial.println(gamePath);
    Serial.flush();
    message = "Sent game path to serial: " + gamePath;
    sent = true;
  } else {
    String newURL = ZAP_URL;
    newURL.replace("<replace>", gamePath.startsWith("steam://") ? SteamIP : ZapIP);
    ZapClient.url(newURL);
    notifyClients("URL: " + newURL);
    int code = ZapClient.launch(gamePath);
    if (code > 0) {
      expressError(code);
      message = "Zaparoo Error Launching Game: " + gamePath + " | Error Code: " + code;
    } else {
      message = "Launched Game: " + gamePath;
      sent = true;
    }
  }
  notifyClients(message);
  return sent;
}

//Send the detected UID to Web Client for Audio Mapping
void sendUIDtoWeb(String UIDStr){
  JsonDocument UIDData;
  UIDData["msgType"] = "UIDTokenID";
  UIDData["data"] = UIDStr;
  cmdClients(UIDData);
}

void getWebConfigData() {
  notifyClients("Retrieving Current Zap ESP Config Data");
  JsonDocument configData;
  configData["msgType"] = "ConfigData";
  configData["data"]["wifi_led_enabled"] = preferences.getBool("En_Wifi_LED", false);
  configData["data"]["motor_enabled"] = preferences.getBool("En_Motor", false);
  configData["data"]["launch_led_enabled"] = preferences.getBool("En_Lnch_LED", false);
  configData["data"]["audio_enabled"] = preferences.getBool("En_Audio", false);
  configData["data"]["pwr_led_enabled"] = preferences.getBool("En_EPwr_LED", false);
  configData["data"]["reset_on_remove_enabled"] = preferences.getBool("En_RoR", true);
  configData["data"]["defAudioPath"] = preferences.getString("Audio_F_Path", "");
  configData["data"]["ZapIP"] = preferences.getString("ZapIP", "mister.local");
  configData["data"]["pin_I2S_DOUT"] = preferences.getInt("I2S_DOUT", 25);
  configData["data"]["pin_I2S_BCLK"] = preferences.getInt("I2S_BCLK", 27);
  configData["data"]["pin_I2S_LRC"] = preferences.getInt("I2S_LRC", 26);
  configData["data"]["val_AUDIO_GAIN"] = preferences.getFloat("AUDIO_GAIN", 1);
  configData["data"]["pin_MOTOR_PIN"] = preferences.getInt("MOTOR_PIN", 32);
  configData["data"]["pin_WIFI_LED_PIN"] = preferences.getInt("WIFI_LED_PIN", 2);
  configData["data"]["pin_LAUNCH_LED_PIN"] = preferences.getInt("LAUNCH_LED_PIN", 33);
  configData["data"]["pin_EXTERNAL_POWER_LED"] = preferences.getInt("EXT_PWRLED_PIN", 15);
  configData["data"]["mister_enabled"] = preferences.getBool("En_Mister", true);
  configData["data"]["steamOS_enabled"] = preferences.getBool("En_SteamOS", false);
  configData["data"]["sdCard_enabled"] = preferences.getBool("En_SDCard", false);
  configData["data"]["SteamIP"] = preferences.getString("SteamIP", "steamOS.local");
  configData["data"]["defDetectAudioPath"] = preferences.getString("Audio_D_Path", "");
  configData["data"]["defRemoveAudioPath"] = preferences.getString("Audio_R_Path", "");
  configData["data"]["defErrAudioPath"] = preferences.getString("Audio_E_Path", "");
  configData["data"]["systemAudio_enabled"] = preferences.getBool("En_SysAudio", false);
  configData["data"]["gameAudio_enabled"] = preferences.getBool("En_GameAudio", false);
  configData["data"]["buzz_on_detect_enabled"] = preferences.getBool("En_Buzz_Det", true);
  configData["data"]["buzz_on_launch_enabled"] = preferences.getBool("En_Buzz_Lau", true);
  configData["data"]["buzz_on_remove_enabled"] = preferences.getBool("En_Buzz_Rem", true);
  configData["data"]["buzz_on_error_enabled"] = preferences.getBool("En_Buzz_Err", true);
  configData["data"]["PN532_module"] = isPN532;
  configData["data"]["zap_ws_path"] = ZaparooLaunchApi::wsPath;
  cmdClients(configData);
}

void setWebConfigData(JsonDocument cfgData) {
  notifyClients("ZAP ESP Now Saving Config Data");
  if (!cfgData["data"]["wifi_led_enabled"]) {
    wifiLedOff();
  }
  if (!cfgData["data"]["launch_led_enabled"]) {
    launchLedOff();
  }
  if (!cfgData["data"]["pwr_led_enabled"] && preferences.getBool("En_EPwr_LED", false)) {
    digitalWrite(EXTERNAL_POWER_LED, LOW);
  }
  setPref_Bool("En_Wifi_LED", cfgData["data"]["wifi_led_enabled"]);
  setPref_Bool("En_Motor", cfgData["data"]["motor_enabled"]);
  setPref_Bool("En_Lnch_LED", cfgData["data"]["launch_led_enabled"]);
  setPref_Bool("En_Audio", cfgData["data"]["audio_enabled"]);
  setPref_Bool("En_EPwr_LED", cfgData["data"]["pwr_led_enabled"]);
  setPref_Bool("En_RoR", cfgData["data"]["reset_on_remove_enabled"]);
  setPref_Str("Audio_F_Path", cfgData["data"]["defAudioPath"]);
  setPref_Str("ZapIP", cfgData["data"]["ZapIP"]);
  setPref_Int("I2S_DOUT", cfgData["data"]["pin_I2S_DOUT"]);
  setPref_Int("I2S_BCLK", cfgData["data"]["pin_I2S_BCLK"]);
  setPref_Int("I2S_LRC", cfgData["data"]["pin_I2S_LRC"]);
  setPref_Float("AUDIO_GAIN", cfgData["data"]["val_AUDIO_GAIN"]);
  setPref_Int("MOTOR_PIN", cfgData["data"]["pin_MOTOR_PIN"]);
  setPref_Int("WIFI_LED_PIN", cfgData["data"]["pin_WIFI_LED_PIN"]);
  setPref_Int("LAUNCH_LED_PIN", cfgData["data"]["pin_LAUNCH_LED_PIN"]);
  setPref_Int("EXT_PWRLED_PIN", cfgData["data"]["pin_EXTERNAL_POWER_LED"]);
  setPref_Bool("En_SDCard", cfgData["data"]["sdCard_enabled"]);
  setPref_Bool("En_SteamOS", cfgData["data"]["steamOS_enabled"]);
  setPref_Bool("En_Mister", cfgData["data"]["mister_enabled"]);
  setPref_Str("SteamIP", cfgData["data"]["SteamIP"]);
  setPref_Str("Audio_D_Path", cfgData["data"]["defDetectAudioPath"]);
  setPref_Str("Audio_R_Path", cfgData["data"]["defRemoveAudioPath"]);
  setPref_Str("Audio_E_Path", cfgData["data"]["defErrAudioPath"]);
  setPref_Bool("En_SysAudio", cfgData["data"]["systemAudio_enabled"]);
  setPref_Bool("En_GameAudio", cfgData["data"]["gameAudio_enabled"]);
  setPref_Bool("En_Buzz_Det", cfgData["data"]["buzz_on_detect_enabled"]);
  setPref_Bool("En_Buzz_Lau", cfgData["data"]["buzz_on_launch_enabled"]);
  setPref_Bool("En_Buzz_Rem", cfgData["data"]["buzz_on_remove_enabled"]);
  setPref_Bool("En_Buzz_Err", cfgData["data"]["buzz_on_error_enabled"]);
  notifyClients("closeWS");
}

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = static_cast<AwsFrameInfo*>(arg);
  if (!info->final || info->index != 0 || info->len != len || info->opcode != WS_TEXT) {
    return;  // Invalid frame
  }
  String webCmd(reinterpret_cast<char*>(data), len);
  JsonDocument root;
  DeserializationError error = deserializeJson(root, webCmd);
  if (error) {
    notifyClients("Failed to Parse JSON");
    return;
  }
  String command = root["cmd"].as<String>();

  if (command == "set_WriteMode") {
    bool enableWriteMode = root["data"];
    notifyClients(enableWriteMode ? "NFC Tag Write Mode Enabled" : "NFC Tag Write Mode Disabled");
    setPref_Bool("En_NFC_Wr", enableWriteMode);
  } else if (command == "write_Tag_Launch_Game") {
    if (preferences.getBool("En_NFC_Wr", false)) {
      notifyClients("NFC Tag Writing the Launch Game Command");
      String launchData = root["launchData"].as<String>();
      String audioLaunchPath = root["audioLaunchPath"].as<String>();
      String audioRemovePath = root["audioRemovePath"].as<String>();
      writeTagLaunch(launchData, audioLaunchPath, audioRemovePath);
    }
  } else if (command == "get_Current_Config") {
    getWebConfigData();
  } else if (command == "set_Current_Config") {
    setWebConfigData(root);
  } else if (command == "Test_Tag_Launch_Game") {
    notifyClients("Test Launching Game");
    String data = root["data"].as<String>();
    send(data);
  } else if (command == "closeWS") {
    setPref_Bool("En_NFC_Wr", false);
    ws.closeAll();
    ws.cleanupClients();
  } else if (command == "getUIDExtdRec") {
    notifyClients("Retrieving UIDExtdRec Data");
    getUIDExtdRec();
  } else if (command == "set_UIDMode") {
    bool enableUIDMode = root["data"];
    notifyClients(enableUIDMode ? "UID Scanning Mode Enabled" : "UID Scanning Mode Disabled");
    UID_ScanMode_enabled = enableUIDMode;
    //get UIDExtdRec data if enabling UID mode
    if(enableUIDMode){getUIDExtdRec();}
  } else if (command == "saveUIDExtdRec") {
    notifyClients("Saving UIDExtdRec Data");
    saveUIDExtdRec(root["data"]);
  } else if (command == "wifi") {
    setPref_Str("Wifi_SSID", root["data"]["ssid"].as<String>());
    setPref_Str("Wifi_PASS", root["data"]["password"].as<String>());
    WiFi.disconnect(); 
    WiFi.mode(WIFI_STA);
    notifyClients("Updated ssid");
  } else {
    notifyClients("Unknown Command");
  }
}

bool sendUid(String& uid) {
  String message;
  bool sent = false;
  if (SERIAL_ONLY) {
    Serial.print("SCAN\tuid=");
    Serial.println(uid);
    Serial.flush();
    message = "Sent Card/Tag UID: " + uid;
    sent = true;
  } else {
    //not possible to determine if steam game from UID so always default to MiSTer if enabled
    String newURL = ZAP_URL;
    newURL.replace("<replace>", mister_enabled ? ZapIP : SteamIP);
    ZapClient.url(newURL);
    int code = ZapClient.launchUid(uid);
    if (code > 0) {
      expressError(code);
      message = "Zaparoo Error Sending Card/Tag UID " + uid + " | Error Code: " + code;
    } else {
      message = "Sent Card/Tag UID: " + uid;
      sent = true;
    }
  }
  notifyClients(message);
  return sent;
}

//Loop 20 times per read to break out and run other loop code
bool readScanner() {
  for (int i = 0; i < 20 && !preferences.getBool("En_NFC_Wr", false); i++) {
    bool present = tokenScanner->tokenPresent();
    ZaparooToken* parsed = present ? tokenScanner->getNewToken() : NULL;
    if (present && parsed) {
      if (!parsed->getValid()) {
        inserted = false;
        delete parsed;
        delay(10);
        continue;
      }
      delete token;
      token = NULL;
      token = parsed;
      cardInsertedActions();
      bool sent = false;
      if (token->isPayloadSet() && !UID_ScanMode_enabled) {
        setUidAudioMappings(token);
        String payload = String(token->getPayload());
        sent = send(payload);
      } else if (token->isIdSet() && !UID_ScanMode_enabled) {
        setUidAudioMappings(token);
        String id = String(token->getId());
        sent = sendUid(id);
      }else if(token->isIdSet() && UID_ScanMode_enabled){
        String id = String(token->getId());
        sendUIDtoWeb(id);
      }
      if(sent && !UID_ScanMode_enabled) {
        String audio = token->isLaunchAudioSet() ? String(token->getLaunchAudio()) : "";
        successActions(audio);
      }
    } else if (!present && inserted) {  //Must have been removed
      String removeAudio = "";
      if (token->isRemoveAudioSet()) {
        removeAudio = token->getRemoveAudio();
      }
      cardRemovedActions(removeAudio);
      if (reset_on_remove_enabled && !SERIAL_ONLY && token->isPayloadSet()) {
        String payloadAsString = String(token->getPayload());
        if (!payloadAsString.startsWith("steam://")) {
          ZapClient.stop();
        }
      }
      notifyClients("Tag Removed");
      tokenScanner->halt();
      return true;
    }
    delay(10);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
#ifdef PN532
  Wire.begin();
  ZaparooPN532Scanner* pnScanner = new ZaparooPN532Scanner();
  pnScanner->setConfig(pn532_i2c);
  pnScanner->setResetPin(PN532_RST_PIN);
  tokenScanner = pnScanner;
  isPN532 = true;
#endif
#ifdef RC522
  ZaparooRC522Scanner* rcScanner = new ZaparooRC522Scanner();
  rcScanner->setConfig(mfrc522);
  mfrc522.PCD_Init();
  tokenScanner = rcScanner;
  isPN532 = false;
#endif
  //Check for PN532 Card Initalisation Failure and reset if in error
  if(!tokenScanner->init() && isPN532){
    tokenScanner->reset();
  }
  preferences.begin("qrplay", false);
  setPref_Bool("En_NFC_Wr", false);
  UID_ScanMode_enabled = false;
  setupPins();

  
  //set globals to reduce the number of call to preference library (performance)
  wifi_led_enabled = preferences.getBool("En_Wifi_LED", false);
  motor_enabled = preferences.getBool("En_Motor", false);
  launch_led_enabled = preferences.getBool("En_Lnch_LED", false);
  audio_enabled = preferences.getBool("En_Audio", false);
  pwr_led_enabled = preferences.getBool("En_EPwr_LED", false);
  reset_on_remove_enabled = preferences.getBool("En_RoR", true);
  defAudioPath = preferences.getString("Audio_F_Path", "");
  defDetectAudioPath = preferences.getString("Audio_D_Path", "");
  defRemoveAudioPath = preferences.getString("Audio_R_Path", "");
  defErrAudioPath = preferences.getString("Audio_E_Path", "");
  ZapIP = preferences.getString("ZapIP", "mister.local");
  val_AUDIO_GAIN = preferences.getFloat("AUDIO_GAIN", 1);
  mister_enabled = preferences.getBool("En_Mister", true);
  steamOS_enabled = preferences.getBool("En_SteamOS", false);
  sdCard_enabled = preferences.getBool("En_SDCard", false);
  systemAudio_enabled = preferences.getBool("En_SysAudio", false);
  gameAudio_enabled = preferences.getBool("En_GameAudio", false);
  SteamIP = preferences.getString("SteamIP", "steamOS.local");
  buzz_on_detect_enabled = preferences.getBool("En_Buzz_Det", true);
  buzz_on_launch_enabled = preferences.getBool("En_Buzz_Lau", true);
  buzz_on_remove_enabled = preferences.getBool("En_Buzz_Rem", true);
  buzz_on_error_enabled = preferences.getBool("En_Buzz_Err", true);

  if (preferences.getBool("En_SDCard", false)) {
    Serial.println("SD CARD MODE");
    fileManager.initFileSystem(ESPWebFileManager::FS_SD_CARD, true);
    fileManager.setServer(&server);
  } else {
    Serial.println("LITTLEFS MODE");
    fileManager.initFileSystem(ESPWebFileManager::FS_LITTLEFS, true);
    fileManager.setServer(&server);
  }

  //check if uidExtdRecord.json file exists and if not create it
  JsonDocument tmpDoc;
  tmpDoc["UID_ExtdRecs"][0]["UID"] = "";
  tmpDoc["UID_ExtdRecs"][0]["launchAudio"] = "";
  tmpDoc["UID_ExtdRecs"][0]["removeAudio"] = "";
  String tmpJSONStr = "";
  serializeJson(tmpDoc, tmpJSONStr);
  if (sdCard_enabled){
    if(!SD.exists(uidExtdRecFile)){
      File UIDfile;
      UIDfile = SD.open(uidExtdRecFile, FILE_WRITE);
      UIDfile.print(tmpJSONStr);
      UIDfile.close();
    }
  }else if(!LittleFS.exists(uidExtdRecFile)){
    File UIDfile;
    UIDfile = LittleFS.open(uidExtdRecFile, FILE_WRITE);
    UIDfile.print(tmpJSONStr);
    UIDfile.close();
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", index_html, index_html_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
}

void loop() {
  connectWifi();
  if (!preferences.getBool("En_NFC_Wr", false)) {
    readScanner();
  }
  delay(50);
}
