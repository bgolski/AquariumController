#include "reconnect.h"

namespace aquarium {

unsigned long reconnectDelayMs(unsigned int attempt) {
  if (attempt <= 1) return 1000UL;
  if (attempt >= 7) return 60000UL;
  unsigned long delay = 1000UL << (attempt - 1);
  if (delay > 60000UL) delay = 60000UL;
  return delay;
}

bool ReconnectTimer::due(bool connected, unsigned long nowMs) {
  if (connected) {
    if (disconnected_) justReconnected_ = true;
    disconnected_ = false;
    attempts_ = 0;
    return false;
  }
  // Not connected.
  if (!disconnected_) {
    disconnected_ = true;
    lastMs_ = nowMs;
    attempts_ = 0;
    return false;
  }
  // Still disconnected — check if it's time to retry.
  if (nowMs - lastMs_ >= reconnectDelayMs(attempts_ + 1)) {
    attempts_++;
    lastMs_ = nowMs;
    return true;
  }
  return false;
}

unsigned int ReconnectTimer::attempts() const { return attempts_; }

bool ReconnectTimer::reconnected() {
  bool v = justReconnected_;
  justReconnected_ = false;
  return v;
}

}  // namespace aquarium
