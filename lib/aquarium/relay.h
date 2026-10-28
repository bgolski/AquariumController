#pragma once

// The relay module in this build switches the light on when its input pin is
// driven HIGH. Build with -D RELAY_ACTIVE_HIGH=0 for a module that is on when LOW.
#ifndef RELAY_ACTIVE_HIGH
#define RELAY_ACTIVE_HIGH 1
#endif

namespace aquarium {

// The pin level (true = HIGH, false = LOW) that puts the light in the
// requested state for a relay that is active-high or active-low.
bool relayLevel(bool lightOn, bool activeHigh = RELAY_ACTIVE_HIGH);

// The light state to restore at boot from the saved value: 1 means on,
// 0 means off, anything else (nothing saved yet, or a bad value) means off.
bool restoredLightState(int saved);

}  // namespace aquarium
