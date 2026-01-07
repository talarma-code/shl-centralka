#pragma once

#include <Arduino.h>
#include <vector>
#include "LineHandlerInterface.h"  // Interface for line handling
#include "GlobalTypes.h"

// Default CS pin for SD on ESP32 DevKit
#define SD_CS_PIN 5

// -------------------- SdRecorder class --------------------
class SdRecorder {
public:
    SdRecorder();

    // -------------------- Types --------------------
    enum class FileType {
        Measurements = 0,
        Events = 1,
        Logs = 2,
        COUNT
    };

    // -------------------- SD initialization --------------------
    // Provide CS pin (default is SD_CS_PIN)
    bool setup(uint8_t csPin = SD_CS_PIN);  // Initialize SD card

    // -------------------- Per-file operations --------------------
    // Create a file for a specific logical file type (measurements/events/logs)
    bool createFile(FileType type, const char* fileName);

    // Append to a specific logical file (use StaticString128 to avoid dynamic allocation)
    bool append(FileType type, const StaticString192 &data);
    bool append(FileType type, const char* data);

    // -------------------- File reading with callback --------------------
    void readFile(FileType type, LineHandlerInterface &handler);                   // Read specified logical file
    void readFile(const char* fileName, LineHandlerInterface &handler); // Read specified file

    // -------------------- File reading into vector --------------------
    std::vector<StaticString192> readFileToVector(FileType type);                // Read specified logical file into vector
    std::vector<StaticString192> readFileToVector(const char* fileName); // Read specified file into vector

private:
    static constexpr size_t kFileTypeCount = static_cast<size_t>(FileType::COUNT);
    StaticString192 _fileNames[kFileTypeCount];  // Mapping from FileType index to filename
    uint8_t _csPin = SD_CS_PIN;

    inline int _idx(FileType t) const { return static_cast<int>(t); }
};
