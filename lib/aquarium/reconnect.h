#pragma once

// Wi-Fi reconnect timing with backoff. The firmware calls due() on every loop
// iteration with the current connection state.

namespace aquarium {

// Wait before reconnect attempt `attempt` (1 = the first try after a drop):
// 1000 ms, doubling for each later attempt, never more than 60000 ms.
// Attempt 0 is treated like attempt 1. Must not overflow for large attempts.
unsigned long reconnectDelayMs(unsigned int attempt);

class ReconnectTimer {
 public:
  // While connected: returns false, and forgets any earlier drop.
  // On the first call after the connection drops: remembers nowMs, returns false.
  // While still disconnected: returns true (time to call WiFi.reconnect()) once
  // reconnectDelayMs(attempts() + 1) has passed since the drop or since the
  // previous true; each true increments attempts().
  // Works across millis() wrapping around.
  bool due(bool connected, unsigned long nowMs);

  // Reconnect attempts made since the current drop (0 while connected).
  unsigned int attempts() const;

  // True exactly once, on the first call after due() has seen the connection
  // come back after a drop (the firmware restarts mDNS then). False otherwise.
  bool reconnected();

 private:
  bool disconnected_ = false;
  bool justReconnected_ = false;
  unsigned int attempts_ = 0;
  unsigned long lastMs_ = 0;
};

}  // namespace aquarium
