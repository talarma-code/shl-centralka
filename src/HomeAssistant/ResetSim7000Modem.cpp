#include "ResetSim7000Modem.h"

ResetSim7000Modem::Result ResetSim7000Modem::step() {
    switch (_phase) {
        case Phase::Begin: {
            _attempts = 0;
            _phase = Phase::SendCfun0;
            return { Next::Stay, 100, false };
        }
        case Phase::SendCfun0: {
            _modem.sendAT("+CFUN=0");
            _phase = Phase::WaitCfun0;
            return { Next::Stay, _delaySendCfunMs, false };
        }
        case Phase::WaitCfun0: {
            _modem.sendAT("+CFUN?");
            int r = _modem.waitResponse(_delayWaitCfun0Ms, "+CFUN: 0");
            if (r == 1) {
                _phase = Phase::SendCfun1;
                return { Next::Stay, 200, false };
            }
            _attempts++;
            if (_attempts >= _retryLimit) {
                _phase = Phase::Begin; // reset for next time
                return { Next::Error, 200, true };
            }
            return { Next::Stay, _delayWaitCfun0Ms, false };
        }
        case Phase::SendCfun1: {
            _modem.sendAT("+CFUN=1");
            _phase = Phase::WaitCfun1;
            return { Next::Stay, _delaySendCfunMs, false };
        }
        case Phase::WaitCfun1: {
            _modem.sendAT("+CFUN?");
            int r = _modem.waitResponse(_delayWaitCfun1Ms, "+CFUN: 1");
            if (r == 1) {
                _phase = Phase::ProbeAT;
                return { Next::Stay, 300, false };
            }
            _attempts++;
            if (_attempts >= _retryLimit) {
                _phase = Phase::Begin;
                return { Next::Error, 200, true };
            }
            return { Next::Stay, _delayWaitCfun1Ms, false };
        }
        case Phase::ProbeAT: {
            _modem.sendAT("AT");
            int r = _modem.waitResponse(_delayProbeAtMs);
            if (r == 1) {
                _phase = Phase::Done;
                return { Next::WaitForNetwork, 2000, false };
            }
            _attempts++;
            if (_attempts >= _retryLimit) {
                _phase = Phase::Begin;
                return { Next::Error, 200, true };
            }
            return { Next::Stay, _delayProbeAtMs, false };
        }
        case Phase::Done: {
            _phase = Phase::Begin;
            _attempts = 0;
            return { Next::Stay, 100, false };
        }
    }
    // Fallback
    return { Next::Stay, 200, false };
}
