#pragma once
#include <Arduino.h>
#include "esp_system.h"
#include "StaticString.h"
#include "GlobalTypes.h"

enum class SystemDataType : uint8_t {
    Measurements = 0,
    Events = 1,
    Timer = 2
};

enum class MeasurementDataType : uint8_t {
    Now = 0,
    Historical = 1
};

struct TimerDataPacket {
    uint32_t timerId;
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

    uint16_t HeaterEnableForSeconds;
    MeasurementDataType measurementType;
};

struct SystemMessagePacket {
    SystemDataType type;
    union {
        MeasurementDataPacket measurementData;
        SystemEventPacket systemEvent;
        TimerDataPacket timerData;
    } payload;

};

enum class LogLevel : uint8_t {
    detailDebug = 0,
    debug = 1,
    info = 2,
    error = 3,
    critical = 4
};

struct SystemLogPacket  {
    LogLevel level;
    uint32_t timestamp;
    StaticString96 logMessage;
};


