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
#include "FeedbackManager.h"

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
FeedbackManager feedback;

//globals
String ZAP_URL = "ws://<replace>:7497" + String(ZaparooLaunchApi::wsPath);
ZaparooToken* token = NULL;
bool inserted = false;
bool isPN532 = false;
int timeoutLoop = 0;
bool isConnected = false;
bool isWebLog = false;
bool softReset = false;
bool mister_enabled = true;
bool steamOS_enabled = false;
bool UID_ScanMode_enabled = false;
String SteamIP = "steamOS.local";
String ZapIP = "mister.local";

//Prototypes
void notifyClients(const String& txtMsgToSend, const String& msgType);
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

void notifyClients(const String& txtMsgToSend, const String& msgType) {
  Serial.println(txtMsgToSend);
  if (isWebLog) {
    JsonDocument msgJson;
    msgJson["msgType"] = "notify";
    msgJson["data"]["msgTxt"] = txtMsgToSend;
    msgJson["data"]["type"] = msgType;
    String output;
    serializeJson(msgJson, output);
    delay(200);
    ws.textAll(output.c_str());
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
  WiFi.mode(WIFI_AP);
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
  String ssid = preferences.getString("Wifi_SSID", "");
  if(ssid.isEmpty()){
    startApMode();
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, preferences.getString("Wifi_PASS", ""));
  int retries = 30;
  while (WiFi.status() != WL_CONNECTED && retries--) {
    feedback.wifiLedOn();
    delay(500);
    feedback.wifiLedOff();
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
  feedback.motorOn();
  feedback.motorOff(250);
  feedback.motorOn(100);
  feedback.motorOff(250);
  feedback.wifiLedOn();
}

void writeTagLaunch(String& launchCmd, String& audioLaunchFile, String& audioRemoveFile) {
  String tmpLaunchCmd = launchCmd;
  JsonDocument cmdData;
  cmdData["msgType"] = "writeResults";
  tmpLaunchCmd.replace("launch_cmd::", "");
  notifyClients("Launch Cmd Written: " + launchCmd, "log");
  if (tokenScanner->tokenPresent()) {
    bool success = tokenScanner->writeLaunch(launchCmd, audioLaunchFile, audioRemoveFile);
    if (success) {
      cmdData["data"]["isSuccess"] = true;
      cmdData["data"]["isCardDetected"] = true;
      cmdClients(cmdData);
    } else {
      cmdData["data"]["isSuccess"] = false;
      cmdData["data"]["isCardDetected"] = true;
      cmdClients(cmdData);
    }
  } else {
    cmdData["data"]["isSuccess"] = false;
    cmdData["data"]["isCardDetected"] = false;
    cmdClients(cmdData);
  }  
  tokenScanner->halt();
}

//Load the UIDExtdRec.json and pass to Web Client
void getUIDExtdRec(){
  JsonDocument result;
  feedback.getUidMappings(result);
  cmdClients(result);
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
    notifyClients("URL: " + newURL, "log");
    int code = ZapClient.launch(gamePath);
    if (code > 0) {
      feedback.expressError(code);
      message = "Zaparoo Error Launching Game: " + gamePath + " | Error Code: " + code;
    } else {
      message = "Launched Game: " + gamePath;
      sent = true;
    }
  }
  notifyClients(message, "log");
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
  notifyClients("Retrieving Current Zap ESP Config Data", "log");
  JsonDocument configData;
  feedback.set(configData);
  configData["msgType"] = "ConfigData";
  configData["data"]["ZapIP"] = preferences.getString("ZapIP", "mister.local");
  configData["data"]["mister_enabled"] = preferences.getBool("En_Mister", true);
  configData["data"]["steamOS_enabled"] = preferences.getBool("En_SteamOS", false);
  configData["data"]["SteamIP"] = preferences.getString("SteamIP", "steamOS.local");
  configData["data"]["PN532_module"] = isPN532;
  configData["data"]["zap_ws_path"] = ZaparooLaunchApi::wsPath; 
  cmdClients(configData);
}

void setWebConfigData(JsonDocument& cfgData) {
  notifyClients("ZAP ESP Now Saving Config Data", "log");
  feedback.launchLedOff();
  feedback.update(cfgData);
  if(cfgData["data"].containsKey("ZapIP")){
    setPref_Str("ZapIP", cfgData["data"]["ZapIP"]);
  }
  if(cfgData["data"].containsKey("steamOS_enabled")){
    setPref_Bool("En_SteamOS", cfgData["data"]["steamOS_enabled"]);
  }
  if(cfgData["data"].containsKey("mister_enabled")){
    setPref_Bool("En_Mister", cfgData["data"]["mister_enabled"]);
  }
  if(cfgData["data"].containsKey("SteamIP")){
    setPref_Str("SteamIP", cfgData["data"]["SteamIP"]);
  }
  preferences.end();
  Serial.println("Rebooting after saving");
  ESP.restart();
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
    notifyClients("Failed to Parse JSON", "log");
    return;
  }
  String command = root["cmd"].as<String>();

  if (command == "set_WriteMode") {
    bool enableWriteMode = root["data"];
    notifyClients(enableWriteMode ? "NFC Tag Write Mode Enabled" : "NFC Tag Write Mode Disabled", "log");
    setPref_Bool("En_NFC_Wr", enableWriteMode);
  } else if (command == "write_Tag_Launch_Game" && preferences.getBool("En_NFC_Wr", false)) {
      notifyClients("NFC Tag Writing the Launch Game Command", "log");
      String launchData = root["data"]["launchData"].as<String>();
      String audioLaunchPath = root["data"]["audioLaunchPath"].as<String>();
      String audioRemovePath = root["data"]["audioRemovePath"].as<String>();
      writeTagLaunch(launchData, audioLaunchPath, audioRemovePath);
  } else if (command == "get_Current_Config") {
      getWebConfigData();
  } else if (command == "set_Current_Config") {
      setWebConfigData(root);
  } else if (command == "Test_Tag_Launch_Game") {
      notifyClients("Test Launching Game", "alert");
      String data = root["data"].as<String>();
      send(data);
  } else if (command == "closeWS") {
      setPref_Bool("En_NFC_Wr", false);
      ws.closeAll();
      ws.cleanupClients();
  } else if (command == "getUIDExtdRec") {
      notifyClients("Retrieving UIDExtdRec Data", "log");
      getUIDExtdRec();
  } else if (command == "set_UIDMode") {
      bool enableUIDMode = root["data"];
      notifyClients(enableUIDMode ? "UID Scanning Mode Enabled" : "UID Scanning Mode Disabled", "alert");
      UID_ScanMode_enabled = enableUIDMode;
      //get UIDExtdRec data if enabling UID mode
      if(enableUIDMode){
        getUIDExtdRec();
      }
  } else if (command == "saveUIDExtdRec") {
      notifyClients("Saving UIDExtdRec Data", "log");
      JsonDocument data = root["data"];
      feedback.saveUidMapping(data);
      //saveUIDExtdRec(root["data"]);
  } else if (command == "wifi") {
    setPref_Str("Wifi_SSID", root["data"]["ssid"].as<String>());
    setPref_Str("Wifi_PASS", root["data"]["password"].as<String>());
    WiFi.disconnect(); 
    WiFi.mode(WIFI_STA);
    notifyClients("Updated ssid", "log");
  } else {
    notifyClients("Unknown Command", "log");
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
      feedback.expressError(code);
      message = "Zaparoo Error Sending Card/Tag UID " + uid + " | Error Code: " + code;
    } else {
      message = "Sent Card/Tag UID: " + uid;
      sent = true;
    }
  }
  notifyClients(message, "log");
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
      feedback.cardInsertedActions(token);
      inserted = true;
      bool sent = false;
      if (token->isPayloadSet() && !UID_ScanMode_enabled) {
        //notifyClients("token set", "log");
        feedback.setUidAudioMappings(token);
        String payload = String(token->getPayload());
        sent = send(payload);
      } else if (token->isIdSet() && !UID_ScanMode_enabled) {
       //notifyClients("id set", "log");
       feedback.setUidAudioMappings(token);
        String id = String(token->getId());
        sent = sendUid(id);
      }else if(token->isIdSet() && UID_ScanMode_enabled){
        String id = String(token->getId());
        sendUIDtoWeb(id);
      }
      if(sent && !UID_ScanMode_enabled) {
        String audio = token->isLaunchAudioSet() ? String(token->getLaunchAudio()) : "";
        feedback.successActions(token);
      }
    } else if (!present && inserted) {  //Must have been removed
      String removeAudio = "";
      if (token->isRemoveAudioSet()) {
        removeAudio = token->getRemoveAudio();
      }
      feedback.cardRemovedActions(token);
      if (feedback.resetOnRemove && !SERIAL_ONLY && token->isPayloadSet()) {
        String payloadAsString = String(token->getPayload());
        if (!payloadAsString.startsWith("steam://")) {
          ZapClient.stop();
        }
      }
      notifyClients("Tag Removed", "log");
      inserted = false;
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
  feedback.init(&preferences);
  setPref_Bool("En_NFC_Wr", false);
  UID_ScanMode_enabled = false;
  
  //set globals to reduce the number of call to preference library (performance)
  ZapIP = preferences.getString("ZapIP", "mister.local");
  mister_enabled = preferences.getBool("En_Mister", true);
  steamOS_enabled = preferences.getBool("En_SteamOS", false);
  SteamIP = preferences.getString("SteamIP", "steamOS.local");

  if (feedback.sdCardEnabled) {
    Serial.println("SD CARD MODE");
    fileManager.initFileSystem(ESPWebFileManager::FS_SD_CARD, true);
    fileManager.setServer(&server);
  } else {
    Serial.println("LITTLEFS MODE");
    fileManager.initFileSystem(ESPWebFileManager::FS_LITTLEFS, true);
    fileManager.setServer(&server);
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
