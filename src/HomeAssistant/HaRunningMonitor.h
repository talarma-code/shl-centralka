#pragma once

#include <stdint.h>
#include <PubSubClient.h>

class MqttMeasurementsPublisher;

class HaRunningMonitor {
public:
    enum class Next : uint8_t { Stay, ReconnectNeeded };

    struct Result {
        Next next;
        uint16_t delayMs;
    };

    HaRunningMonitor(PubSubClient& client, MqttMeasurementsPublisher& statusPublisher)
	    : _client(client), _statusPublisher(statusPublisher) {}

    // Configure intervals (in milliseconds and seconds)
    void configure(uint16_t loopIntervalMs,
                   uint8_t loopsBeforeCheck,
                   uint32_t heartbeatIntervalSec) {
        _loopIntervalMs = loopIntervalMs;
        _loopsBeforeCheck = loopsBeforeCheck;
        _heartbeatIntervalSec = heartbeatIntervalSec;

        // Pre-compute how many step() calls correspond to one heartbeat period
        uint32_t ticks = 0;
        if (_loopIntervalMs > 0) {
            ticks = (heartbeatIntervalSec * 1000UL) / _loopIntervalMs;
        }
        if (ticks == 0) {
            ticks = 1; // at least once per call if misconfigured
        }
        _ticksPerHeartbeat = ticks;
    }

    void reset() { _mqttTick = 0; }

    // One non-blocking step; performs MQTT maintenance and optional heartbeat
    Result step();

private:
    bool publishHeartbeat(uint32_t epoch);

    PubSubClient& _client;
    MqttMeasurementsPublisher& _statusPublisher;

    uint16_t _loopIntervalMs = 500;     // interval between loop() calls
    uint8_t  _loopsBeforeCheck = 3;     // how many loops before connection check
    uint32_t _heartbeatIntervalSec = 60; // heartbeat period

    uint8_t  _mqttTick = 0;

    // Heartbeat scheduling based on "ticks" (calls to step())
    uint32_t _ticksPerHeartbeat = 120;      // default for 500 ms * 120 = 60 s
    uint32_t _heartbeatTickCounter = 0;     // counts calls to step()
    uint32_t _lastHeartbeatEpoch = 0;       // last real timestamp used in payload
};
