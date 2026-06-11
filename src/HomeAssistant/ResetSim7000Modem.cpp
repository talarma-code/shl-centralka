/*
 * ResetSim7000Modem timing summary (default configuration)
 *
 * Default delays from ResetSim7000Modem::configure():
 *   _delaySendCfunMs   = 500 ms
 *   _delayWaitCfun0Ms   = 1000 ms
 *   _delayWaitCfun1Ms   = 1500 ms
 *   _delayProbeAtMs     = 500 ms
 *
 * Phases:
 *   Begin:           100 ms
 *   DisableRFModule: 500 ms
 *   WaitForRFDisable: waitResponse(1000) + retry delay(1000)
 *   EnableRFModule:  500 ms
 *   WaitForRFEnable: waitResponse(1500) + retry delay(1500)
 *   ProbeAT:         waitResponse(500) + retry delay(500)
 *   Done:            100 ms
 *
 * Best-case reset time: ~6.7 seconds
 * Path: Begin -> DisableRFModule -> WaitForRFDisable success -> EnableRFModule -> WaitForRFEnable success -> ProbeAT success -> Done
 *
 * Worst-case successful reset before retry limit:
 *   WaitForRFDisable  29 failed tries + success = 59.2 s
 *   WaitForRFEnable   29 failed tries + success = 88.8 s
 *   ProbeAT           29 failed tries + success = 31.6 s
 *   Total ~180.7 seconds (~3 min 1 s)
 *
 * Error return timings (default retryLimit = 30):
 *   WaitForRFDisable error after ~59.2 s
 *   WaitForRFEnable error after ~89.3 s (plus prior DisableRFModule/WaitForRFDisable path)
 *   ProbeAT error after ~32.0 s (after successful RF disable/enable phases)
 *
 * Note: configure() can change these timings if custom delays are set.
 */

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
            _modem.sendAT("AT");
            int r = _modem.waitResponse(_delayProbeAtMs);
            if (r == 1) {
                _phase = Phase::Done;
                return { Next::WaitForNetwork, 200, false };
            }
            _attempts++;
            if (_attempts >= _retryLimit) {
                _phase = Phase::Begin;
                LOG_ERROR("Modem reset failed at ProbeAT after %d attempts", _attempts);
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
