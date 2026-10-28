#include "setup_fallback.h"

namespace aquarium {

SetupFallback::SetupFallback(unsigned long waitMs) : waitMs_(waitMs) {}

bool SetupFallback::update(bool hasSavedNetwork, bool connected, unsigned long nowMs) {
  if (!started_) {
    started_ = true;
    startMs_ = nowMs;
  }

  if (connected) {
    everConnected_ = true;
    return false;
  }

  if (everConnected_) {
    return false;
  }

  if (!hasSavedNetwork) {
    return true;
  }

  return (nowMs - startMs_) >= waitMs_;
}

} // namespace aquarium

