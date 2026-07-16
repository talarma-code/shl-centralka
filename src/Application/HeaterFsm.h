#pragma once

#include <Arduino.h>
#include "IntertaskDataModel.h"
#include "ShlProtocol.h"
#include "IShlProtocolTransport.h"

class HeaterFsm {
public:
    enum class Next : uint8_t { Stay, RecivedResponse, Error };

    enum class State : uint8_t { Idle, WaitingResponse };

    struct Result {
        Next next;
        uint16_t delayMs;
        uint16_t totalPower;
        uint16_t voltage;
        uint16_t heaterRealState;
    };

    Result step(const ApplicationMessagePacket& evt);
    void registerTransport(IShlProtocolTransport* transportLayer);
    void registerHeaterMac(const uint8_t* mac);
    void setHeaterState(bool requestedState);
    void sendCommand(bool turnOn);

private:
    bool _expectedHeaterState = false;
    State _state = State::Idle;
    uint8_t _retryCount = 0;
    IShlProtocolTransport* _transport = nullptr;
    uint8_t _messageCounter = 0;
    uint8_t MAC_HEATER[6] = {0};

    Result handleIdleState(const ApplicationMessagePacket& evt);
    Result handleWaitingResponseState(const ApplicationMessagePacket& evt);


    void sendPacket(const ShlProtocolPacket& packet) const;
    bool handleResponse(const ShlProtocolPacket& pkt, uint16_t& totalPower, uint16_t& voltage);
};
