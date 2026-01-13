#include "WaitForNetworkMonitor.h"

WaitForNetworkMonitor::Result WaitForNetworkMonitor::step() {
    // Check if network is ready
    if (_modem.isNetworkConnected()) {
        _attempts = 0;
        return { Next::GprsConnect, _onSuccessDelayMs, false };
    }

    // Not connected yet; handle attempts budget
    if (_attempts >= _maxAttempts) {
        _attempts = 0; // reset counter for the next cycle
        return { Next::SoftwareRestartModem, _onRestartDelayMs, true };
    }

    _attempts++;
    return { Next::Stay, _pollDelayMs, false };
}
