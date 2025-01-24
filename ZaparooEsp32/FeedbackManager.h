#pragma once

#include <AudioFileSourceLittleFS.h>
#include "AudioFileSourceSD.h"
#include <AudioOutputI2S.h>
#include <AudioGeneratorMP3.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "ZaparooToken.h"

class FeedbackManager {
private:
    Preferences* preferences;
    void setupPins();
    void createUidMappingFile();
public:
    float audioGain = 1.0;
    bool wifiLedEnabled = false;
    bool motorEnabled = false;
    bool launchLedEnabled = false;
    bool audioEnabled = false;
    bool pwrLedEnabled = false;
    bool resetOnRemove = true;
    bool sdCardEnabled = false;
    bool buzzOnDetect = true;
    bool buzzOnLaunch = true;
    bool buzzOnRemove = true;
    bool buzzOnError = true;

    String defaultInsertAudio = "";
    String defaultLaunchAudio = "";
    String defaultRemoveAudio = "";
    String defaultErrorAudio = "";

    AudioOutputI2S* out = nullptr;

    int motorPin = -1;
    int launchLedPin = -1;
    int wifiLedPin = -1;
    int pwrLedPin = -1;
    int i2sBclkPin = -1;
    int i2sLrcPin = -1;
    int i2sDoutPin = -1;

    FeedbackManager();
    ~FeedbackManager();
    void init(Preferences* prefs);
    void update(JsonDocument& doc);
    void set(JsonDocument& doc);
    void motorOn(int predelay = 0);
    void motorOff(int predelay = 0);
    void launchLedOn(int predelay = 0);
    void launchLedOff(int predelay = 0, int postDelay = 0);
    void wifiLedOn();
    void wifiLedOff();
    void expressError(int code);
    void successActions(ZaparooToken* obj);
    void setUidAudioMappings(ZaparooToken* obj);
    void getUidMappings(JsonDocument& toSet);
    void saveUidMapping(JsonDocument& value);
    int playAudio(const char* audioPath);
    void cardInsertedActions(ZaparooToken* obj);
    void cardRemovedActions(ZaparooToken* obj);
};
