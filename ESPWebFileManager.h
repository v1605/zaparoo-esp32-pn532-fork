/*
 * EspWebFileManager Library
 * Copyright (C) 2024 Jobit Joseph
 * Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
 * You may not use this work for commercial purposes. Modifications must credit the original author.
 * See the LICENSE file for more details.
 * Project Name: ESPWebFileManager Library
 * Project Brief: ESPWebFileManager Library
 * Author: Jobit Joseph @ https://github.com/jobitjoseph
 * IDE: Arduino IDE 2.x.x
 * Arduino Core: ESP32 Arduino Core V 3.0.7
 * GitHub: https://github.com/jobitjoseph/ESPWebFileManager
 * Dependencies : 
 *                Async TCP Library for ESP32 V 3.2.14 @ https://github.com/mathieucarbou/AsyncTCP
 *                ESPAsyncWebServer Library V 2.2.6 @ https://github.com/mathieucarbou/ESPAsyncWebServer
 * Copyright © Jobit Joseph
 * 
 * This code is licensed under the following conditions:
 *
 * 1. Non-Commercial Use:
 * This program is free software: you can redistribute it and/or modify it
 * for personal or educational purposes under the condition that credit is given 
 * to the original author. Attribution is required, and the original author 
 * must be credited in any derivative works or distributions.
 *
 * 2. Commercial Use:
 * For any commercial use of this software, you must obtain a separate license
 * from the original author. Contact the author for permissions or licensing
 * options before using this software for commercial purposes.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING 
 * FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Jobit Joseph
 * Date: 01 December 2024
 *
 * For commercial use or licensing requests, please contact [jobitjoseph1@gmail.com].
 */
//MOdded

#ifndef ESP_WEB_FILE_MANAGER_H
#define ESP_WEB_FILE_MANAGER_H
#include <FS.h>

#ifdef ESP32
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

// Include specific file systems
#include <SPIFFS.h>
#include <SD.h>
#include <LittleFS.h>
#include <FFat.h>

#define EN_DEBUG
#define DEBUG_SERIAL Serial

#ifdef EN_DEBUG
#define DEBUG_PRINTLN(x) { DEBUG_SERIAL.println(x); }
#define DEBUG_PRINT(x) { DEBUG_SERIAL.print(x); }
#define DEBUG_PRINT2(x, y)   DEBUG_SERIAL.print(F(x)); DEBUG_SERIAL.println(y)
#define DEBUG_PRINTX(...)     DEBUG_SERIAL.printf(__VA_ARGS__)
#else
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT(x)
#endif

class ESPWebFileManager {
public:
    enum FileSystemType { FS_UNKNOWN, FS_SD_CARD, FS_SPIFFS, FS_LITTLEFS, FS_FATFS };

private:
    FS *current_fs = nullptr;
    bool memory_ready = false;
    String str_data = "";
    AsyncWebServer *_server = nullptr;
    FileSystemType fs_type = FS_UNKNOWN;

public:
    ESPWebFileManager();
    ~ESPWebFileManager();

    bool initFileSystem(FileSystemType fsType, bool formatOnFail = false);
    String sanitizePath(const String& path);

    void setServer(AsyncWebServer *server);

    // Utility functions
    void listDir(const char *dirname, uint8_t levels);

private:
    bool initSDCard(bool formatOnFail);
    bool initSPIFFS(bool formatOnFail);
    bool initLittleFS(bool formatOnFail);
    bool initFATfs(bool formatOnFail);
};

#endif