#include "WaitForSIM7000Init.h"
#include "Log.h"

WaitForSIM7000Init::Result WaitForSIM7000Init::step() {
    
    switch (_phase) {
        case Phase::Begin: {
            _attempts = 0;
            _phase = Phase::EnableRFModule;
            return { Next::Stay, 100, false };
        }
        case Phase::EnableRFModule: {
            _modem.sendAT("+CFUN=1");
            _phase = Phase::WaitForRFEnable;
            return { Next::Stay, _delaySendCfunMs, false };
        }
        case Phase::WaitForRFEnable: {
            _modem.sendAT("+CFUN?");
            int r = _modem.waitResponse(_delayWaitCfun1Ms, "+CFUN: 1");
            if (r == 1) {
                _phase = Phase::ProbeAT;
                return { Next::Stay, 300, false };
            }
            _attempts++;
            if (_attempts >= _retryLimit) {
                _phase = Phase::Begin;
                LOG_ERROR("Modem init failed at WaitForRFEnable after %d attempts", _attempts);
                return { Next::Error, 200, true };
            }
            return { Next::Stay, _delayWaitCfun1Ms, false };
        }
        case Phase::ProbeAT: {
            _modem.sendAT("AT");
            int r = _modem.waitResponse(_delayProbeAtMs);
            if (r == 1) {
                _phase = Phase::Done;
                return { Next::Ready, 200, false };
            }
            _attempts++;
            if (_attempts >= _retryLimit) {
                _phase = Phase::Begin;
                LOG_ERROR("Modem init failed at ProbeAT after %d attempts", _attempts);
                return { Next::Error, 200, true };
            }
            return { Next::Stay, _delayProbeAtMs, false };
        }
        case Phase::Backoff: {
            _phase = Phase::EnableRFModule;
            return { Next::Stay, 100, false };
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
