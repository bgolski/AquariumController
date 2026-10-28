#pragma once

// Decides when the setup network opens on its own.
//
// With no saved network it is on at once. With a saved network it opens only
// if the board has not connected since it started and has been trying for
// waitMs; it closes as soon as the board connects. Once the board has connected
// it never opens on its own again: later drops are handled by reconnecting.

namespace aquarium {

class SetupFallback {
 public:
  explicit SetupFallback(unsigned long waitMs = 600000UL);

  // Call on every loop iteration. The first call marks the start time.
  // Returns true while the setup network should be on. Works across millis()
  // wrapping around.
  bool update(bool hasSavedNetwork, bool connected, unsigned long nowMs);

 private:
  unsigned long waitMs_;
  bool started_ = false;
  bool everConnected_ = false;
  unsigned long startMs_ = 0;
};

}  // namespace aquarium
