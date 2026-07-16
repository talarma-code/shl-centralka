#pragma once
#include <PubSubClient.h>
#include <Arduino.h>
#include "GlobalTypes.h"
#include "IntertaskDataModel.h"

class MqttPublisher {
public:
    explicit MqttPublisher(PubSubClient& client)
        : _client(client) {}

    bool publishPacket(const MeasurementDataPacket& m);
    bool publishResetReason();
    bool publishEspNowEvent(EspNowEventPacket espNowEventPacket);

    // Publish device online heartbeat with local timestamp
    bool publishOnlineHeartbeat(uint32_t epoch);
    bool publishDeviceTime(uint32_t epoch);

    StaticString32 formatVoltageString(uint16_t value_x10) const;

private:
    void publishInt(const char* topic, int32_t value);
    void publishUint(const char* topic, uint32_t value);
    void publishUint(const char* topic, const char* value);

    bool publishTopicPayload(const char* topic, const char* payload);
    const char* heaterRequestedStatus(HeaterStatus status) const;
    

    PubSubClient& _client;
    uint32_t _punlishFailureCount = 0;
    uint32_t _publishSuccessCount = 0;
};
