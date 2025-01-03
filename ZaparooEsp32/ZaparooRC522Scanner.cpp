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
      return result;
    }

    bool isNewToken() override{
      NfcTag tag = nfc->read();
      String id = tag.getUidString();
      id.replace(" ", "");
      if(id == ""){
        Serial.println("Id blank");
      }
      bool result = id != lastId;
      if(result){
        lastId = id;
      }
      return result;
    }

    ZaparooToken getToken() override {
        ZaparooToken token;
        NfcTag tag = nfc->read();
        String id = tag.getUidString();
        id.replace(" ", "");
        token.setId(id.c_str());
        if(tag.hasNdefMessage()){
          NdefMessage message = tag.getNdefMessage();
          int recordCount = message.getRecordCount();
          if(recordCount == 0) return token;
          token.setPayload(parseNdfMessage(message, token , 0));
          if(!token.getValid()){
            return token;
          }
          if(recordCount > 1){
            token.setLaunchAudio(parseNdfMessage(message, token , 1));
          }
          if(recordCount > 2){
            token.setRemoveAudio(parseNdfMessage(message, token , 2));
          }
        }
        return token;
    }

    void halt() override {
        clearCache();
    }

    void clearCache() override{
      lastId = "";
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
    MFRC522* config;
    String lastId = "";
    NfcAdapter* nfc = nullptr;

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
};
