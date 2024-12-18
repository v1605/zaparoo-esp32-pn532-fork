#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <UUID.h>
#include <atomic>
#include <SPI.h>
#include <MFRC522.h>
#include <NfcAdapter.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <LittleFS.h>
#include "ESPWebFileManager.h"
#include "index.h"
#include "qr_code_js.h"

#define WIFI_SSID "****"
#define WIFI_PASSWORD "****"
#define SS_PIN 5
#define RST_PIN 4

MFRC522 mfrc522(SS_PIN, RST_PIN);
NfcAdapter nfc = NfcAdapter(&mfrc522);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
ESPWebFileManager fileManager;
using namespace websockets;
String lastNFC_ID = "";
int idLoop = 0;
AudioOutputI2S* out;
bool isConnected = false;
bool isWebLog = false;
Preferences preferences;
String ZAP_URL = "ws://<replace>:7497";
String STEAM_URL = "ws://<replace>:7497";

void setup()
{
  Serial.begin(115200);
  preferences.begin("qrplay", false);
  setPref_Bool("En_NFC_Wr", false);
  setupPins();
  bool connected = connectWifi();
  notifyClients("Zap ESP Ready");
  SPI.begin();
  mfrc522.PCD_Init();
  nfc.begin();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, index_html_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/qrcode.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/plain", qrcode_js, qrcode_js_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  if(preferences.getBool("En_SDCard", false)){
    Serial.println("SD CARD MODE");
    fileManager.initFileSystem(ESPWebFileManager::FS_SD_CARD, true);
    fileManager.setServer(&server);
  }else{
    Serial.println("LITTLEFS MODE");
    fileManager.initFileSystem(ESPWebFileManager::FS_LITTLEFS, true);
    fileManager.setServer(&server);
  }

}

void setupPins(){
  if(preferences.getBool("En_Motor", false)){
    #define MOTOR_PIN preferences.getInt("MOTOR_PIN", 32)
    pinMode(MOTOR_PIN, OUTPUT);
  }
  if(preferences.getBool("En_Wifi_LED", false)){
    #define WIFI_LED_PIN preferences.getInt("WIFI_LED_PIN", 2)
    pinMode(WIFI_LED_PIN, OUTPUT);
  }
  if(preferences.getBool("En_Lnch_LED", false)){
    #define LAUNCH_LED_PIN preferences.getInt("LAUNCH_LED_PIN", 33)
    pinMode(LAUNCH_LED_PIN, OUTPUT);
  }
  if(preferences.getBool("En_EPwr_LED", false)){
    #define EXTERNAL_POWER_LED preferences.getInt("EXT_PWRLED_PIN", 22)
    pinMode(EXTERNAL_POWER_LED, OUTPUT);
    digitalWrite(EXTERNAL_POWER_LED, HIGH);
  }
  if(preferences.getBool("En_Audio", false)){
    #define I2S_DOUT preferences.getInt("I2S_DOUT", 25)
    #define I2S_BCLK preferences.getInt("I2S_BCLK", 27)
    #define I2S_LRC preferences.getInt("I2S_LRC", 26)
    #define AUDIO_GAIN preferences.getFloat("AUDIO_GAIN", 1)
    String defAudioPath = preferences.getString("Audio_F_Path", "/gbc-startup.mp3");
    const char* launchAudio = defAudioPath.c_str();
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetChannels(1);
    out->SetGain(AUDIO_GAIN);
    if(preferences.getBool("En_SDCard", false)){
      // if (!SD.begin(SD_CS_PIN,SPISD)) {  //SD_CS_PIN this pin is just the dummy pin since the SD need the input 
      //   Serial.println(F("failed to do SD Card"));
      // }
    }else {
      if (!LittleFS.begin(true)) {
        Serial.println("An error has occurred while mounting LittleFS. No launch audio");
      }
      else if (!LittleFS.exists(launchAudio)) {
        Serial.println("Launch audio file not found");
      }
    }
  }
}

void motorOn(int predelay=0){
  if(preferences.getBool("En_Motor", false)){
    delay(predelay);
    analogWrite(MOTOR_PIN, 255);
  }
}

