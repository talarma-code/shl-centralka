#include "SdRecorder.h"
#include "GlobalTypes.h"
#include <SPI.h>
#include <SD.h>

SdRecorder::SdRecorder() {}

bool SdRecorder::setup(uint8_t csPin) {
    _csPin = csPin;
    Serial.println("Initializing SD card...");

    if (!SD.begin(_csPin)) {
        Serial.println("Failed to initialize SD card!");
        return false;
    }

    Serial.println("SD card initialized successfully.");
    return true;
}

bool SdRecorder::createFile(FileType type, const char* fileName) {
    if (SD.exists(fileName)) {
        Serial.println("File already exists, using existing file.");
        _fileNames[_idx(type)].assign(fileName);
        return true;
    }

    File file = SD.open(fileName, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to create file!");
        return false;
    }

    file.close();
    _fileNames[_idx(type)].assign(fileName);
    Serial.print("File created: ");
    Serial.println(_fileNames[_idx(type)].c_str());
    return true;
}

bool SdRecorder::append(FileType type, const StaticString192 &data) {
    const StaticString192 &fname = _fileNames[_idx(type)];
    if (fname.length() == 0) {
        Serial.println("No file configured for this type. Call createFile() first.");
        return false;
    }

    File file = SD.open(fname.c_str(), FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for append!");
        return false;
    }

    if (!file.println(data.c_str())) {
        Serial.println("Write failed!");
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool SdRecorder::append(FileType type, const char* data) {
    const StaticString192 &fname = _fileNames[_idx(type)];
    if (fname.length() == 0) {
        Serial.println("No file configured for this type. Call createFile() first.");
        return false;
    }

    File file = SD.open(fname.c_str(), FILE_APPEND);
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
void SdRecorder::readFile(FileType type, LineHandlerInterface &handler) {
    const StaticString192 &fname = _fileNames[_idx(type)];
    if (fname.length() == 0) {
        Serial.println("No file configured for this type. Cannot read.");
        return;
    }
    readFile(fname.c_str(), handler);
}

void SdRecorder::readFile(const char* fileName, LineHandlerInterface &handler) {
    File file = SD.open(fileName);
    if (!file) {
        Serial.println("Failed to open file for reading!");
        return;
    }

    constexpr size_t kBufSize = 128 + 1; // match StaticString128 capacity
    char buf[kBufSize];

    while (file.available()) {
        size_t n = file.readBytesUntil('\n', buf, kBufSize - 1);
        buf[n] = '\0';
        StaticString192 s;
        s.assign(buf);
        handler.handle(s);
    }

    file.close();
}

// -------------------- Vector version --------------------
std::vector<StaticString192> SdRecorder::readFileToVector(FileType type) {
    const StaticString192 &fname = _fileNames[_idx(type)];
    if (fname.length() == 0) {
        Serial.println("No file configured for this type. Cannot read.");
        return {};
    }
    return readFileToVector(fname.c_str());
}

std::vector<StaticString192> SdRecorder::readFileToVector(const char* fileName) {
    std::vector<StaticString192> lines;

    File file = SD.open(fileName);
    if (!file) {
        Serial.println("Failed to open file for reading!");
        return lines;
    }

    constexpr size_t kBufSize = 128 + 1; // match StaticString128 capacity
    char buf[kBufSize];

    while (file.available()) {
        size_t n = file.readBytesUntil('\n', buf, kBufSize - 1);
        buf[n] = '\0';
        StaticString192 s;
        s.assign(buf);
        lines.push_back(s);
    }

    file.close();
    return lines;
}
