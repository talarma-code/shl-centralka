#pragma once
#include <TinyGsmClientSIM7000.h>
#include <stdint.h>

class ResetSim7000Modem {
public:
    enum class Phase : uint8_t { Begin, SendCfun0, WaitCfun0, SendCfun1, WaitCfun1, ProbeAT, Done };
    enum class Next : uint8_t { Stay, WaitForNetwork, Error };

    struct Result {
        Next next;
        uint16_t delayMs;
        bool failed; // true if entering Error due to restart failure
    };

    explicit ResetSim7000Modem(TinyGsmSim7000& modem)
        : _modem(modem) {}

    // One non-blocking step; returns what to do next and when
    Result step();

    // Optional: configure timings and retry limits
    void configure(uint16_t sendCfunDelayMs,
                   uint16_t waitCfun0Ms,
                   uint16_t waitCfun1Ms,
                   uint16_t probeAtMs,
                   uint8_t retryLimit) {
        _delaySendCfunMs = sendCfunDelayMs;
        _delayWaitCfun0Ms = waitCfun0Ms;
        _delayWaitCfun1Ms = waitCfun1Ms;
        _delayProbeAtMs = probeAtMs;
        _retryLimit = retryLimit;
    }

private:
    TinyGsmSim7000& _modem;
    Phase _phase = Phase::Begin;
    uint8_t _attempts = 0;

    // Defaults tuned for SIM7000 bring-up
    uint16_t _delaySendCfunMs = 500;
    uint16_t _delayWaitCfun0Ms = 1000;
    uint16_t _delayWaitCfun1Ms = 1500;
    uint16_t _delayProbeAtMs   = 500;
    uint8_t  _retryLimit       = 30;
};