void motorOff(int predelay=0){
  if(preferences.getBool("En_Motor", false)){
    delay(predelay);
    analogWrite(MOTOR_PIN, 0);
  }
}

void launchLedOn(int predelay=0){
  if(preferences.getBool("En_Lnch_LED", false)){
    delay(predelay);
    digitalWrite(LAUNCH_LED_PIN, HIGH);
  }
}

void launchLedOff(int predelay=0, int postDelay=0){
  if(preferences.getBool("En_Lnch_LED", false)){
    delay(predelay);
    digitalWrite(LAUNCH_LED_PIN, LOW);
    delay(postDelay);
  }
}

void wifiLedOn(){
  if(preferences.getBool("En_Wifi_LED", false)){
    digitalWrite(WIFI_LED_PIN, HIGH);
  }
}

void wifiLedOff(){
  if(preferences.getBool("En_Wifi_LED", false)){
    digitalWrite(WIFI_LED_PIN, LOW);
  }
}

void expressError(int code){
  for(int i=0; i < code; i++){
    motorOn();
    launchLedOn();
    motorOff(800);
    launchLedOff(0, 400);
  }
}

void successActions(){
  launchLedOn(0);
  if(!preferences.getBool("En_SysAudio", false) && !preferences.getBool("En_GameAudio", false)){
    playAudio("Audio_F_Path");
    motorOn(0);
    motorOff(100);
    motorOn(50);
  }else{
    motorOn(0);
    motorOff(100);
    motorOn(50);
  }
  if(preferences.getBool("En_GameAudio", false)){
    //GetGameAudio
    //playSystemAudio("Audio_F_Path");
  }
  if(preferences.getBool("En_SysAudio", false)){
    //GetSystemAudio
    //playGameAudio("Audio_F_Path");
  }
  motorOff(100);
  launchLedOff(0,0);
}

void playAudio(String PrefString){
  if(preferences.getBool("En_Audio", false)){
    String defAudioPath = preferences.getString(PrefString.c_str(), "");
    if(defAudioPath.length() > 0){
      const char* launchAudio = defAudioPath.c_str();
      AudioFileSourceLittleFS* file = new AudioFileSourceLittleFS(launchAudio);
      AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
      mp3->begin(file, out);
      while(mp3->loop()){}
      mp3->stop();
      delete file;
      delete mp3;
    }else{
      delay(1000);
    }
  }
}

void cardInsertedActions(){
  String tmpPath = preferences.getString("Audio_D_Path", "");
  if(tmpPath.length() > 0){
    playAudio("Audio_D_Path");
  }
  motorOn(0);
  motorOff(100);
}

void cardRemovedActions(){
  String tmpPath = preferences.getString("Audio_R_Path", "");
  if(tmpPath.length() > 0){
    playAudio("Audio_R_Path");
  }
  motorOn(0);
  motorOff(100);
}

void notifyClients(String txtMsgToSend) {
  Serial.println(txtMsgToSend);
  if(isWebLog){
    JsonDocument msgJson;
    msgJson["msgType"] = "notify";
    msgJson["data"] = txtMsgToSend;
    String output;
    serializeJson(msgJson, output);
    delay(500);
    ws.textAll(output.c_str());
    if(txtMsgToSend == "closeWS"){
      delay(500);
      ESP.restart();
    }
  }else{
    if(txtMsgToSend == "closeWS"){
      ESP.restart();
    }
  }  
}

