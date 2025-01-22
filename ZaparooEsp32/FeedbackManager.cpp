#include "FeedbackManager.h"
#include "ZaparooEsp32.hpp"

#define UID_MAP_FILE "/uidExtdRecord.json"

FeedbackManager::FeedbackManager() {
    out = nullptr;
}

FeedbackManager::~FeedbackManager() {
    if (out) {
        delete out;
    }
}

void FeedbackManager::init(Preferences* prefs) {
    preferences = prefs;
    audioGain = preferences->getFloat("audioGain", 1.0);
    isWebLog = preferences->getBool("isWebLog", false);
    wifiLed = preferences->getBool("wifiLed", false);
    motorEnabled = preferences->getBool("motor", false);
    launchLed = preferences->getBool("launchLed", false);
    audioEnabled = preferences->getBool("audio", false);
    pwrLed = preferences->getBool("pwrLed", false);
    resetOnRemove = preferences->getBool("resetOnRemove", true);
    sdCardEnabled = preferences->getBool("sdCard", false);
    systemAudioEnabled = preferences->getBool("systemAudio", false);
    gameAudioEnabled = preferences->getBool("gameAudio", false);
    buzzOnDetect = preferences->getBool("buzzOnDetect", true);
    buzzOnLaunch = preferences->getBool("buzzOnLaunch", true);
    buzzOnRemove = preferences->getBool("buzzOnRemove", true);
    buzzOnError = preferences->getBool("buzzOnError", true);
    defaultInsertAudio = (char*)preferences->getString("defaultInsertAudio", "").c_str();
    defaultLaunchAudio = (char*)preferences->getString("defaultLaunchAudio", "").c_str();
    defaultRemoveAudio = (char*)preferences->getString("defaultRemoveAudio", "").c_str();
    defaultErrorAudio = (char*)preferences->getString("defaultErrorAudio", "").c_str();

    motorPin = preferences->getInt("motorPin", -1);
    launchLedPin = preferences->getInt("launchLedPin", -1);
    wifiLedPin = preferences->getInt("wifiLedPin", -1);
    pwrLedPin = preferences->getInt("pwrLedPin", -1);

    // Read the I2S pins from preferences but don't save them in member variables
    i2sBclkPin = preferences->getInt("i2sBclkPin", -1);
    i2sLrcPin = preferences->getInt("i2sLrcPin", -1);
    i2sDoutPin = preferences->getInt("i2sDoutPin", -1);

    setupPins();

}


void FeedbackManager::setupPins() {
  if (motorEnabled) {
    pinMode(motorPin, OUTPUT);
  }
  if (wifiLed) {
    pinMode(wifiLedPin, OUTPUT);
  }
  if (launchLed) {
    pinMode(launchLedPin, OUTPUT);
  }
  if (pwrLed) {
    pinMode(pwrLedPin, OUTPUT);
    digitalWrite(pwrLedPin, HIGH);
  }
  if (audioEnabled) {
    delete out;
    out = NULL;
    out = new AudioOutputI2S();
    out->SetPinout(i2sBclkPin, i2sLrcPin, i2sDoutPin);
    out->SetChannels(1);
    out->SetGain(audioGain);
    if (sdCardEnabled) {
      if (!SD.begin(SS_PIN)) {
        Serial.println(F("failed to do SD Card"));
      }
    }
  }
}

