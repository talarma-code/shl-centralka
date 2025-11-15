#pragma once
#include "HeaterAPI.h"
#include "NetworkTransport/EspNowTransport.h"

class HeaterEspNow : public HeaterAPI {
public:
    HeaterEspNow(int pin);

    void turnOn() override;
    void turnOff() override;
    bool isOn() const override;
    void registerTransport(IMatterLikeTransport* transportLayer);

private:
    int _pin;
    bool _state;
    IMatterLikeTransport* _transpot;
};
