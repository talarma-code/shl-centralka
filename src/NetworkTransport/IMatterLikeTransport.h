#pragma once
#include <Arduino.h>
#include "../MatterLikeProtocol/MatterLikePacket.h"

class IMatterLikeTransport {
public:
  virtual ~IMatterLikeTransport() = default;

  // Initialize transport (radio, sockets, etc.)
  virtual bool begin() = 0;

  // Send packet to a specific peer
  virtual bool send(const uint8_t *peerMac, const MatterLikePacket &packet) = 0;

  // Optional ACK sending
  virtual void sendAck(const uint8_t *peerMac, const MatterLikePacket &receivedPacket) = 0;

  // Set user callback for received packets
  virtual void onPacketReceived(void (*callback)(const MatterLikePacket &, const uint8_t *senderMac)) = 0;
};