void FeedbackManager::update(JsonDocument& doc) {
    if (doc["data"].containsKey("audioGain")) {
        audioGain = doc["data"]["audioGain"].as<float>();
        preferences->putFloat("audioGain", audioGain);
    }
    if (doc["data"].containsKey("isWebLog")) {
        isWebLog = doc["data"]["isWebLog"].as<bool>();
        preferences->putBool("isWebLog", isWebLog);
    }
    if (doc["data"].containsKey("wifiLed")) {
        wifiLed = doc["data"]["wifiLed"].as<bool>();
        preferences->putBool("wifiLed", wifiLed);
    }
    if (doc["data"].containsKey("motor")) {
        motorEnabled = doc["data"]["motor"].as<bool>();
        preferences->putBool("motor", motorEnabled);
    }
    if (doc["data"].containsKey("launchLed")) {
        launchLed = doc["data"]["launchLed"].as<bool>();
        preferences->putBool("launchLed", launchLed);
    }
    if (doc["data"].containsKey("audioEnabled")) {
        audioEnabled = doc["data"]["audioEnabled"].as<bool>();
        preferences->putBool("audioEnabled", audioEnabled);
    }
    if (doc["data"].containsKey("pwrLed")) {
        pwrLed = doc["data"]["pwrLed"].as<bool>();
        preferences->putBool("pwrLed", pwrLed);
    }
    if (doc["data"].containsKey("resetOnRemove")) {
        resetOnRemove = doc["data"]["resetOnRemove"].as<bool>();
        preferences->putBool("resetOnRemove", resetOnRemove);
    }
    if (doc["data"].containsKey("sdCardEnabled")) {
        sdCardEnabled = doc["data"]["sdCardEnabled"].as<bool>();
        preferences->putBool("sdCardEnabled", sdCardEnabled);
    }
    if (doc["data"].containsKey("systemAudioEnabled")) {
        systemAudioEnabled = doc["data"]["systemAudioEnabled"].as<bool>();
        preferences->putBool("systemAudioEnabled", systemAudioEnabled);
    }
    if (doc["data"].containsKey("gameAudioEnabled")) {
        gameAudioEnabled = doc["data"]["gameAudioEnabled"].as<bool>();
        preferences->putBool("gameAudioEnabled", gameAudioEnabled);
    }
    if (doc["data"].containsKey("buzzOnDetect")) {
        buzzOnDetect = doc["data"]["buzzOnDetect"].as<bool>();
        preferences->putBool("buzzOnDetect", buzzOnDetect);
    }
    if (doc["data"].containsKey("buzzOnLaunch")) {
        buzzOnLaunch = doc["data"]["buzzOnLaunch"].as<bool>();
        preferences->putBool("buzzOnLaunch", buzzOnLaunch);
    }
    if (doc["data"].containsKey("buzzOnRemove")) {
        buzzOnRemove = doc["data"]["buzzOnRemove"].as<bool>();
        preferences->putBool("buzzOnRemove", buzzOnRemove);
    }
    if (doc["data"].containsKey("buzzOnError")) {
        buzzOnError = doc["data"]["buzzOnError"].as<bool>();
        preferences->putBool("buzzOnError", buzzOnError);
    }
    if (doc["data"].containsKey("defaultInsertAudio")) {
        defaultInsertAudio = (char*)doc["data"]["defaultInsertAudio"].as<String>().c_str();
        preferences->putString("defaultInsertAudio", defaultInsertAudio);
    }
    if (doc["data"].containsKey("defaultLaunchAudio")) {
        defaultLaunchAudio = (char*)doc["data"]["defaultLaunchAudio"].as<String>().c_str();
        preferences->putString("defaultLaunchAudio", defaultLaunchAudio);
    }
    if (doc["data"].containsKey("defaultRemoveAudio")) {
        defaultRemoveAudio = (char*)doc["data"]["defaultRemoveAudio"].as<String>().c_str();
        preferences->putString("defaultRemoveAudio", defaultRemoveAudio);
    }
    if (doc["data"].containsKey("defaultErrorAudio")) {
        defaultErrorAudio = (char*)doc["data"]["defaultErrorAudio"].as<String>().c_str();
        preferences->putString("defaultErrorAudio", defaultErrorAudio);
    }

    // Pin assignments - saved to Preferences
    if (doc["data"].containsKey("motorPin")) {
        motorPin = doc["data"]["motorPin"].as<int>();
        preferences->putInt("motorPin", motorPin);
    }
    if (doc["data"].containsKey("launchLedPin")) {
        launchLedPin = doc["data"]["launchLedPin"].as<int>();
        preferences->putInt("launchLedPin", launchLedPin);
    }
    if (doc["data"].containsKey("wifiLedPin")) {
        wifiLedPin = doc["data"]["wifiLedPin"].as<int>();
        preferences->putInt("wifiLedPin", wifiLedPin);
    }
    if (doc["data"].containsKey("pwrLedPin")) {
        pwrLedPin = doc["data"]["pwrLedPin"].as<int>();
        preferences->putInt("pwrLedPin", pwrLedPin);
    }
    if (doc["data"].containsKey("i2sBclkPin")) {
        i2sBclkPin = doc["data"]["i2sBclkPin"].as<int>();
        preferences->putInt("i2sBclkPin", i2sBclkPin);
    }
    if (doc["data"].containsKey("i2sLrcPin")) {
        i2sLrcPin = doc["data"]["i2sLrcPin"].as<int>();
        preferences->putInt("i2sLrcPin", i2sLrcPin);
    }
    if (doc["data"].containsKey("i2sDoutPin")) {
        i2sDoutPin = doc["data"]["i2sDoutPin"].as<int>();
        preferences->putInt("i2sDoutPin", i2sDoutPin);
    }
}


