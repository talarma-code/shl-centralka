#pragma once
#include <TinyGsmClientSIM7000.h>
#include <stdint.h>

class WaitForNetworkMonitor {
public:
    enum class Next : uint8_t { Stay, GprsConnect, SoftwareRestartModem };

    struct Result {
        Next next;
        uint16_t delayMs;
        bool exhausted; // true if attempts limit reached and we suggest restart
    };

    explicit WaitForNetworkMonitor(TinyGsmSim7000& modem)
        : _modem(modem) {}

    // One non-blocking step; returns what to do next and when
    Result step();

    // Configure timings and max attempts
    void configure(uint16_t pollDelayMs,
                   uint8_t maxAttempts,
                   uint16_t onSuccessDelayMs,
                   uint16_t onRestartDelayMs) {
        _pollDelayMs = pollDelayMs;
        _maxAttempts = maxAttempts;
        _onSuccessDelayMs = onSuccessDelayMs;
        _onRestartDelayMs = onRestartDelayMs;
    }

    void reset() { _attempts = 0; }

private:
    TinyGsmSim7000& _modem;
    uint8_t _attempts = 0;

    // Defaults aligned with previous in-place logic
    uint16_t _pollDelayMs = 500;
    uint8_t  _maxAttempts = 20;   // can be overridden from task via configure()
    uint16_t _onSuccessDelayMs = 100;
    uint16_t _onRestartDelayMs = 500;
};
