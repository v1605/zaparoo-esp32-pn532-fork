#include <Arduino.h>
#include "ZaparooScanner.cpp"
#include "ZaparooToken.h"
#include <MFRC522.h>

#include <NfcAdapter.h>

class ZaparooRC522Scanner : public ZaparooScanner {
public:

    ZaparooRC522Scanner(){}

    ~ZaparooRC522Scanner() override {
      delete nfc;
    }

    bool init() override{
      if(nfc){
        delete nfc;
      }
      nfc = new NfcAdapter(config);
      nfc->begin();
      return true;
    }

    bool tokenPresent() override {
      bool result = nfc->tagPresent();
      if(!result){
        //tagPresent returns true once, until a new tag is added. These ensure old tag is present
        result = config->PICC_IsNewCardPresent() || config->PICC_ReadCardSerial();
      }else{
        delay(500); //Helps with deduplicate reads
      }
      return result;
    }

    //There was an attempt to use the result of tokenPresent to determine this
    //For some reason, reading the id this way prevents an error where a tag cannot be re-read after a few attempts
    bool isNewToken() override{
      String id = getUidString();
      if(id == ""){
        Serial.println("Blank id, reset");
        reset();
        nfc->tagPresent();
        id = getUidString();
      }
      bool result = id != lastId;
      if(result){
        lastId = id;
      }
      return result;
    }

    ZaparooToken getToken() override {
        ZaparooToken token;
        NfcTag* tag = new NfcTag(nfc->read());
        //Seems like every few reads (semi-random) it needs a reset to read a record, never seems to need more than one reset
        for (int i = 0; i < 2 && !tag->hasNdefMessage() ; i++) {
          Serial.println("Entered the ndef loop");
          delete tag;
          reset();
          nfc->tagPresent(); //Ensure registers are setup again for lib
          tag = new NfcTag(nfc->read()); //Using pointers to prevent heap errors
        }
        String id = tag->getUidString();
        id.replace(" ", "");
        id.toLowerCase();
        token.setId(id.c_str());
        if(tag->hasNdefMessage()){
          NdefMessage message = tag->getNdefMessage();
          int recordCount = message.getRecordCount();
          if(recordCount == 0){
            delete tag;
            return token;
          }
          token.setPayload(parseNdfMessage(message, token , 0));
          if(!token.getValid()){
            delete tag;
            return token;
          }
          if(recordCount > 1){
            token.setLaunchAudio(parseNdfMessage(message, token , 1));
          }
          if(recordCount > 2){
            token.setRemoveAudio(parseNdfMessage(message, token , 2));
          }
        }
        delete tag;
        return token;
    }

    void halt() override {
        clearCache();
        //reset();
        config->uid.size = 0;
        delay(50);
        //config->MIFARE_Read(255, ZEROES, &ZEROES[0]);
        //delay(1000); //Long delay helps fix issue with duplicated reads with quick tag swap
    }

    void reset(){
        nfc->haltTag();
        config->PCD_Reset();
        config->PCD_Init();
        /*for (byte i = 0; i < config->uid.size; i++) {
           config->uid.uidByte[i] = 0;  // Optionally clear the UID bytes
        }
        config->uid.size = 0;
        Serial.print("UID size before reset: ");
        Serial.println(config->uid.size);
        config->PCD_Reset();
        config->PCD_Init();
        Serial.print("UID size after reset: ");
        Serial.println(config->uid.size);
        delay(5000);*/
    }

    void clearCache() override{
      lastId.clear();
    }

    // Write a token to the given device
    bool writeLaunch(String& launchCmd, String& audioLaunchFile, String& audioRemoveFile) override {
        if (nfc->tagPresent()) {
          nfc->erase();
          NdefMessage message = NdefMessage();
          message.addTextRecord(launchCmd.c_str());
          if(audioLaunchFile.length() > 0){
            message.addTextRecord(audioLaunchFile.c_str());
          }
          if(audioRemoveFile.length() > 0){
            message.addTextRecord(audioRemoveFile.c_str());
          }
          bool success = nfc->write(message);
          if(!success){
            reset();
          }
          return success;
        }
        return false;
    }

    bool supportsWrite() const override {
        return true;
    }

    void setConfig(MFRC522& config){
      this->config = &config;
    }

private:
    MFRC522* config=NULL;
    String lastId = "";
    NfcAdapter* nfc = NULL;

    const char* parseNdfMessage(NdefMessage& message, ZaparooToken& currentToken ,int recordIndex){
      NdefRecord record = message.getRecord(recordIndex);
      int payloadLength = record.getPayloadLength();
      const byte *payload = record.getPayload();
      String payloadAsString = "";
      for (int i = 3; i < payloadLength; i++) {
            int tmpIntChar = payload[i];
            if(!isAscii(tmpIntChar)){
              currentToken.setValid(false);
              return "";
            }
            payloadAsString += (char)payload[i];
      }
       return payloadAsString.c_str();
    } 

    /*String getUidString() {
      byte *uid = config->uid.uidByte;
      uint8_t  uidLength = config->uid.size;
      String uidString = "";
      for (unsigned int i = 0; i < uidLength; i++){
          if (uid[i] < 0xF){
              uidString += "0";
          }
          uidString += String((unsigned int)uid[i], (unsigned char)HEX);
      }
      return uidString;
    }*/

    String getUidString() {
      byte* uid = config->uid.uidByte;
      uint8_t uidLength = config->uid.size;
      char uidString[2 * uidLength + 1] = {0};
      for (unsigned int i = 0; i < uidLength; i++) {
          char hex[3]; 
          if (uid[i] < 0x10) {
              snprintf(hex, sizeof(hex), "0%X", uid[i]);
          } else {
              snprintf(hex, sizeof(hex), "%X", uid[i]);
          }
          strcat(uidString, hex);
      }
    return String(uidString);
}

};
