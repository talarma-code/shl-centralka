#pragma once

#include <Arduino.h>
#include "IntertaskDataModel.h"
#include "MatterLike.h"
#include "HeaterAPI.h"

class HeaterFsm {
public:
    enum class Next : uint8_t { Stay, Acked, Error };

    enum class State : uint8_t { Idle, WaitingAck };

    struct Result {
        Next next;
        uint16_t delayMs;
    };

    explicit HeaterFsm(HeaterAPI& heater);

    void startCommand(bool turnOn);
    Result step(const ApplicationMessagePacket& evt);

private:
    HeaterAPI& _heater;
    bool _expectedState = false;
    State _state = State::Idle;
    uint8_t _retryCount = 0;

    void sendCommand();
    bool handleAckPacket(const MatterLikePacket& ml);
};
