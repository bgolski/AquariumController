#include <unity.h>

#include <cmath>

#include "aquarium.h"

using namespace aquarium;

void setUp() {}
void tearDown() {}

void test_valid_celsius_range() {
  TEST_ASSERT_TRUE(isValidCelsius(24.5f));
  TEST_ASSERT_TRUE(isValidCelsius(-55.0f));
  TEST_ASSERT_TRUE(isValidCelsius(125.0f));
  TEST_ASSERT_FALSE(isValidCelsius(kProbeDisconnectedC));
  TEST_ASSERT_FALSE(isValidCelsius(-55.1f));
  TEST_ASSERT_FALSE(isValidCelsius(125.1f));
  TEST_ASSERT_FALSE(isValidCelsius(NAN));
}

void test_valid_reading_replaces_value() {
  Temperature t = applyReading(Temperature{}, 25.25f);
  TEST_ASSERT_TRUE(t.present);
  TEST_ASSERT_EQUAL_FLOAT(25.25f, t.celsius);
}

void test_failed_reading_keeps_last_value() {
  Temperature t = applyReading(Temperature{}, 25.25f);
  t = applyReading(t, kProbeDisconnectedC);
  TEST_ASSERT_FALSE(t.present);
  TEST_ASSERT_EQUAL_FLOAT(25.25f, t.celsius);
}

void test_fahrenheit() {
  TEST_ASSERT_EQUAL_FLOAT(32.0f, toFahrenheit(0.0f));
  TEST_ASSERT_EQUAL_FLOAT(77.0f, toFahrenheit(25.0f));
  TEST_ASSERT_EQUAL_STRING("77.45", formatTwoDecimals(toFahrenheit(25.25f)).c_str());
}

static Snapshot sample() {
  Snapshot s;
  s.lightOn = true;
  s.temperature = applyReading(Temperature{}, 25.25f);
  s.relayPin = 25;
  s.tempSensorPin = 26;
  s.uptimeMs = 123456;
  s.temperatureUpdatedMs = 120000;
  s.wifiRssiDbm = -61;
  s.ip = "10.0.0.5";
  return s;
}

void test_status_json() {
  TEST_ASSERT_EQUAL_STRING(
      "{\"light\":\"ON\",\"lightOn\":true,\"tempProbePresent\":true,"
      "\"temperatureC\":25.25,\"temperatureF\":77.45,\"relayPin\":25,\"tempSensorPin\":26,"
      "\"uptimeMs\":123456,\"temperatureUpdatedMs\":120000,\"wifiRssiDbm\":-61,"
      "\"ip\":\"10.0.0.5\"}",
      statusJson(sample()).c_str());
}

void test_status_json_without_probe() {
  Snapshot s = sample();
  s.lightOn = false;
  s.temperature.present = false;
  std::string json = statusJson(s);
  TEST_ASSERT_NOT_EQUAL(std::string::npos, json.find("\"light\":\"OFF\",\"lightOn\":false"));
  TEST_ASSERT_NOT_EQUAL(std::string::npos,
                        json.find("\"temperatureC\":null,\"temperatureF\":null"));
}

void test_homebridge_json() {
  TEST_ASSERT_EQUAL_STRING(
      "{\"accessory\":\"AquariumController\",\"services\":{\"lightbulb\":{\"on\":true},"
      "\"temperatureSensor\":{\"statusActive\":true,\"currentTemperatureC\":25.25}},"
      "\"meta\":{\"uptimeMs\":123456,\"temperatureUpdatedMs\":120000,\"wifiRssiDbm\":-61}}",
      homebridgeJson(sample()).c_str());
}

void test_homebridge_json_without_probe() {
  Snapshot s = sample();
  s.temperature.present = false;
  std::string json = homebridgeJson(s);
  TEST_ASSERT_NOT_EQUAL(std::string::npos,
                        json.find("\"statusActive\":false,\"currentTemperatureC\":null"));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_valid_celsius_range);
  RUN_TEST(test_valid_reading_replaces_value);
  RUN_TEST(test_failed_reading_keeps_last_value);
  RUN_TEST(test_fahrenheit);
  RUN_TEST(test_status_json);
  RUN_TEST(test_status_json_without_probe);
  RUN_TEST(test_homebridge_json);
  RUN_TEST(test_homebridge_json_without_probe);
  return UNITY_END();
}
