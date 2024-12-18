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
#include "ESPWebFileManager.h"
#include "SecData.h"
ESPWebFileManager::ESPWebFileManager() {}
ESPWebFileManager::~ESPWebFileManager() {}

bool ESPWebFileManager::initFileSystem(FileSystemType fsType, bool formatOnFail) {
    fs_type = fsType;

    switch (fsType) {
        case FS_SD_CARD:
            DEBUG_PRINTLN("Initializing SD Card...");
            return initSDCard(formatOnFail);
        case FS_SPIFFS:
            DEBUG_PRINTLN("Initializing SPIFFS...");
            return initSPIFFS(formatOnFail);
        case FS_LITTLEFS:
            DEBUG_PRINTLN("Initializing LittleFS...");
            return initLittleFS(formatOnFail);
        case FS_FATFS:
            DEBUG_PRINTLN("Initializing FATfs...");
            return initFATfs(formatOnFail);
        default:
            DEBUG_PRINTLN("Unknown file system type.");
            return false;
    }
}

bool ESPWebFileManager::initSDCard(bool formatOnFail) {
    DEBUG_PRINTLN("Starting SD Card initialization...");
    if (!SD.begin()) {
        DEBUG_PRINTLN("SD Card mount failed.");
        if (formatOnFail) {
            DEBUG_PRINTLN("Formatting SD Card is not implemented in this library.");
        }
        memory_ready = false;
        return false;
    }

    DEBUG_PRINTLN("SD Card mounted successfully.");
    memory_ready = true;
    current_fs = &SD;
    return true;
}

bool ESPWebFileManager::initSPIFFS(bool formatOnFail) {
    DEBUG_PRINTLN("Starting SPIFFS initialization...");

    if (!SPIFFS.begin(true)) {
        DEBUG_PRINTLN("SPIFFS mount failed.");
        if (formatOnFail) {
            DEBUG_PRINTLN("Attempting to format SPIFFS...");
            if (SPIFFS.format()) {
                DEBUG_PRINTLN("SPIFFS formatted successfully. Retrying mount...");
                if (SPIFFS.begin(true)) {
                    DEBUG_PRINTLN("SPIFFS mounted successfully after formatting.");
                    memory_ready = true;
                    current_fs = &SPIFFS;
                    return true;
                } else {
                    DEBUG_PRINTLN("SPIFFS mount failed even after formatting.");
                }
            } else {
                DEBUG_PRINTLN("SPIFFS formatting failed.");
            }
        }
        memory_ready = false;
        return false;
    }

    DEBUG_PRINTLN("SPIFFS mounted successfully.");
    memory_ready = true;
    current_fs = &SPIFFS;
    return true;
}


bool ESPWebFileManager::initLittleFS(bool formatOnFail) {
    DEBUG_PRINTLN("Starting LittleFS initialization...");

    if (!LittleFS.begin(true)) {
        DEBUG_PRINTLN("LittleFS mount failed.");
        if (formatOnFail) {
            DEBUG_PRINTLN("Attempting to format LittleFS...");
            if (LittleFS.format()) {
                DEBUG_PRINTLN("LittleFS formatted successfully. Retrying mount...");
                if (LittleFS.begin(true)) {
                    DEBUG_PRINTLN("LittleFS mounted successfully after formatting.");
                    memory_ready = true;
                    current_fs = &LittleFS;
                    return true;
                } else {
                    DEBUG_PRINTLN("LittleFS mount failed even after formatting.");
                }
            } else {
                DEBUG_PRINTLN("LittleFS formatting failed.");
            }
        }
        memory_ready = false;
        return false;
    }

    DEBUG_PRINTLN("LittleFS mounted successfully.");
    memory_ready = true;
    current_fs = &LittleFS;
    return true;
}
    String ESPWebFileManager::sanitizePath(const String& path) {
    String sanitized = path;
    while (sanitized.indexOf("//") >= 0) {
        sanitized.replace("//", "/");
    }
    if (sanitized.endsWith("/")) {
        sanitized.remove(sanitized.length() - 1); // Remove trailing slash
    }
    return sanitized;
}

bool ESPWebFileManager::initFATfs(bool formatOnFail) {
    DEBUG_PRINTLN("Starting FATfs initialization...");

    if (!FFat.begin(true)) {
        DEBUG_PRINTLN("FATfs mount failed.");
        if (formatOnFail) {
            DEBUG_PRINTLN("Attempting to format FATfs...");
            if (FFat.format()) {
                DEBUG_PRINTLN("FATfs formatted successfully. Retrying mount...");
                if (FFat.begin(true)) {
                    DEBUG_PRINTLN("FATfs mounted successfully after formatting.");
                    memory_ready = true;
                    current_fs = &FFat;
                    return true;
                } else {
                    DEBUG_PRINTLN("FATfs mount failed even after formatting.");
                }
            } else {
                DEBUG_PRINTLN("FATfs formatting failed.");
            }
        }
        memory_ready = false;
        return false;
    }

    DEBUG_PRINTLN("FATfs mounted successfully.");
    memory_ready = true;
    current_fs = &FFat;
    return true;
}

