#pragma once
#include "HeaterAPI.h"

class HeaterEspNow : public HeaterAPI {
public:
    HeaterEspNow(int pin);

    void turnOn() override;
    void turnOff() override;
    bool isOn() const override;

private:
    int _pin;
    bool _state;
};
