#pragma once
#include <TinyGsmClientSIM7000.h>
#include <stdint.h>

class WaitForSIM7000Init
{
public:
    enum class Phase : uint8_t
    {
        Begin,
        SendAT,
        ProbeAT
    };
    enum class Next : uint8_t
    {
        Stay,
        Ready,
        Error
    };

    struct Result
    {
        Next next;
        uint16_t delayMs;    
    };

    explicit WaitForSIM7000Init(TinyGsmSim7000 &modem)
        : _modem(modem) {}

    // One non-blocking step; returns what to do next and when
    Result step();

    // Optional: configure timings and retry limits
    void configure(uint8_t retryATLimit,
                   uint8_t waitLimitMs,
                   uint8_t retryWaitLimit)
    {
        _retryATLimit = retryATLimit;
        _waitLimitForATResponseMs = waitLimitMs;
        _retryWaitLimit = retryWaitLimit;
    }

private:
    void sendATCommand();
    TinyGsmSim7000 &_modem;
    Phase _phase = Phase::Begin;
    uint8_t _waitForATResponseAttempts = 0;
    uint8_t _ATSendCounter= 0;

    // number of AT commands taht will be sedn before decide that the modem is not responding
    uint8_t _retryATLimit = 20;

    // determin how long to wait for responses before send another AT command (300*10 = 3s)
    uint16_t _waitLimitForATResponseMs = 300;
    uint8_t _retryWaitLimit = 10;
};
