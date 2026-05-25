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
    bool publishResetReason();

    // Publish device online heartbeat with local timestamp
    bool publishOnlineHeartbeat(uint32_t epoch);

    StaticString32 formatVoltageString(uint16_t value_x10) const;

private:
    void publishInt(const char* name, int32_t value);
    void publishUint(const char* name, uint32_t value);
    void publishUint(const char* name, const char* value);

    bool publishTopicPayload(const char* fieldName, const char* payload);

    PubSubClient& _client;
    const char* _baseTopic;
    uint32_t _punlishFailureCount = 0;
    uint32_t _publishSuccessCount = 0;
};
