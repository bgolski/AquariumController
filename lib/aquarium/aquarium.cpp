#include "aquarium.h"

#include <cmath>
#include <cstdio>

namespace aquarium {

bool isValidCelsius(float celsius) {
  return !std::isnan(celsius) && celsius != kProbeDisconnectedC && celsius >= -55.0f &&
         celsius <= 125.0f;
}

Temperature applyReading(Temperature current, float rawCelsius) {
  if (isValidCelsius(rawCelsius)) {
    current.present = true;
    current.celsius = rawCelsius;
  } else {
    current.present = false;
  }
  return current;
}

float toFahrenheit(float celsius) { return (celsius * 9.0f / 5.0f) + 32.0f; }

std::string formatTwoDecimals(float value) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%.2f", value);
  return buf;
}

static const char* jsonBool(bool value) { return value ? "true" : "false"; }

static std::string celsiusOrNull(const Temperature& t) {
  return t.present ? formatTwoDecimals(t.celsius) : "null";
}

static std::string fahrenheitOrNull(const Temperature& t) {
  return t.present ? formatTwoDecimals(toFahrenheit(t.celsius)) : "null";
}

std::string statusJson(const Snapshot& s) {
  std::string out = "{";
  out += "\"light\":\"";
  out += s.lightOn ? "ON" : "OFF";
  out += "\",\"lightOn\":";
  out += jsonBool(s.lightOn);
  out += ",\"tempProbePresent\":";
  out += jsonBool(s.temperature.present);
  out += ",\"temperatureC\":" + celsiusOrNull(s.temperature);
  out += ",\"temperatureF\":" + fahrenheitOrNull(s.temperature);
  out += ",\"relayPin\":" + std::to_string(s.relayPin);
  out += ",\"tempSensorPin\":" + std::to_string(s.tempSensorPin);
  out += ",\"uptimeMs\":" + std::to_string(s.uptimeMs);
  out += ",\"temperatureUpdatedMs\":" + std::to_string(s.temperatureUpdatedMs);
  out += ",\"wifiRssiDbm\":" + std::to_string(s.wifiRssiDbm);
  out += ",\"ip\":\"" + s.ip + "\"}";
  return out;
}

std::string homebridgeJson(const Snapshot& s) {
  std::string out = "{";
  out += "\"accessory\":\"AquariumController\",";
  out += "\"services\":{";
  out += "\"lightbulb\":{\"on\":";
  out += jsonBool(s.lightOn);
  out += "},";
  out += "\"temperatureSensor\":{\"statusActive\":";
  out += jsonBool(s.temperature.present);
  out += ",\"currentTemperatureC\":" + celsiusOrNull(s.temperature);
  out += "}},";
  out += "\"meta\":{";
  out += "\"uptimeMs\":" + std::to_string(s.uptimeMs);
  out += ",\"temperatureUpdatedMs\":" + std::to_string(s.temperatureUpdatedMs);
  out += ",\"wifiRssiDbm\":" + std::to_string(s.wifiRssiDbm);
  out += "}}";
  return out;
}

}  // namespace aquarium
