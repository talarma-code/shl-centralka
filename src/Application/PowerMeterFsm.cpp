#include "PowerMeterFsm.h"  

PowerMeterFsm::PowerMeterFsm() : orwe520PowerMeter(), sdm120ctPowerMeter() {}

void PowerMeterFsm::setup() {
    orwe520PowerMeter.setup();
    sdm120ctPowerMeter.setup();
}

