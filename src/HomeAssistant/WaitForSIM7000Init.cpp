#include "WaitForSIM7000Init.h"
#include "Log.h"

WaitForSIM7000Init::Result WaitForSIM7000Init::step() {
    
    switch (_phase) {
        case Phase::Begin: {
            _waitForATResponseAttempts = 0;
            _ATSendCounter = 0;
            _phase = Phase::SendAT;
            return { Next::Stay, 100 };
        }
        case Phase::SendAT: {
            _waitForATResponseAttempts = 0;
            if (_ATSendCounter >= _retryATLimit) {
                LOG_ERROR("ERROR - No response to AT command after %d attempts", _ATSendCounter);
                _phase = Phase::Begin;
                return { Next::Error, 100 };
            }
            
            sendATCommand(); 
            _ATSendCounter++;

            _phase = Phase::ProbeAT;
            return { Next::Stay, _waitLimitForATResponseMs };   
        }
        case Phase::ProbeAT: {
            int r = _modem.waitResponse(200);
            if (r == 1) {
                LOG_INFO("Modem responded to AT command after %d attempts and %d AT commands", _waitForATResponseAttempts, _ATSendCounter);
                _phase = Phase::Begin; 
                return { Next::Ready, 100 };
            }
            _waitForATResponseAttempts++;
            if (_waitForATResponseAttempts >= _retryWaitLimit) {
                _phase = Phase::SendAT;
                return { Next::Stay, 100 };
            }
            return { Next::Stay, _waitLimitForATResponseMs };
        }
        
    }
    // underermine state machine - should never reach here
    _phase = Phase::Begin;
    return { Next::Stay, 200 };
}

void WaitForSIM7000Init::sendATCommand() {
    //_modem.sendAT always add "AT" at the beginning, so we can send an empty command to just trigger the modem response
    _modem.sendAT("");
}