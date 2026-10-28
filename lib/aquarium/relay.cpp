#include "relay.h"

namespace aquarium {

bool relayLevel(bool lightOn, bool activeHigh) {
  return activeHigh ? lightOn : !lightOn;
}

bool restoredLightState(int saved) {
  return saved == 1;
}

}  // namespace aquarium
