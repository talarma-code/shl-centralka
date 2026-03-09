#include "HeaterFsm.h"
#include "GlobalTypes.h"

HeaterFsm::HeaterFsm(HeaterAPI& heater)
    : _heater(heater) {}

void HeaterFsm::startCommand(bool turnOn) {
    if (_state == State::WaitingAck) {
        return;
    }
    _expectedState = turnOn;
    _state = State::WaitingAck;
    _retryCount = 0;
    sendCommand();
}

HeaterFsm::Result HeaterFsm::step(const ApplicationMessagePacket& evt) {
    if (_state == State::Idle) {
        return {Next::Acked, INTERVAL_100_MS};
    }

    // waiting for ack/state report
    if (evt.type == ApplicationCommandType::MatterPacket) {
        const MatterLikePacket& ml = evt.payload.matterPacket.packet;
        if (handleAckPacket(ml)) {
            _state = State::Idle;
            return {Next::Acked, INTERVAL_100_MS};
        }
        return {Next::Stay, INTERVAL_100_MS};
    }

    if (evt.type == ApplicationCommandType::Timer) {
        if (_retryCount >= 3) {
            _state = State::Idle;
            return {Next::Error, INTERVAL_100_MS};
        }
        sendCommand();
        return {Next::Stay, INTERVAL_100_MS};
    }

    return {Next::Stay, INTERVAL_100_MS};
}

void HeaterFsm::sendCommand() {
    if (_expectedState) {
        _heater.turnOn();
    } else {
        _heater.turnOff();
    }
    _retryCount++;
}

bool HeaterFsm::handleAckPacket(const MatterLikePacket& ml) {
    if (MatterLike::isAckResponsePacket(ml)) {
        return true;
    }

    if (ml.payload.clusterId == CLUSTER_ONOFF &&
        ml.payload.commandId == CMD_REPORT_ATTRIBUTE &&
        ml.payload.attributeId == ATTR_ONOFF_STATE) {
        bool reportedState = (ml.payload.value != 0);
        return reportedState == _expectedState;
    }

    return false;
}