void cmdClients(JsonDocument cmdJson){
  String output;
  serializeJson(cmdJson, output);
  Serial.print("WS MSG SENT: ");
  Serial.println(output);
  delay(500);
  ws.textAll(output.c_str());
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String webCmd = String((char *)data);
    JsonDocument root;
    DeserializationError error = deserializeJson(root, webCmd);
    if (error){
      isWebLog = true;
      notifyClients("Failed to Parse JSON");
      return;
    }else{
      isWebLog = true;
      //WebUI has changed to tag creation mode
      if(root["cmd"] == "set_WriteMode"){
        if(root["data"]){
          notifyClients("NFC Tag Write Mode Enabled");
          setPref_Bool("En_NFC_Wr", true);
        }else{
          notifyClients("NFC Tag Write Mode Disabled");
          setPref_Bool("En_NFC_Wr", false);
        }        
      }
      //Write launch game command to tag
      if(root["cmd"] == "write_Tag_Launch_Game"){
        notifyClients("NFG Tag Writing the Launch Game Command");
        String tmpRetDataStr = root["data"];
        if(preferences.getBool("En_NFC_Wr", false)){
          writeTagLaunch(tmpRetDataStr);
        }
      }
      //Get current config data for Web Page
      if(root["cmd"] == "get_Current_Config"){     
        getWebConfigData();
      }
      //Set current config data from Web Page
      if(root["cmd"] == "set_Current_Config"){
        setWebConfigData(root);     
      }
      //Test Launch A game
      if(root["cmd"] == "Test_Tag_Launch_Game"){
        String tmpPath = root["data"];
        notifyClients("Test Launching Game");
        launchGame(tmpPath);     
      }
      //Close WebSocket
      if(root["cmd"] == "closeWS"){
        setPref_Bool("En_NFC_Wr", false);
        ws.closeAll();
        ws.cleanupClients();   
      }
    }
  }
}

void getWebConfigData(){
  notifyClients("Retrieving Current Zap ESP Config Data");
  bool wifi_led_enabled = preferences.getBool("En_Wifi_LED", false);
  bool motor_enabled = preferences.getBool("En_Motor", false);
  bool launch_led_enabled = preferences.getBool("En_Lnch_LED", false);
  bool audio_enabled = preferences.getBool("En_Audio", false);
  bool pwr_led_enabled = preferences.getBool("En_EPwr_LED", false);
  bool reset_on_remove_enabled = preferences.getBool("En_RoR", true);
  String defAudioPath = preferences.getString("Audio_F_Path", "");
  String defDetectAudioPath = preferences.getString("Audio_D_Path", "");
  String defRemoveAudioPath = preferences.getString("Audio_R_Path", "");
  String ZapIP = preferences.getString("ZapIP", "mister.local");
  int pin_I2S_DOUT = preferences.getInt("I2S_DOUT", 25);
  int pin_I2S_BCLK = preferences.getInt("I2S_BCLK", 27);
  int pin_I2S_LRC = preferences.getInt("I2S_LRC", 26);
  float val_AUDIO_GAIN = preferences.getFloat("AUDIO_GAIN", 1);
  int pin_MOTOR_PIN = preferences.getInt("MOTOR_PIN", 32);
  int pin_WIFI_LED_PIN = preferences.getInt("WIFI_LED_PIN", 2);
  int pin_LAUNCH_LED_PIN = preferences.getInt("LAUNCH_LED_PIN", 33);
  int pin_EXTERNAL_POWER_LED = preferences.getInt("EXT_PWRLED_PIN", 22);
  bool mister_enabled = preferences.getBool("En_Mister", true);
  bool steamOS_enabled = preferences.getBool("En_SteamOS", false);
  bool sdCard_enabled = preferences.getBool("En_SDCard", false);
  bool systemAudio_enabled = preferences.getBool("En_SysAudio", false);
  bool gameAudio_enabled = preferences.getBool("En_GameAudio", false);
  String SteamIP = preferences.getString("SteamIP", "steamOS.local");
  JsonDocument configData;
  configData["msgType"] = "ConfigData";
  configData["data"]["wifi_led_enabled"] = wifi_led_enabled;
  configData["data"]["motor_enabled"] = motor_enabled;
  configData["data"]["launch_led_enabled"] = launch_led_enabled;
  configData["data"]["audio_enabled"] = audio_enabled;
  configData["data"]["pwr_led_enabled"] = pwr_led_enabled;
  configData["data"]["reset_on_remove_enabled"] = reset_on_remove_enabled;
  configData["data"]["defAudioPath"] = defAudioPath;
  configData["data"]["ZapIP"] = ZapIP;
  configData["data"]["pin_I2S_DOUT"] = pin_I2S_DOUT;
  configData["data"]["pin_I2S_BCLK"] = pin_I2S_BCLK;
  configData["data"]["pin_I2S_LRC"] = pin_I2S_LRC;
  configData["data"]["val_AUDIO_GAIN"] = val_AUDIO_GAIN;
  configData["data"]["pin_MOTOR_PIN"] = pin_MOTOR_PIN;
  configData["data"]["pin_WIFI_LED_PIN"] = pin_WIFI_LED_PIN;
  configData["data"]["pin_LAUNCH_LED_PIN"] = pin_LAUNCH_LED_PIN;
  configData["data"]["pin_EXTERNAL_POWER_LED"] = pin_EXTERNAL_POWER_LED;
  configData["data"]["mister_enabled"] = mister_enabled;
  configData["data"]["steamOS_enabled"] = steamOS_enabled;
  configData["data"]["sdCard_enabled"] = sdCard_enabled;
  configData["data"]["SteamIP"] = SteamIP;
  configData["data"]["defDetectAudioPath"] = defDetectAudioPath;
  configData["data"]["defRemoveAudioPath"] = defRemoveAudioPath;
  configData["data"]["systemAudio_enabled"] = systemAudio_enabled;
  configData["data"]["gameAudio_enabled"] = gameAudio_enabled;

  cmdClients(configData);
}

