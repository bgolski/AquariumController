#include <unity.h>

#include <string>
#include <vector>

#include "network_list.h"

using namespace aquarium;

void setUp() {}
void tearDown() {}

void test_signal_bars() {
  TEST_ASSERT_EQUAL(4, signalBars(-30));
  TEST_ASSERT_EQUAL(4, signalBars(-55));
  TEST_ASSERT_EQUAL(3, signalBars(-56));
  TEST_ASSERT_EQUAL(3, signalBars(-65));
  TEST_ASSERT_EQUAL(2, signalBars(-66));
  TEST_ASSERT_EQUAL(2, signalBars(-75));
  TEST_ASSERT_EQUAL(1, signalBars(-85));
  TEST_ASSERT_EQUAL(0, signalBars(-86));
  TEST_ASSERT_EQUAL(0, signalBars(-100));
}

void test_dedupes_sorts_and_drops_hidden() {
  std::vector<Network> scan = {
      {"Cafe", -80, false}, {"", -40, true},     {"Home", -70, true},
      {"Home", -50, true},  {"Alpha", -80, true}, {"Home", -90, true},
  };
  std::vector<Network> out = networksToOffer(scan);
  TEST_ASSERT_EQUAL_UINT32(3, out.size());
  TEST_ASSERT_EQUAL_STRING("Home", out[0].ssid.c_str());
  TEST_ASSERT_EQUAL(-50, out[0].rssi);
  TEST_ASSERT_EQUAL_STRING("Alpha", out[1].ssid.c_str());
  TEST_ASSERT_EQUAL_STRING("Cafe", out[2].ssid.c_str());
  TEST_ASSERT_FALSE(out[2].secure);
}

void test_limit() {
  std::vector<Network> scan;
  for (int i = 0; i < 30; i++) scan.push_back({"n" + std::to_string(i), -40 - i, true});
  std::vector<Network> out = networksToOffer(scan, 5);
  TEST_ASSERT_EQUAL_UINT32(5, out.size());
  TEST_ASSERT_EQUAL_STRING("n0", out[0].ssid.c_str());
  TEST_ASSERT_EQUAL_STRING("n4", out[4].ssid.c_str());
  TEST_ASSERT_EQUAL_UINT32(20, networksToOffer(scan).size());
  TEST_ASSERT_EQUAL_UINT32(0, networksToOffer({}).size());
  TEST_ASSERT_EQUAL_UINT32(0, networksToOffer(scan, 0).size());
}

void test_json_string() {
  TEST_ASSERT_EQUAL_STRING("\"Home\"", jsonString("Home").c_str());
  TEST_ASSERT_EQUAL_STRING("\"\"", jsonString("").c_str());
  TEST_ASSERT_EQUAL_STRING("\"Bob\\\"s \\\\ net\"", jsonString("Bob\"s \\ net").c_str());
  TEST_ASSERT_EQUAL_STRING("\"a\\nb\\rc\\td\"", jsonString("a\nb\rc\td").c_str());
  TEST_ASSERT_EQUAL_STRING("\"x\\u0001\\u001f\"", jsonString("x\x01\x1f").c_str());
  TEST_ASSERT_EQUAL_STRING("\"Caf\xc3\xa9 \xf0\x9f\x90\x9f\"", jsonString("Caf\xc3\xa9 \xf0\x9f\x90\x9f").c_str());
}

void test_networks_json() {
  std::vector<Network> n = {{"Home", -50, true}, {"Ca\"fe", -84, false}};
  TEST_ASSERT_EQUAL_STRING(
      "[{\"ssid\":\"Home\",\"bars\":4,\"secure\":true},"
      "{\"ssid\":\"Ca\\\"fe\",\"bars\":1,\"secure\":false}]",
      networksJson(n).c_str());
  TEST_ASSERT_EQUAL_STRING("[]", networksJson({}).c_str());
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_signal_bars);
  RUN_TEST(test_dedupes_sorts_and_drops_hidden);
  RUN_TEST(test_limit);
  RUN_TEST(test_json_string);
  RUN_TEST(test_networks_json);
  return UNITY_END();
}