void ESPWebFileManager::listDir(const char *dirname, uint8_t levels) {
    if (!memory_ready) {
        DEBUG_PRINTLN("File system not initialized.");
        return;
    }

    File root = current_fs->open(dirname);
    if (!root || !root.isDirectory()) {
        DEBUG_PRINTLN("Failed to open directory.");
        return;
    }

    str_data = ""; // Clear the response string
    File file = root.openNextFile();

    while (file) {
        if (!str_data.isEmpty()) {
            str_data += ":"; // Separate entries with a colon
        }

        if (file.isDirectory()) {
            str_data += "1," + String(file.name()) + ",-"; // Folders don't have sizes
        } else {
            str_data += "0," + String(file.name()) + "," + String(file.size());
        }
        file = root.openNextFile();
    }

    file.close();
}


void ESPWebFileManager::setServer(AsyncWebServer *server)
{
    DEBUG_PRINTLN("Starting Server...!");
    if (server == nullptr) {
        DEBUG_PRINT("Server is null!");
        return;
    }
    _server = server;

    _server->on("/file", HTTP_GET, [&](AsyncWebServerRequest *request){ 
            AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", SecData, SecData_len);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
            // request->send(200, "text/html", html_page); 
            // request->send(200, "text/plain", "Test route working");
        });

    server->on("/get-folder-contents", HTTP_GET, [&](AsyncWebServerRequest *request){
        DEBUG_PRINT2("path:", request->arg("path").c_str());
        listDir(request->arg("path").c_str(), 0);
        request->send(200, "text/plain", str_data);
    });

    server->on("/upload", HTTP_POST, [&](AsyncWebServerRequest *request)
        { request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"File upload complete\"}"); }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
        String file_path;

        file_path = "/";
        file_path += filename;

        if(!index)
        {
            DEBUG_PRINTX("UploadStart: %s\n", file_path.c_str());
            if(current_fs->exists(file_path)) 
            {
                current_fs->remove(file_path);
            }
        }

        File file = current_fs->open(file_path, FILE_APPEND);
        if(file) 
        {
            if(file.write(data, len) != len)
            {
                DEBUG_PRINT("File write failed");
            }
            file.close();
        }
        if(final)
        {
            DEBUG_PRINTX("UploadEnd: %s, %u B\n", file_path.c_str(), index+len);
        } 
    });



server->on("/delete", HTTP_GET, [&](AsyncWebServerRequest *request) {
    String path = request->hasParam("path") ? request->getParam("path")->value() : "";
    path = sanitizePath(path); // Apply sanitization

    DEBUG_PRINT2("Deleting File: ", path);
    if (current_fs->exists(path)) {
        current_fs->remove(path);
        request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"File deleted successfully\"}");
    } else {
        request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"File not found\"}");
    }
});

    server->on("/download", HTTP_GET, [&](AsyncWebServerRequest *request){
        String path;
        if (request->hasParam("path")) 
        {
            path = request->getParam("path")->value();
        } 
        else 
        {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Path not provided\"}");
            return;
        }
        path = sanitizePath(path); // Apply sanitization
        DEBUG_PRINT2("Downloading File: ", path);
        if (current_fs->exists(path)) 
        {
            request->send(*current_fs, path, String(), true);
        } 
        else 
        {
            request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"File not found\"}");
        } 
    });
}

/*
void ESPWebFileManager::setServer(AsyncWebServer *server) {
    _server = server;

    _server->on("/list", HTTP_GET, [&](AsyncWebServerRequest *request) {
        String path = "/";
        if (request->hasParam("path")) {
            path = request->getParam("path")->value();
        }
        listDir(path.c_str(), 0);
        request->send(200, "text/plain", "Directory listed");
    });

    DEBUG_PRINTLN("Web server routes set up.");
}

void ESPWebFileManager::listDir(const char *dirname, uint8_t levels) {
    if (!memory_ready) {
        DEBUG_PRINTLN("File system not initialized.");
        return;
    }

    File root = current_fs->open(dirname);
    if (!root || !root.isDirectory()) {
        DEBUG_PRINTLN("Failed to open directory.");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        DEBUG_PRINTLN(file.isDirectory() ? "DIR: " : "FILE: ");
        DEBUG_PRINTLN(file.name());
        file = root.openNextFile();
    }
}
*/