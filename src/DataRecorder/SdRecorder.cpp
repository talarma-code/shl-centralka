#include "SdRecorder.h"
#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN 5   // Default CS pin for ESP32 DevKit

SdRecorder::SdRecorder() {}

bool SdRecorder::setup() {
    Serial.println("Initializing SD card...");

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Failed to initialize SD card!");
        return false;
    }

    Serial.println("SD card initialized successfully.");
    return true;
}

bool SdRecorder::createFile(const char* fileName) {
    if (SD.exists(fileName)) {
        Serial.println("File already exists, using existing file.");
        _fileName = String(fileName);
        return true;
    }

    File file = SD.open(fileName, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to create file!");
        return false;
    }

    file.close();
    _fileName = String(fileName);
    Serial.println("File created: " + _fileName);
    return true;
}

bool SdRecorder::append(const String &data) {
    if (_fileName.length() == 0) {
        Serial.println("No file selected. Call createFile() first.");
        return false;
    }

    File file = SD.open(_fileName.c_str(), FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for append!");
        return false;
    }

    if (!file.println(data)) {
        Serial.println("Write failed!");
        file.close();
        return false;
    }

    file.close();
    return true;
}

// Overload: append C-string directly (avoids creating Arduino String)
bool SdRecorder::append(const char* data) {
    if (_fileName.length() == 0) {
        Serial.println("No file selected. Call createFile() first.");
        return false;
    }

    File file = SD.open(_fileName.c_str(), FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for append!");
        return false;
    }

    if (!file.println(data)) {
        Serial.println("Write failed!");
        file.close();
        return false;
    }

    file.close();
    return true;
}

// -------------------- Callback version --------------------
void SdRecorder::readFile(LineHandlerInterface &handler) {
    if (_fileName.length() == 0) {
        Serial.println("No file selected. Cannot read.");
        return;
    }
    readFile(_fileName.c_str(), handler);
}

void SdRecorder::readFile(const char* fileName, LineHandlerInterface &handler) {
    File file = SD.open(fileName);
    if (!file) {
        Serial.println("Failed to open file for reading!");
        return;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        handler.handle(line);
    }

    file.close();
}

// -------------------- Vector version --------------------
std::vector<String> SdRecorder::readFileToVector() {
    if (_fileName.length() == 0) {
        Serial.println("No file selected. Cannot read.");
        return {};
    }
    return readFileToVector(_fileName.c_str());
}

std::vector<String> SdRecorder::readFileToVector(const char* fileName) {
    std::vector<String> lines;

    File file = SD.open(fileName);
    if (!file) {
        Serial.println("Failed to open file for reading!");
        return lines;
    }

    while (file.available()) {
        lines.push_back(file.readStringUntil('\n'));
    }

    file.close();
    return lines;
}
