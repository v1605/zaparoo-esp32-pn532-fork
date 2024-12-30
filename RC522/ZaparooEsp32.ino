#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <SPI.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <LittleFS.h>
#include <ESPWebFileManager.h>
#include <MFRC522.h>
#include <NfcAdapter.h>
#include <ZaparooLaunchApi.h>
#include <ZaparooRestResult.h>
#include "index.h"
#include "qr_code_js.h"
#include "ZaparooEsp32.hpp"


MFRC522 mfrc522(SS_PIN, RST_PIN);
NfcAdapter nfc = NfcAdapter(&mfrc522);
//AudioFileSourceSD *source = NULL;
AudioOutputI2S* out;
Preferences preferences;
using namespace websockets;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
ESPWebFileManager fileManager;
ZaparooLaunchApi ZapClient;

//globals
String ZAP_URL = "ws://<replace>:7497";
String lastNFC_ID = "";
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
  //pinMode(PN532_RST_PIN, OUTPUT);
  //digitalWrite(PN532_RST_PIN, HIGH);
  preferences.begin("qrplay", false);
  setPref_Bool("En_NFC_Wr", false);
  SPI.begin();
  mfrc522.PCD_Init();
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
    // if(preferences.getBool("En_SDCard", false)){
    //   if (!SD.begin(SS_PIN)) { 
    //     Serial.println(F("failed to do SD Card"));
    //   }
    // }
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
        // File file = SD.open(launchAudio);
        // if(file){
        //   source = new AudioFileSourceSD();
        //   if(source->open(launchAudio)){
        //     AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
        //     mp3->begin(source, out);
        //     while(mp3->loop()){}
        //     mp3->stop();
        //     delete mp3;
        //   }
        // }
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
  if(tmpPath.length() > 0){
    playAudio(tmpPath);
  }
  motorOn(0);
  motorOff(100);
}

void cardRemovedActions(String audioRemPath){
  String tmpPath = audioRemPath;
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
    newURL.replace("<replace>", SteamIP);
  }else{
    newURL.replace("<replace>", ZapIP);
  }
  ZapClient.url(newURL);
  int code = ZapClient.launchUid(uid);
  if(code > 0){
    expressError(code);
    notifyClients("Zaparoo Error Sending Card/Tag UID - Error Code: " + code);
  }
  notifyClients("Sent Card/Tag UID");
  return code == 0;
}

void writeTagLaunch(String launchCmd, String audioLaunchFile, String audioRemoveFile){
  notifyClients("Starting Card Writer");
  nfc.begin();
  String tmpLaunchCmd = launchCmd;
  String tmpAudioFile = audioLaunchFile;
  String tmpAudioRemFile = audioRemoveFile;
  tmpLaunchCmd.replace("launch_cmd::", "");
  if (nfc.tagPresent()) {
    nfc.erase();
    NdefMessage message = NdefMessage();
    message.addTextRecord(tmpLaunchCmd.c_str());
    if(tmpAudioFile.length() > 0){
      message.addTextRecord(tmpAudioFile.c_str());
    }
    if(tmpAudioRemFile.length() > 0){
      message.addTextRecord(tmpAudioRemFile.c_str());
    }
    bool success = nfc.write(message);
    if(success){
      notifyClients("Data sucessfully written. Remove the Tag/Card and close 'Creation Mode' before testing.");
    }else{
      notifyClients("Data write failed. Resetting the NFC device! Remove the Tag/Card and try again.");
      //resetPN532();
    }
  }else{
    notifyClients("No NFC Tag/Card detected - Aborting Write - Please insert a Valid NFC Tag/Card");
  }
  nfc.haltTag();
}

void resetPN532(){
  // digitalWrite(PN532_RST_PIN, LOW);
  // delay(100);
  // digitalWrite(PN532_RST_PIN, HIGH);
}


