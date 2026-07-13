#include "MqttPublisher.h"
#include "MqttTopics.h"
#include "TimeUtils.h"
#include "ResetReason.h"
#include "Log.h"

bool MqttPublisher::publishPacket(const MeasurementDataPacket& m) {
    _publishSuccessCount = 0;
    _punlishFailureCount = 0;
    _client.loop();
    delay(60);

    publishUint(energia_wyprodukowana_l1, m.L1EnergyProduced);
    publishUint(energia_wyprodukowana_l2, m.L2EnergyProduced);
    publishUint(energia_pobrana_grzalka, m.HeaterEnergyConsumed);
    publishUint(energia_pobrana_dom, m.HomeTotalEnergyConsumed);
    publishUint(moc_chwilowa_wyprodukowana_l1, m.L1Power3minW);
    publishUint(moc_chwilowa_wyprodukowana_l2, m.L2Power3minW);
    publishUint(moc_sdm120_l1, m.L1PowerNowW);
    publishUint(moc_sdm120_l2, m.L2PowerNowW);
    publishUint(moc_chwilowa_pobrana_dom, m.HomePower3minW);

    publishTopicPayload(napiecie_faza_l1, formatVoltageString(m.L1Voltage_x10).c_str());
    publishTopicPayload(napiecie_faza_l2, formatVoltageString(m.L2Voltage_x10).c_str());
    publishUint(stan_grzalki, m.heaterRequestedStatus == HeaterStatus::On ? "On" : "Off");

    _client.loop();

    LOG_DEBUG("MQTT publish: success=%d, fail=%d", _publishSuccessCount, _punlishFailureCount);

    if (_punlishFailureCount >= 3) {
        return false;
    }
    return true;
}

bool MqttPublisher::publishResetReason() {
    _publishSuccessCount = 0;
    _punlishFailureCount = 0;
    _client.loop();
    delay(60);

    publishTopicPayload(centrala_reset_reason, lastResetReason());
    publishUint(centrala_reset_code, lastResetReasonCode());

    _client.loop();
    LOG_DEBUG("MQTT publish: success=%d, fail=%d", _publishSuccessCount, _punlishFailureCount);

    if (_punlishFailureCount >= 3) {
        return false;
    }
    return true;
}

bool MqttPublisher::publishEspNowEvent(HeaterCommunicationStatus heaterCommunicationStatus) {
    _publishSuccessCount = 0;
    _punlishFailureCount = 0;
    _client.loop();
    delay(60);

    if (heaterCommunicationStatus == HeaterCommunicationStatus::Ok) {
        publishTopicPayload(status_komunikacji_grzalka, "OK");
    } else {
        publishTopicPayload(status_komunikacji_grzalka, "No communication");
    }

    _client.loop();
    LOG_DEBUG("MQTT publish: success=%d, fail=%d", _publishSuccessCount, _punlishFailureCount);

    if (_punlishFailureCount >= 3) {
        return false;
    }
    return true;
}

bool MqttPublisher::publishOnlineHeartbeat(uint32_t epoch) {
    _client.loop();
    delay(60);
    StaticString32 ts = formatIsoTimestamp(epoch);

    LOG_DEBUG("Publishing online heartbeat with timestamp: %s", ts.c_str());
    return publishTopicPayload(centrala_last_seen, ts.c_str());
}

StaticString32 MqttPublisher::formatVoltageString(uint16_t value_x10) const {
    char payload[16];
    unsigned long whole = value_x10 / 10UL;
    unsigned long frac = value_x10 % 10UL;
    snprintf(payload, sizeof(payload), "%lu.%lu", whole, frac);
    return StaticString32(payload);
}

void MqttPublisher::publishInt(const char* topic, int32_t value) {
    char payload[24];
    snprintf(payload, sizeof(payload), "%ld", (long)value);
    publishTopicPayload(topic, payload);
}

void MqttPublisher::publishUint(const char* topic, uint32_t value) {
    char payload[24];
    snprintf(payload, sizeof(payload), "%lu", (unsigned long)value);
    publishTopicPayload(topic, payload);
}

void MqttPublisher::publishUint(const char* topic, const char* value) {
    publishTopicPayload(topic, value);
}

bool MqttPublisher::publishTopicPayload(const char* topic, const char* payload) {
    _client.loop();
    const size_t tlen = strlen(topic);
    const size_t plen = strlen(payload);

    bool ok = false;
    for (uint8_t attempt = 0; attempt < 2; ++attempt) {
        _client.loop();
        ok = _client.publish(topic, payload);
        if (ok) break;
        delay(60);
        LOG_DEBUG("publish retry...");
        _client.loop();
    }

    _client.loop();
    delay(40);

    if (ok) {
        _publishSuccessCount++;
    } else {
        _punlishFailureCount++;
        Serial.printf("MQTT publish FAILED: topic='%s' topic_len=%u payload_len=%u connected=%d state=%d\n",
                      topic, (unsigned)tlen, (unsigned)plen, _client.connected(), _client.state());
    }
    return ok;
}
