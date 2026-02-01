#pragma once

#include <Arduino.h>
#include "ApplicationDataModel.h"
#include "MatterLike.h"
#include "HeaterEspNow.h"

class HeaterFsm {
public:
    enum class State : uint8_t {
        Idle
    };

    struct Result {
        bool hasCommand;
        HeaterCommandPacket command;
    };

    explicit HeaterFsm(HeaterEspNow& heater);

    // Process single Matter packet and (optionally) produce a HeaterCommandPacket
    Result step(const MatterPacketWithMac& pkt);

private:
    HeaterEspNow& _heater;
    State _state = State::Idle;
};
