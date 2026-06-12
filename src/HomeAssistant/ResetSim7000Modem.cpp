

#include "ResetSim7000Modem.h"
#include "Log.h"

ResetSim7000Modem::Result ResetSim7000Modem::step() {
    switch (_phase) {
        case Phase::Begin: {
            _attempts = 0;
            _phase = Phase::DisableRFModule;
            return { Next::Stay, 100, false };
        }
        case Phase::DisableRFModule: {
            _modem.sendAT("+CFUN=0");
            _phase = Phase::WaitForRFDisable;
            return { Next::Stay, _delaySendCfunMs, false };
        }
        case Phase::WaitForRFDisable: {
            _modem.sendAT("+CFUN?");
            int r = _modem.waitResponse(_delayWaitCfun0Ms, "+CFUN: 0");
            if (r == 1) {
                _phase = Phase::EnableRFModule;
                return { Next::Stay, 200, false };
            }
            _attempts++;
            if (_attempts >= _retryLimit) {
                _phase = Phase::Begin; // reset for next time
                LOG_ERROR("Modem reset failed at WaitForRFDisable after %d attempts", _attempts);

                return { Next::Error, 200, true };
            }
            return { Next::Stay, _delayWaitCfun0Ms, false };
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
                LOG_ERROR("Modem reset failed at WaitForRFEnable after %d attempts", _attempts);
                return { Next::Error, 200, true };
            }
            return { Next::Stay, _delayWaitCfun1Ms, false };
        }
        case Phase::ProbeAT: {
            const auto res = _waitForInit.step();
            switch (res.next)
            {
            case WaitForSIM7000Init::Next::Stay:
                return { Next::Stay, res.delayMs, false };
                break;
            case WaitForSIM7000Init::Next::Ready:
                _phase = Phase::Done;
                return { Next::WaitForNetwork, res.delayMs, false };
                break;
            case WaitForSIM7000Init::Next::Error:
                 _phase = Phase::Begin;
                return { Next::Error, res.delayMs, true };
                break;
            }
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
