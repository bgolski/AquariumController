#include <unity.h>

#include "relay.h"

using namespace aquarium;

void setUp() {}
void tearDown() {}

void test_active_high_relay() {
  TEST_ASSERT_TRUE(relayLevel(true, true));
  TEST_ASSERT_FALSE(relayLevel(false, true));
}

void test_active_low_relay() {
  TEST_ASSERT_FALSE(relayLevel(true, false));
  TEST_ASSERT_TRUE(relayLevel(false, false));
}

void test_default_is_active_high() {
  TEST_ASSERT_TRUE(relayLevel(true));
  TEST_ASSERT_FALSE(relayLevel(false));
}

void test_restore_saved_on() { TEST_ASSERT_TRUE(restoredLightState(1)); }

void test_restore_saved_off() { TEST_ASSERT_FALSE(restoredLightState(0)); }

void test_restore_nothing_saved() {
  TEST_ASSERT_FALSE(restoredLightState(-1));
  TEST_ASSERT_FALSE(restoredLightState(7));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_active_high_relay);
  RUN_TEST(test_active_low_relay);
  RUN_TEST(test_default_is_active_high);
  RUN_TEST(test_restore_saved_on);
  RUN_TEST(test_restore_saved_off);
  RUN_TEST(test_restore_nothing_saved);
  return UNITY_END();
}