void FeedbackManager::set(JsonDocument& doc) {
    doc["data"]["audioGain"] = audioGain;
    doc["data"]["isWebLog"] = isWebLog;
    doc["data"]["wifiLed"] = wifiLed;
    doc["data"]["motor"] = motorEnabled;
    doc["data"]["launchLed"] = launchLed;
    doc["data"]["audioEnabled"] = audioEnabled;
    doc["data"]["pwrLed"] = pwrLed;
    doc["data"]["resetOnRemove"] = resetOnRemove;
    doc["data"]["sdCardEnabled"] = sdCardEnabled;
    doc["data"]["systemAudioEnabled"] = systemAudioEnabled;
    doc["data"]["gameAudioEnabled"] = gameAudioEnabled;
    doc["data"]["buzzOnDetect"] = buzzOnDetect;
    doc["data"]["buzzOnLaunch"] = buzzOnLaunch;
    doc["data"]["buzzOnRemove"] = buzzOnRemove;
    doc["data"]["buzzOnError"] = buzzOnError;
    doc["data"]["defaultInsertAudio"] = defaultInsertAudio;
    doc["data"]["defaultLaunchAudio"] = defaultLaunchAudio;
    doc["data"]["defaultRemoveAudio"] = defaultRemoveAudio;
    doc["data"]["defaultErrorAudio"] = defaultErrorAudio;
    doc["data"]["motorPin"] = motorPin;
    doc["data"]["launchLedPin"] = launchLedPin;
    doc["data"]["wifiLedPin"] = wifiLedPin;
    doc["data"]["pwrLedPin"] = pwrLedPin;
    doc["data"]["i2sBclkPin"] = i2sBclkPin;
    doc["data"]["i2sLrcPin"] = i2sLrcPin;
    doc["data"]["i2sDoutPin"] = i2sDoutPin;
}


void FeedbackManager::motorOn(int predelay) {
    if (motorEnabled) {
        delay(predelay);
        analogWrite(motorPin, 255);
    }
}

void FeedbackManager::motorOff(int predelay) {
    if (motorEnabled) {
        delay(predelay);
        analogWrite(motorPin, 0);
    }
}

void FeedbackManager::launchLedOn(int predelay) {
    if (launchLed) {
        delay(predelay);
        digitalWrite(launchLedPin, HIGH);
    }
}

void FeedbackManager::launchLedOff(int predelay, int postDelay) {
    if (launchLed) {
        delay(predelay);
        digitalWrite(launchLedPin, LOW);
        delay(postDelay);
    }
}

void FeedbackManager::wifiLedOn() {
    if (wifiLed) {
        digitalWrite(wifiLedPin, HIGH);
    }
}

void FeedbackManager::wifiLedOff() {
    if (wifiLed) {
        digitalWrite(wifiLedPin, LOW);
    }
}

void FeedbackManager::expressError(int code) {
    for (int i = 0; i < code; i++) {
        launchLedOn();
        if (buzzOnError) {
            motorOn(0);
            motorOff(800);
        }
        if (defaultErrorAudio && strlen(defaultErrorAudio) > 0) {
            playAudio(defaultErrorAudio);
        }
        launchLedOff(0, 400);
    }
}

