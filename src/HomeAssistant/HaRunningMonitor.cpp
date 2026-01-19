#include "HaRunningMonitor.h"
#include "MqttMeasurementsPublisher.h"
#include "TimeUtils.h"
#include "Log.h"

#include <time.h>
#include <Arduino.h>

// Device RTC is configured to local time (e.g. UTC+1).
// This offset is used to convert local epoch to UTC before
// publishing timestamps with a trailing "Z".
static constexpr int8_t DEVICE_LOCAL_UTC_OFFSET_HOURS = 1;

HaRunningMonitor::Result HaRunningMonitor::step() {
    // Maintain MQTT session
    _client.loop();

    // Tick counter for periodic connection check
    _mqttTick++;
    if (_mqttTick >= _loopsBeforeCheck) {
        _mqttTick = 0;
        if (!_client.connected()) {
            // Signal that higher-level state machine should reconnect
            return { Next::ReconnectNeeded, _loopIntervalMs };
        }
    }

    // Heartbeat: count "ticks" (calls to step) instead of measuring time
    _heartbeatTickCounter++;
    if (_heartbeatTickCounter >= _ticksPerHeartbeat) {
        _heartbeatTickCounter = 0;

        // Use current time only for payload, not for scheduling
        time_t now = time(nullptr);
        uint32_t nowEpochLocal = (now > 0) ? static_cast<uint32_t>(now) : 0;

        uint32_t epochToPublish = nowEpochLocal;
        if (nowEpochLocal != 0) {
            // Convert device local time (RTC) to true UTC before formatting
            epochToPublish = convertLocalEpochToUtc(nowEpochLocal, DEVICE_LOCAL_UTC_OFFSET_HOURS);
            _lastHeartbeatEpoch = epochToPublish;
        }

        publishHeartbeat(epochToPublish);
    }

    return { Next::Stay, _loopIntervalMs };
}

bool HaRunningMonitor::publishHeartbeat(uint32_t epoch) {
    if (!_client.connected()) {
        LOG_INFO("MQTT not connected, skipping heartbeat publish");
        return false;
    }

    // Delegate topic and payload construction to MqttMeasurementsPublisher
    _client.loop();
    bool ok = _statusPublisher.publishOnlineHeartbeat(epoch);
    if (!ok) {
        LOG_ERROR("MQTT heartbeat publish FAILED");
    }
    return ok;
}
