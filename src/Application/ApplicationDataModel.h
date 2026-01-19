#pragma once;
#include "MatterLikePacket.h"
#include "SystemTimer.h"

enum class ApplicationCommandType : uint8_t {
    Timer = 0,
    MatterPacket = 1
};

struct ApplicationMessagePacket {
    ApplicationCommandType type;
    union {
        TimerEvent timerEvent;
        MatterPacketWithMac matterPacket;
    } payload;
};