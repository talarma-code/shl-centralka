#pragma once

#include "HeaterAPI.h"
#include "IMatterLikeTransport.h"

class HeaterWithPowerMeter : public HeaterAPI {
public:
    explicit HeaterWithPowerMeter(int pin);

    void registerTransport(IMatterLikeTransport* transportLayer);

    void turnOn() override;
    void turnOff() override;
    bool isOn() const override;

    uint32_t voltage(uint8_t endpointId);
    uint32_t totalActivePower(uint8_t endpointId);

private:
    void sendPacket(const MatterLikePacket& packet) const;

    int _pin;
    bool _state = false;
    IMatterLikeTransport* _transport = nullptr;
};
