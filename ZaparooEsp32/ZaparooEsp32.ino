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
#include <ZaparooLaunchApi.h>
#include <ZaparooRestResult.h>
#include "index.h"
#include "qr_code_js.h"
#include "ZaparooEsp32.hpp"
#include "ZaparooScanner.cpp"

#ifdef PN532
  #include "scanners\ZaparooPN532Scanner.cpp"
  PN532_I2C pn532_i2c(Wire);
#endif

#ifdef RC522
  #include "ZaparooRC522Scanner.cpp"
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
AudioFileSourceSD* source = NULL;
ZaparooScanner* tokenScanner = NULL;

//globals
String ZAP_URL = "ws://<replace>:7497";
ZaparooToken token;
bool inserted = false;
bool isPN532 = false;
int idLoop = 0;
int timeoutLoop =0;
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
String SteamIP = "steamOS.local";
String defAudioPath = "";
String defDetectAudioPath = "";
String defRemoveAudioPath = "";
String ZapIP = "mister.local";



void setup()
{
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
  tokenScanner->init();
  preferences.begin("qrplay", false);
  setPref_Bool("En_NFC_Wr", false);
  setupPins();
  bool connected = connectWifi();
  
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
  ZapIP = preferences.getString("ZapIP", "mister.local");
  val_AUDIO_GAIN = preferences.getFloat("AUDIO_GAIN", 1);
  mister_enabled = preferences.getBool("En_Mister", true);
  steamOS_enabled = preferences.getBool("En_SteamOS", false);
  sdCard_enabled = preferences.getBool("En_SDCard", false);
  systemAudio_enabled = preferences.getBool("En_SysAudio", false);
  gameAudio_enabled = preferences.getBool("En_GameAudio", false);
  SteamIP = preferences.getString("SteamIP", "steamOS.local");

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
    #define EXTERNAL_POWER_LED preferences.getInt("EXT_PWRLED_PIN", 15)
    pinMode(EXTERNAL_POWER_LED, OUTPUT);
    digitalWrite(EXTERNAL_POWER_LED, HIGH);
  }
  if(preferences.getBool("En_Audio", false)){
    #define I2S_DOUT preferences.getInt("I2S_DOUT", 25)
    #define I2S_BCLK preferences.getInt("I2S_BCLK", 27)
    #define I2S_LRC preferences.getInt("I2S_LRC", 26)
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetChannels(1);
    out->SetGain(preferences.getFloat("AUDIO_GAIN", 1));
    #ifdef PN532
      if(preferences.getBool("En_SDCard", false)){
        if (!SD.begin(SS_PIN)) { 
          Serial.println(F("failed to do SD Card"));
        }
      }
    #endif
  }
}

void motorOn(int predelay=0){
  if(motor_enabled){
    delay(predelay);
    analogWrite(MOTOR_PIN, 255);
  }
}

void motorOff(int predelay=0){
  if(motor_enabled){
    delay(predelay);
    analogWrite(MOTOR_PIN, 0);
  }
}

void launchLedOn(int predelay=0){
  if(launch_led_enabled){
    delay(predelay);
    digitalWrite(LAUNCH_LED_PIN, HIGH);
  }
}

void launchLedOff(int predelay=0, int postDelay=0){
  if(launch_led_enabled){
    delay(predelay);
    digitalWrite(LAUNCH_LED_PIN, LOW);
    delay(postDelay);
  }
}

void wifiLedOn(){
  if(wifi_led_enabled){
    digitalWrite(WIFI_LED_PIN, HIGH);
  }
}

