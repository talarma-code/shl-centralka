#include "HeaterFsm.h"
#include "GlobalTypes.h"
#include "IShlProtocolTransport.h"
#include "Log.h"

void HeaterFsm::registerTransport(IShlProtocolTransport* transportLayer) {
    _transport = transportLayer;
}

void HeaterFsm::registerHeaterMac(const uint8_t* mac) {
    memcpy(MAC_HEATER, mac, 6);
    printf("Heater MAC registered: %02X:%02X:%02X:%02X:%02X:%02X\n",
           MAC_HEATER[0], MAC_HEATER[1], MAC_HEATER[2],
           MAC_HEATER[3], MAC_HEATER[4], MAC_HEATER[5]);
}

void HeaterFsm::setHeaterState(bool requestedState) {
    _expectedHeaterState = requestedState;
}

HeaterFsm::Result HeaterFsm::handleIdleState(const ApplicationMessagePacket& evt) {
    if (evt.type == ApplicationCommandType::Timer) {
        _state = State::WaitingResponse;
        _retryCount = 0;
        sendCommand(_expectedHeaterState);
        return {Next::Stay, INTERVAL_5_SECONDS_MS, 0, 0};
    }
    return  {Next::Stay, DO_NOT_RUN_TIMER, 0, 0, 0};
}

HeaterFsm::Result HeaterFsm::handleWaitingResponseState(const ApplicationMessagePacket& evt) {
    if (evt.type == ApplicationCommandType::ProtocolPacket) {
        const ShlProtocolPacket& pkt = evt.payload.protocolPacket.packet;
        uint16_t totalPower;
        uint16_t voltage;
        if (handleResponse(pkt, totalPower, voltage)) {
            _state = State::Idle;
            return {Next::RecivedResponse, DO_NOT_RUN_TIMER, totalPower, voltage, pkt.relay1};
        }
        else{
            // shout be eadge case, we received response but it is not what we expected,
            // so we will retry as standard timeout case - just wait for timer and then resend command
            LOG_ERROR("Heater return unexpected state, retrying... (retry #%u)", _retryCount);
            return {Next::Stay, DO_NOT_RUN_TIMER, 0, 0, 0};
        }
    }

    if (evt.type == ApplicationCommandType::Timer) {
        if (_retryCount >= 3) {
            _state = State::Idle;
            return {Next::Error, INTERVAL_100_MS, 0, 0, 0};
        }
        sendCommand(_expectedHeaterState);
        return {Next::Stay, INTERVAL_5_SECONDS_MS, 0, 0, 0};
    }

    LOG_ERROR("HeaterFsm::handleWaitingResponseState - unexpected event type: %u", evt.type);
    return {Next::Stay, DO_NOT_RUN_TIMER, 0, 0, 0};

}

HeaterFsm::Result HeaterFsm::step(const ApplicationMessagePacket& evt) {
    switch(_state) {
        case State::Idle:
            return handleIdleState(evt);
        case State::WaitingResponse:
            return handleWaitingResponseState(evt);
        default:
            return {Next::Error, INTERVAL_100_MS, 0, 0, 0};
    }
}

void HeaterFsm::sendCommand(bool turnOn) {
    //for now we switching only first relay, second is not used
    ShlProtocolPacket packet = ShlProtocol::createOnOffPayload(_messageCounter++, turnOn, false);
    sendPacket(packet);
    _retryCount++;
}

void HeaterFsm::sendPacket(const ShlProtocolPacket& packet) const {
    if (!_transport) {
        Serial.println("HeaterFsm::transport not registered");
        return;
    }
    _transport->send(MAC_HEATER, packet);
}

bool HeaterFsm::handleResponse(const ShlProtocolPacket& pkt, uint16_t& totalPower, uint16_t& voltage) {
    if (pkt.commandId == SHL_PROTOCOL_CMD_REPORT_ALL || pkt.commandId == SHL_PROTOCOL_CMD_REPORT_POWER || pkt.commandId == SHL_PROTOCOL_CMD_REPORT_VOLTAGE) {
        //we check only first relay state, second is not used
        totalPower = pkt.totalPower;
        voltage = pkt.voltage;
        return true;
    }
    LOG_ERROR("HeaterFsm::handleResponse - unexpected commandId: %u", pkt.commandId);
    return false;
}