void FeedbackManager::successActions(ZaparooToken* obj) {
    launchLedOn(0);
    const char* pathToPlay = obj->getLaunchAudio();
    if (pathToPlay == nullptr || strlen(pathToPlay) == 0) {
        pathToPlay = defaultLaunchAudio;
    }

    if (pathToPlay != nullptr && strlen(pathToPlay) > 0) {
        if (buzzOnLaunch) {
            motorOn(0);
            motorOff(100);
        }
        playAudio(pathToPlay);
    } else {
        if (buzzOnLaunch) {
            motorOn(0);
            motorOff(1000);
        }
    }
    launchLedOff(0, 0);
}

void FeedbackManager::setUidAudioMappings(ZaparooToken* obj) {
    if (!audioEnabled) return;
    File file;
    JsonDocument uidFile;

    if (sdCardEnabled && SD.exists(UID_MAP_FILE)) {
        file = SD.open(UID_MAP_FILE, FILE_READ);
    } else if (LittleFS.exists(UID_MAP_FILE)) {
        file = LittleFS.open(UID_MAP_FILE, "r");
    } else {
        return;
    }

    String uid = String(obj->getId());
    while (file.available()) {
        DeserializationError error = deserializeJson(uidFile, file);
        if (!error) {
            for (JsonObject item : uidFile["UID_ExtdRecs"].as<JsonArray>()) {
                if (item["UID"] == uid) {
                    obj->setLaunchAudio(item["launchAudio"]);
                    obj->setRemoveAudio(item["removeAudio"]);
                    file.close();
                    return;
                }
            }
        }
    }
    file.close();
}

void FeedbackManager::getUidMappings(JsonDocument& toSet){
  JsonDocument parsed;
  File file;
  toSet["msgType"] = "getUIDExtdRec";
  if(sdCardEnabled && SD.exists(UID_MAP_FILE)){
    file = SD.open(UID_MAP_FILE, FILE_READ);
  }else if(LittleFS.exists(UID_MAP_FILE)){
    file = LittleFS.open(UID_MAP_FILE, "r");
  }
  while (file.available()) { 
    DeserializationError error = deserializeJson(parsed, file);
    if(!error){
      toSet["data"] = parsed;
      file.close();
      return;
    }
  }
  file.close();
  return;
}

int FeedbackManager::playAudio(const char* audioPath) {
    if (!audioEnabled || !audioPath || strlen(audioPath) == 0) {
        delay(1000);
        return 0;
    }

    AudioFileSource* file = nullptr;
    if (sdCardEnabled) {
        AudioFileSourceSD* source = new AudioFileSourceSD(audioPath);
        file = source;
    } else {
        AudioFileSourceLittleFS* source = new AudioFileSourceLittleFS(audioPath);
        file = source;
    }

    if (file->getSize() == 0) {
        delete file;
        file = nullptr;
        return 1;
    }

    AudioGeneratorMP3* mp3 = new AudioGeneratorMP3();
    mp3->begin(file, out);
    while (mp3->loop()) {}
    mp3->stop();

    delete mp3;
    mp3 = NULL;
    delete file;
    file = NULL;
    return 0;
}

void FeedbackManager::cardInsertedActions(ZaparooToken* obj) {
    const char* pathToPlay = obj->getDetectAudio();
    if (!pathToPlay || strlen(pathToPlay) == 0) {
        pathToPlay = defaultInsertAudio;
    }
    if (pathToPlay && strlen(pathToPlay) > 0) {
        playAudio(pathToPlay);
    }
    if (buzzOnDetect) {
        motorOn(0);
        motorOff(100);
    }
}

void FeedbackManager::cardRemovedActions(ZaparooToken* obj) {
    const char* pathToPlay = obj->getRemoveAudio();
    if (!pathToPlay || strlen(pathToPlay) == 0) {
        pathToPlay = defaultRemoveAudio;
    }
    if (pathToPlay && strlen(pathToPlay) > 0) {
        playAudio(pathToPlay);
    }
    if (buzzOnRemove) {
        motorOff();
    }
}