bool readNFC()
{
  notifyClients("Starting Card Reader");
  nfc.begin();
  idLoop = 0;
  String payloadAsString = "";
  String payloadAsString2 = "";
  String payloadAsString3 = "";
  while(!preferences.getBool("En_NFC_Wr", false)){
    if (nfc.tagPresent()) {
      NfcTag tag = nfc.read();
      String id = tag.getUidString();
      bool foundMessage = false;
      bool foundMessage2 = false;
      bool foundMessage3 = false;
      bool badRead = false;
      if(id != lastNFC_ID){
        cardInsertedActions();
        lastNFC_ID = id;
        if(tag.hasNdefMessage()){
          NdefMessage message = tag.getNdefMessage();
          int recordCount = message.getRecordCount();
          NdefRecord record = message.getRecord(0);
          int payloadLength = record.getPayloadLength();
          // byte payload[payloadLength];
          // record.getPayload(payload);
          const byte *payload = record.getPayload();
          payloadAsString = "";
          for (int i = 3; i < payloadLength; i++) {
                int tmpIntChar = payload[i];
                if(!isAscii(tmpIntChar)){
                  //bad read restart
                  id = "";
                  badRead = true;
                }
                payloadAsString += (char)payload[i];
          }
          foundMessage = !payloadAsString.equalsIgnoreCase("");
          payloadAsString2 = "";
          if(foundMessage){
            //now check for launch audio file path record on the tag (this could be cleaner)
            if(recordCount > 0) {
              NdefRecord record = message.getRecord(1);
              int payloadLength2 = record.getPayloadLength();
              // byte payload2[payloadLength2];
              // record.getPayload(payload2);
              const byte *payload2 = record.getPayload();
              for (int i = 3; i < payloadLength2; i++) {
                    payloadAsString2 += (char)payload2[i];
              }
              foundMessage2 = !payloadAsString.equalsIgnoreCase("");
            }
            //now check for remove audio file path record on the tag
            if(recordCount > 1) {
              NdefRecord record = message.getRecord(2);
              int payloadLength3 = record.getPayloadLength();
              // byte payload3[payloadLength3];
              // record.getPayload(payload3);
              const byte *payload3 = record.getPayload();
              payloadAsString3 = "";
              for (int i = 3; i < payloadLength3; i++) {
                    payloadAsString3 += (char)payload3[i];
              }
              foundMessage3 = !payloadAsString.equalsIgnoreCase("");
            }else{
              payloadAsString3 = "";
            }            
            if(!badRead){
              if(!SERIAL_ONLY){
                if(send(payloadAsString)){
                  successActions(payloadAsString2);
                  notifyClients("Sent: " + payloadAsString);
                }
              }else{
                Serial.print("SCAN\t" + payloadAsString + "\n");
                Serial.flush();
                successActions(payloadAsString2);
              }
            }
          }else{
            if(idLoop > 3){
              String toSend = id;
              toSend.replace(" ", "");
              toSend.toLowerCase();
              if(!SERIAL_ONLY){
                if(sendUid(toSend)){
                  successActions("");
                  notifyClients("Sent: " + toSend);
                }
              }else{
                Serial.print("SCAN\tuid=" + toSend+ "\n");
                Serial.flush();
              }          
            }
          }
        }
      }else{
        //Same Tag ID - doing nothing except do soft pn532 reset every 500 read cycles to avoid timeout
        // if(idLoop == 500){
        //   softReset = true;
        //   resetPN532();
        //   idLoop = 4;
        // }
        idLoop++;      
      }
    }else{
      if(lastNFC_ID != "" && !nfc.tagPresent() && !softReset){
        //trigger a remove event;
        cardRemovedActions(payloadAsString3);
        if(reset_on_remove_enabled && !payloadAsString.startsWith("steam://") && !SERIAL_ONLY){
          ZapClient.stop();
        }
        notifyClients("Tag Removed!");
        lastNFC_ID = "";
        idLoop = 0;
        nfc.haltTag();        
        return true;
      }else if (lastNFC_ID != "" && !nfc.tagPresent() & softReset){
        //softreset has been run to keep pn532 alive - clear vars and continue but do not trigger a remove event; 
        softReset = false;
        nfc.haltTag();
        return true;
      }
    }
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
  delay(500);
}