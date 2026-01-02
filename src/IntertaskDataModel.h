#pragma once
#include <Arduino.h>
#include "esp_system.h"
#include "StaticString.h"

enum class SystemDataType : uint8_t {
    Measurements = 0,
    Events = 1
};

struct SystemEventPacket { 
    uint32_t timestamp;
    esp_reset_reason_t reason;
};

struct MeasurementDataPacket {
    uint32_t timestamp;

    uint32_t L1Power;
    uint32_t L2Power;
    uint32_t HeaterPower;
    uint32_t HomeTotalPower;

    uint16_t L1Voltage_x10; 
    uint16_t L2Voltage_x10;

};

struct SystemMessagePacket {
    SystemDataType type;
    union {
        MeasurementDataPacket measurementData;
        SystemEventPacket systemEvent;
    } payload;

};

struct SystemLogPacket  {
    uint32_t timestamp;
    StaticString<50> logMessage;
};
