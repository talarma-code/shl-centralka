#pragma once
#include "ORWE520PowerMeter.h"
#include "SDM120CTPowerMeter.h"

class PowerMeterFsm {
public:
    PowerMeterFsm();
    void setup();

private:
    ORWE520PowerMeter orwe520PowerMeter;
    SDM120CTPowerMeter sdm120ctPowerMeter;
};