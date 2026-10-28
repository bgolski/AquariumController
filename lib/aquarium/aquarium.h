#pragma once

// Board-independent logic for the aquarium controller: temperature handling and
// the JSON payloads. No Arduino headers, so it builds and tests on a computer
// (`pio test -e native`) as well as on the ESP32.

#include <string>

namespace aquarium {

// DallasTemperature reports this when the probe does not answer.
constexpr float kProbeDisconnectedC = -127.0f;

struct Temperature {
  bool present = false;  // false until a valid reading arrives, or after a failed one
  float celsius = 0.0f;  // last valid reading; kept when a later read fails
};

// True for a reading the DS18B20 can actually produce (-55 to 125 °C).
bool isValidCelsius(float celsius);

// Applies a raw probe reading: a valid one replaces the stored value, an invalid
// one marks the probe absent and keeps the last good value.
Temperature applyReading(Temperature current, float rawCelsius);

float toFahrenheit(float celsius);

// Everything the status endpoints report, gathered by the firmware.
struct Snapshot {
  bool lightOn = false;
  Temperature temperature;
  int relayPin = 0;
  int tempSensorPin = 0;
  unsigned long uptimeMs = 0;
  unsigned long temperatureUpdatedMs = 0;
  int wifiRssiDbm = 0;
  std::string ip;
};

// GET /status/json
std::string statusJson(const Snapshot& s);

// GET /homebridge/payload
std::string homebridgeJson(const Snapshot& s);

// Two decimal places, as the plain-text temperature endpoints return.
std::string formatTwoDecimals(float value);

}  // namespace aquarium
