#include "FeedbackManager.h"

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
    int i2sBclkPin = preferences->getInt("i2sBclkPin", -1);
    int i2sLrcPin = preferences->getInt("i2sLrcPin", -1);
    int i2sDoutPin = preferences->getInt("i2sDoutPin", -1);

    if (audioEnabled) {
        out = new AudioOutputI2S();
        out->SetPinout(i2sBclkPin, i2sLrcPin, i2sDoutPin);
        out->SetChannels(1);
        out->SetGain(audioGain);
    }
}

void FeedbackManager::update(JsonDocument& doc) {
    if (doc.containsKey("audioGain")) {
        audioGain = doc["audioGain"].as<float>();
        preferences->putFloat("audioGain", audioGain);
    }
    if (doc.containsKey("isWebLog")) {
        isWebLog = doc["isWebLog"].as<bool>();
        preferences->putBool("isWebLog", isWebLog);
    }
    if (doc.containsKey("wifiLed")) {
        wifiLed = doc["wifiLed"].as<bool>();
        preferences->putBool("wifiLed", wifiLed);
    }
    if (doc.containsKey("motor")) {
        motorEnabled = doc["motor"].as<bool>();
        preferences->putBool("motor", motorEnabled);
    }
    if (doc.containsKey("launchLed")) {
        launchLed = doc["launchLed"].as<bool>();
        preferences->putBool("launchLed", launchLed);
    }
    if (doc.containsKey("audioEnabled")) {
        audioEnabled = doc["audioEnabled"].as<bool>();
        preferences->putBool("audioEnabled", audioEnabled);
    }
    if (doc.containsKey("pwrLed")) {
        pwrLed = doc["pwrLed"].as<bool>();
        preferences->putBool("pwrLed", pwrLed);
    }
    if (doc.containsKey("resetOnRemove")) {
        resetOnRemove = doc["resetOnRemove"].as<bool>();
        preferences->putBool("resetOnRemove", resetOnRemove);
    }
    if (doc.containsKey("sdCardEnabled")) {
        sdCardEnabled = doc["sdCardEnabled"].as<bool>();
        preferences->putBool("sdCardEnabled", sdCardEnabled);
    }
    if (doc.containsKey("systemAudioEnabled")) {
        systemAudioEnabled = doc["systemAudioEnabled"].as<bool>();
        preferences->putBool("systemAudioEnabled", systemAudioEnabled);
    }
    if (doc.containsKey("gameAudioEnabled")) {
        gameAudioEnabled = doc["gameAudioEnabled"].as<bool>();
        preferences->putBool("gameAudioEnabled", gameAudioEnabled);
    }
    if (doc.containsKey("buzzOnDetect")) {
        buzzOnDetect = doc["buzzOnDetect"].as<bool>();
        preferences->putBool("buzzOnDetect", buzzOnDetect);
    }
    if (doc.containsKey("buzzOnLaunch")) {
        buzzOnLaunch = doc["buzzOnLaunch"].as<bool>();
        preferences->putBool("buzzOnLaunch", buzzOnLaunch);
    }
    if (doc.containsKey("buzzOnRemove")) {
        buzzOnRemove = doc["buzzOnRemove"].as<bool>();
        preferences->putBool("buzzOnRemove", buzzOnRemove);
    }
    if (doc.containsKey("buzzOnError")) {
        buzzOnError = doc["buzzOnError"].as<bool>();
        preferences->putBool("buzzOnError", buzzOnError);
    }
    if (doc.containsKey("defaultInsertAudio")) {
        defaultInsertAudio = (char*)doc["defaultInsertAudio"].as<String>().c_str();
        preferences->putString("defaultInsertAudio", defaultInsertAudio);
    }
    if (doc.containsKey("defaultLaunchAudio")) {
        defaultLaunchAudio = (char*)doc["defaultLaunchAudio"].as<String>().c_str();
        preferences->putString("defaultLaunchAudio", defaultLaunchAudio);
    }
    if (doc.containsKey("defaultRemoveAudio")) {
        defaultRemoveAudio = (char*)doc["defaultRemoveAudio"].as<String>().c_str();
        preferences->putString("defaultRemoveAudio", defaultRemoveAudio);
    }
    if (doc.containsKey("defaultErrorAudio")) {
        defaultErrorAudio = (char*)doc["defaultErrorAudio"].as<String>().c_str();
        preferences->putString("defaultErrorAudio", defaultErrorAudio);
    }

    // Pin assignments - saved to Preferences
    if (doc.containsKey("motorPin")) {
        motorPin = doc["motorPin"].as<int>();
        preferences->putInt("motorPin", motorPin);
    }
    if (doc.containsKey("launchLedPin")) {
        launchLedPin = doc["launchLedPin"].as<int>();
        preferences->putInt("launchLedPin", launchLedPin);
    }
    if (doc.containsKey("wifiLedPin")) {
        wifiLedPin = doc["wifiLedPin"].as<int>();
        preferences->putInt("wifiLedPin", wifiLedPin);
    }
    if (doc.containsKey("pwrLedPin")) {
        pwrLedPin = doc["pwrLedPin"].as<int>();
        preferences->putInt("pwrLedPin", pwrLedPin);
    }
    if (doc.containsKey("i2sBclkPin")) {
        int i2sBclkPin = doc["i2sBclkPin"].as<int>();
        preferences->putInt("i2sBclkPin", i2sBclkPin);
    }
    if (doc.containsKey("i2sLrcPin")) {
        int i2sLrcPin = doc["i2sLrcPin"].as<int>();
        preferences->putInt("i2sLrcPin", i2sLrcPin);
    }
    if (doc.containsKey("i2sDoutPin")) {
        int i2sDoutPin = doc["i2sDoutPin"].as<int>();
        preferences->putInt("i2sDoutPin", i2sDoutPin);
    }
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
    const char* uidExtdRecFile = "/uidExtdRecord.json";
    File file;
    JsonDocument uidFile;

    if (sdCardEnabled && SD.exists(uidExtdRecFile)) {
        file = SD.open(uidExtdRecFile, FILE_READ);
    } else if (LittleFS.exists(uidExtdRecFile)) {
        file = LittleFS.open(uidExtdRecFile, "r");
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
