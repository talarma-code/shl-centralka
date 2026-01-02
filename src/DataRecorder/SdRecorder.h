#pragma once

#include <Arduino.h>
#include <vector>
#include "LineHandlerInterface.h"  // Interface for line handling

// -------------------- SdRecorder class --------------------
class SdRecorder {
public:
    SdRecorder();

    // -------------------- SD initialization --------------------
    bool setup();  // Initialize SD card

    // -------------------- File operations --------------------
    bool createFile(const char* fileName);       // Create a file if it doesn't exist
    bool append(const String &data);             // Append data to the current file
    bool append(const char* data);                // Append C-string data to the current file (avoids dynamic String allocation)

    // -------------------- File reading with callback --------------------
    void readFile(LineHandlerInterface &handler);                   // Read current file
    void readFile(const char* fileName, LineHandlerInterface &handler); // Read specified file

    // -------------------- File reading into vector --------------------
    std::vector<String> readFileToVector();                // Read current file into vector
    std::vector<String> readFileToVector(const char* fileName); // Read specified file into vector

private:
    String _fileName;  // Name of the current file
};