void setWebConfigData(JsonDocument cfgData){
  notifyClients("ZAP ESP Now Saving Config Data");
  if(!cfgData["data"]["wifi_led_enabled"]){
    wifiLedOff();
  }
  if(!cfgData["data"]["launch_led_enabled"]){
    launchLedOff();
  }
  if(!cfgData["data"]["pwr_led_enabled"] && preferences.getBool("En_EPwr_LED", false)){
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
  setPref_Bool("En_SysAudio", cfgData["data"]["systemAudio_enabled"]);
  setPref_Bool("En_GameAudio", cfgData["data"]["gameAudio_enabled"]);
  notifyClients("closeWS");
}

void setPref_Bool(const String key, bool valBool){
  preferences.putBool(key.c_str(), valBool);
}

void setPref_Int(const String key, int valInt){
  preferences.putInt(key.c_str(), valInt);
}

void setPref_Str(const String key, String valStr){
  preferences.putString(key.c_str(), valStr);
}

void setPref_Float(const String key, float valFloat){
  preferences.putFloat(key.c_str(), valFloat);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
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


int ZaparooSendCmd(JsonDocument payload){  
  String ZapIP = preferences.getString("ZapIP", "mister.local");
  String newURL = ZAP_URL;
  std::atomic<bool> complete;
  std::atomic<int> lastError;
  newURL.replace("<replace>", ZapIP);
  notifyClients("ZAP URL: " + newURL);
  WebsocketsClient client;
  lastError.store(0);
  complete.store(false);
  UUID uuid;
  const char* id = uuid.toCharArray();
  payload["id"]= uuid.toCharArray();
  client.onMessage([&, &id](WebsocketsMessage msg){
    if(complete.load()) return;
    JsonDocument result;
    DeserializationError error = deserializeJson(result, msg.data());
    if (error) {
      lastError.store(4);
      complete.store(true);
      return;
    }
    const char* resultId = result["id"];
    if(strcmp(id, resultId) != 0) return;
    if(result.containsKey("result")){
      lastError.store(3);
    }
    complete.store(true);
  });
  if(!client.connect(newURL)){
    return 2;
  }
  String request;
  serializeJson(payload, request);
  client.send(request);
  while(!complete.load()){
    client.poll();
  }
  client.close();
  return lastError.load();
}


bool connectWifi()
{  
  if (WiFi.status() == WL_CONNECTED){
    return true;
  }
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int maxRetries = 10;
  while (WiFi.status() != WL_CONNECTED)
  {
    if(preferences.getBool("En_Wifi_LED", false)){
      wifiLedOn();
    }
    delay(500);
    if(preferences.getBool("En_Wifi_LED", false)){
      wifiLedOff();
    }
    Serial.print(".");
    maxRetries--;
    if (maxRetries <= 0)    {
      return false;
    }
  }
  Serial.print("WiFi connected - Zap ESP IP = ");
  Serial.println(WiFi.localIP());
  WiFi.setSleep(false);
  server.begin();
  initWebSocket();
  if(preferences.getBool("En_Motor", false)){
    motorOn();
    motorOff(250);
    motorOn(100);
    motorOff(250);
  }
  if(preferences.getBool("En_Wifi_LED", false)){
    wifiLedOn();
  }
  return true;
}



bool launchGame(String code)
{
  String newCode = code;
  newCode.replace("{\"path\":\"", "");
  newCode.replace("\"}", "");
  notifyClients("Zaparoo Lauch Parameter: " + newCode);
  JsonDocument ZapCmd;
  ZapCmd["jsonrpc"]= "2.0";
  ZapCmd["method"]="launch";
  ZapCmd["id"]= "";
  ZapCmd["params"]["text"] = newCode;
  ZapCmd["params"]["uid"] = "52f6242e-7a5a-11ef-bf93-020304050607"; //fake
  ZapCmd.shrinkToFit();
  int resCode = ZaparooSendCmd(ZapCmd);
  bool launched = resCode == 0;
  if(launched){
    notifyClients("Launched Game");
    return true;
  }else{
    notifyClients("Zaparoo Error Launching Game - code: " + resCode);
    return false;
  }
}

void callResetMister()
{
  JsonDocument ZapCmd;
  ZapCmd["jsonrpc"]= "2.0";
  ZapCmd["method"]="stop";
  ZapCmd["id"]= "";
  ZapCmd.shrinkToFit();
  int resCode = ZaparooSendCmd(ZapCmd);
}

void writeTagLaunch(String launchCmd){
  isWebLog = true;
  String tmpLaunchCmd = launchCmd;
  tmpLaunchCmd.replace("launch_cmd::", "");
  notifyClients("Tag data to Write: " + tmpLaunchCmd);
  if (nfc.tagPresent()) {
    notifyClients("Erasing Tag");
    nfc.erase();
    notifyClients("Writing Data");
    NdefMessage message = NdefMessage();
    message.addTextRecord(tmpLaunchCmd.c_str());
    bool success = nfc.write(message);
    if(success){
      notifyClients("Data sucessfully written. Remove the Tag/Card and disable 'Tag Creation Mode' before testing.");
    }else{
      notifyClients("Data write failed. Remove the Tag/Card and try again.");
    }
  }else{
    notifyClients("No NFC Tag/Card detected - Aborting Write - Please insert a Valid NFC Tag/Card");
  }

}

void readNFC()
{
  if (nfc.tagPresent()) {
    notifyClients("Found Tag");
    cardInsertedActions();
    NfcTag tag = nfc.read();
    String id = tag.getUidString();
    bool foundMessage = false;
    if(id != lastNFC_ID){
      lastNFC_ID = id;
      notifyClients("Tag ID: " + id);
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
        foundMessage = !payloadAsString.equalsIgnoreCase("");
        if(foundMessage){
          notifyClients("Found Msg: SCAN\t" + payloadAsString + "\n");
          bool hasLaunched = launchGame(payloadAsString);
          successActions(); 
          delay(900);
        }else{
          if(idLoop > 3){
            String toSend = id;
            toSend.replace(" ", "");
            toSend.toLowerCase();
            notifyClients("Not Found Msg: SCAN\tuid=" + toSend+ "\n");
            successActions();          
            delay(900);
          }
        }
      }
    }else{
      idLoop++;
      notifyClients("Same Tag ID - doing nothing");
    }
  }else{
    if(lastNFC_ID != "" && !nfc.tagPresent()){
      notifyClients("Tag Removed!");
      cardRemovedActions();
      lastNFC_ID = "";
      idLoop = 0;
      if(preferences.getBool("En_RoR", true)){
        notifyClients("Reseting MiSTer");
        callResetMister();
      }
    }
  }
}

void loop()
{ 
  bool connected = connectWifi();
  if (isConnected != connected){
    isConnected = connected;
  }
  if (isConnected) {
    if(!preferences.getBool("En_NFC_Wr", false)){
        readNFC();
    }
  }
  delay(900);
}