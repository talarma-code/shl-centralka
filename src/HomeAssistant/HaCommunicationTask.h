#pragma once
// Use SIM7000 modem types explicitly (avoid depending on global macro ordering)
#include <TinyGsmClientSIM7000.h>
#include <PubSubClient.h>

#include "ActiveTask.h"
#include "ActiveQueue.h"
#include "IntertaskDataModel.h"



class HaCommunicationTask : public ActiveTask {
public:
    HaCommunicationTask();
    void setup() override;
    void loop() override;
private: 
    ActiveQueue<SystemMessagePacket> dataRecorderQueue; 
    HardwareSerial hardwareModem;
    TinyGsmSim7000 tinyGsmModem;
    TinyGsmSim7000::GsmClientSim7000 tinyGsmClient;
    PubSubClient mqttClient;

    // bool initModem();
    // bool connectToNetwork();
};
