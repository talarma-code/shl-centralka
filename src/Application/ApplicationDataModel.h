#pragma once;
#include "MatterLikePacket.h"
#include "SystemTimer.h"

enum class ApplicationCommandType : uint8_t {
    Timer = 0,
    MatterPacket = 1,
    HeaterCommand = 2
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



struct ApplicationMessagePacket {
    ApplicationCommandType type;
    union {
        TimerEvent timerEvent;
        MatterPacketWithMac matterPacket;
        HeaterCommandPacket heaterCommandPacket;
    } payload;
};