void wifiLedOff(){
  if(wifi_led_enabled){
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

void successActions(String audioPath){
  launchLedOn(0);  
  if(audioPath.length() > 0){    
    playAudio(audioPath);
    motorOn(0);
    motorOff(100);
    motorOn(50);
  }else{
    String tmpAudioPath = defAudioPath;
    if(tmpAudioPath.length() > 0) {
      playAudio(tmpAudioPath);
    }
    motorOn(0);
    motorOff(100);
    motorOn(50);
  }
  motorOff(100);
  launchLedOff(0,0);
}

void playAudio(String PrefString){
  if(audio_enabled){
    if(PrefString.length() > 0){
      const char* launchAudio = PrefString.c_str();
      if(sdCard_enabled){
          File file = SD.open(launchAudio);
          if(file){
            source = new AudioFileSourceSD();
            if(source->open(launchAudio)){
              AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
              mp3->begin(source, out);
              while(mp3->loop()){}
              mp3->stop();
              delete mp3;
            }
            delete source;
          }else{
            notifyClients("Audio file did not exist, check path: " + PrefString);
            return;
          }
      }else{        
        AudioFileSourceLittleFS* file = new AudioFileSourceLittleFS(launchAudio);
        AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
        mp3->begin(file, out);
        while(mp3->loop()){}
        mp3->stop();
        delete file;
        delete mp3;
      }
      
    }else{
      delay(1000);
    }
  }
}

void cardInsertedActions(){
  String tmpPath = defDetectAudioPath;
  inserted = true;
  if(tmpPath.length() > 0){
    playAudio(tmpPath);
  }
  motorOn(0);
  motorOff(100);
}

void cardRemovedActions(String audioRemPath){
  String tmpPath = audioRemPath;
  inserted = false;
  if(audioRemPath.length() > 0){
    playAudio(tmpPath);
  }else{
    String tmpPath = defRemoveAudioPath;
    if(tmpPath.length() > 0){
      playAudio(tmpPath);
    }
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
    delay(200);
    ws.textAll(output.c_str());
    if(txtMsgToSend == "closeWS"){
      delay(100);
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
  delay(100);
  ws.textAll(output.c_str());
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

bool connectWifi()
{  
  if (WiFi.status() == WL_CONNECTED){
    return true;
  }
  WiFi.begin(ssid, password);
  int maxRetries = 10;
  while (WiFi.status() != WL_CONNECTED)
  {
    if(wifi_led_enabled){
      wifiLedOn();
    }
    delay(500);
    if(wifi_led_enabled){
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
  if(motor_enabled){
    motorOn();
    motorOff(250);
    motorOn(100);
    motorOff(250);
  }
  if(wifi_led_enabled){
    wifiLedOn();
  }
  return true;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String webCmd = String((char *)data);
    JsonDocument root;
    DeserializationError error = deserializeJson(root, webCmd);
    if (error){      
      notifyClients("Failed to Parse JSON");
      return;
    }else{
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
        String tmpRetDataStr = root["launchData"];
        String tmpRetAudioLaunchStr = root["audioLaunchPath"];
        String tmpRetAudioRemStr = root["audioRemovePath"];
        if(preferences.getBool("En_NFC_Wr", false)){
          writeTagLaunch(tmpRetDataStr, tmpRetAudioLaunchStr, tmpRetAudioRemStr);
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
        send(tmpPath);     
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
  configData["data"]["systemAudio_enabled"] = preferences.getBool("En_SysAudio", false);
  configData["data"]["gameAudio_enabled"] = preferences.getBool("En_GameAudio", false);
  configData["data"]["PN532_module"] = isPN532;
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

bool send(String& gamePath){
  String newURL = ZAP_URL;
  if(gamePath.startsWith("steam://")){
    newURL.replace("<replace>", SteamIP);
  }else{
    newURL.replace("<replace>", ZapIP);
  }
  ZapClient.url(newURL);
  int code = ZapClient.launch(gamePath);
  if(code > 0){
    expressError(code);
    notifyClients("Zaparoo Error Launching Game - Error Code: " + code);
  }
  notifyClients("Launched Game: " + gamePath);
  return code == 0;
}

bool sendUid(String& uid){
  //not possible to determine if steam game from UID so always default to MiSTer if enabled
  String newURL = ZAP_URL;
  if(mister_enabled){
     newURL.replace("<replace>", ZapIP);
  }else{
    newURL.replace("<replace>", SteamIP);
  }
  ZapClient.url(newURL);
  int code = ZapClient.launchUid(uid);
  if(code > 0){
    expressError(code);
    notifyClients("Zaparoo Error Sending Card/Tag UID - Error Code: " + code);
  }else{
    notifyClients("Sent Card/Tag UID");
  }
  return code == 0;
}


void writeTagLaunch(String launchCmd, String audioLaunchFile, String audioRemoveFile){
  String tmpLaunchCmd = launchCmd;
  tmpLaunchCmd.replace("launch_cmd::", "");
  if (tokenScanner->tokenPresent()) {
    bool success = tokenScanner->writeLaunch(launchCmd, audioLaunchFile, audioRemoveFile);
    if(success){
      notifyClients("Data sucessfully written. Remove the Tag/Card and close 'Creation Mode' before testing.");
    }else{
      notifyClients("Data write failed. Resetting the NFC device! Remove the Tag/Card and try again.");
    }
  }else{
    notifyClients("No NFC Tag/Card detected - Aborting Write - Please insert a Valid NFC Tag/Card");
  }
  tokenScanner->halt();
}


bool readNFC() {
  //while(!preferences.getBool("En_NFC_Wr", false)){
  for(int i=0; i < 20 && !preferences.getBool("En_NFC_Wr", false); i++){
    if (tokenScanner->tokenPresent()) {
      if(tokenScanner->isNewToken()){
        idLoop = 0;
        ZaparooToken parsed = tokenScanner->getToken();
        if(!parsed.getValid()){
          inserted = false;
          delay(10);
          continue;
        }
        token = parsed;
        cardInsertedActions();
        if(token.isPayloadSet()){
            String payload = String(token.getPayload());
            bool sent = true;
            if(!SERIAL_ONLY){
              sent = send(payload);
            }else{
              Serial.print("SCAN\t" + payload + "\n");
              Serial.flush();
            }
            if(sent){
              String audio = "";
              if(token.isLaunchAudioSet()){
                audio = String(token.getLaunchAudio());
              }
               successActions(audio);
            }
          }else if(token.isIdSet()){
            String toSend = String(token.getId());
            toSend.toLowerCase();
            bool sent = true;
            if(!SERIAL_ONLY){
              sent = sendUid(toSend);
            }else{
              Serial.print("SCAN\tuid=" + toSend+ "\n");
              Serial.flush();
            }
            if(sent){
            successActions("");
            notifyClients("Sent: " + toSend);
            }   
          }
      }
    }else if(inserted){ //Must have been removed
      String removeAudio = "";
      if(token.isRemoveAudioSet()){
        removeAudio = token.getRemoveAudio();
      }
      cardRemovedActions(removeAudio);
      bool isSteamPayload = false;
      if(reset_on_remove_enabled && !SERIAL_ONLY && token.isPayloadSet()){
        String payloadAsString = String(token.getPayload());
        if(!payloadAsString.startsWith("steam://")){
          ZapClient.stop();
        }
      }
      notifyClients("Tag Removed!");
      idLoop = 0;
      tokenScanner->halt();
      return true;
    }
    delay(10);
  }
  return false;
}

void loop()
{ 
  bool connected = connectWifi();
  if (isConnected != connected){
    isConnected = connected;
  }
  if (isConnected) {
    if(!preferences.getBool("En_NFC_Wr", false)){
        bool bReadCard = readNFC();
    }
  }
  delay(50);
}