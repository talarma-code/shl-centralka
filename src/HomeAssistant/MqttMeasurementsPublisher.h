#pragma once
#include <PubSubClient.h>
#include <Arduino.h>
#include "GlobalTypes.h"
#include "IntertaskDataModel.h"

class MqttMeasurementsPublisher {
public:
    MqttMeasurementsPublisher(PubSubClient& client, const char* baseTopic)
        : _client(client), _baseTopic(baseTopic) {}

    bool publishPacket(const MeasurementDataPacket& m);

    StaticString32 formatVoltageString(uint16_t value_x10) const;

private:
    bool publishInt(const char* name, int32_t value);
    bool publishUint(const char* name, uint32_t value);
    bool publishUint(const char* name, const char* value);

    bool publishTopicPayload(const char* fieldName, const char* payload);

    PubSubClient& _client;
    const char* _baseTopic;
};
