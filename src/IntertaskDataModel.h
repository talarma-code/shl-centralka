#pragma once

#include <Arduino.h>
#include "esp_system.h"
#include "StaticString.h"
#include "GlobalTypes.h"
#include "ShlProtocolPacket.h"
#include "SystemTimer.h"


enum class SystemDataType : uint8_t {
    Measurements = 0,
    Events = 1,
    Timer = 2,
    RtcSync = 3,
    ServiceCommand = 4,
    NotifyEspNowEvent = 5
};

enum class MeasurementDataType : uint8_t {
    Now = 0,
    Historical = 1
};

enum class HeaterStatus : uint8_t {
    Off = 0,
    On = 1
};

enum class HeaterCommunicationStatus : uint8_t {
    NoCommunication = 0,
    Ok = 1
};

enum class ServiceCommandEnum : uint8_t {
    SoftwareResetModem = 0,
    HardwareResetModem = 1
};

struct ServiceCommandPacket {
    ServiceCommandEnum command;
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
    HeaterStatus heaterRequestedStatus;

    MeasurementDataType measurementType;
};

struct EspNowEventPacket {
    uint32_t timestamp;
    HeaterCommunicationStatus heaterCommunicationStatus;
};

struct SystemMessagePacket {
    SystemDataType type;
    union {
        MeasurementDataPacket measurementData;
        SystemEventPacket systemEvent;
        TimerDataPacket timerData;
        ServiceCommandPacket serviceCommandData;
        EspNowEventPacket espNowEventData;
    } payload;

};
//-------------------------------------------------------------------
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
//-------------------------------------------------------------------

enum class ApplicationCommandType : uint8_t {
    Timer = 0,
    ProtocolPacket = 1,
    HeaterCommand = 2,
    RtcSync = 3
};

enum class HeaterCommandType : uint8_t {
    Unused = 0,
    Ack = 1,
    State = 2,
    Power = 3
};

struct HeaterCommandPacket {
    HeaterCommandType type;
    bool state;
    uint32_t power;
};

struct RtcSyncCommandPacket {
    uint32_t epochTime;
};

struct ApplicationMessagePacket {
    ApplicationCommandType type;
    union {
        TimerEvent timerEvent;
        ShlProtocolWithMacAddress protocolPacket;
        HeaterCommandPacket heaterCommandPacket;
        RtcSyncCommandPacket rtcSyncCommandPacket;
    } payload;
};
//-------------------------------------------------------------------

