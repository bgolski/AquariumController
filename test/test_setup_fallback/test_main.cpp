#include <unity.h>

#include <limits>

#include "setup_fallback.h"

using namespace aquarium;

void setUp() {}
void tearDown() {}

void test_no_saved_network_opens_at_once() {
  SetupFallback f(600000);
  TEST_ASSERT_TRUE(f.update(false, false, 5000));
  TEST_ASSERT_TRUE(f.update(false, false, 6000));
}

void test_saved_network_waits() {
  SetupFallback f(600000);
  TEST_ASSERT_FALSE(f.update(true, false, 1000));
  TEST_ASSERT_FALSE(f.update(true, false, 600999));
  TEST_ASSERT_TRUE(f.update(true, false, 601000));
  TEST_ASSERT_TRUE(f.update(true, false, 900000));
}

void test_closes_when_connected_and_never_reopens() {
  SetupFallback f(600000);
  f.update(true, false, 0);
  TEST_ASSERT_TRUE(f.update(true, false, 700000));
  TEST_ASSERT_FALSE(f.update(true, true, 710000));
  TEST_ASSERT_FALSE(f.update(true, false, 720000));
  TEST_ASSERT_FALSE(f.update(true, false, 5000000));
}

void test_connected_first_never_opens() {
  SetupFallback f(600000);
  TEST_ASSERT_FALSE(f.update(true, true, 0));
  TEST_ASSERT_FALSE(f.update(true, false, 10000000));
  TEST_ASSERT_FALSE(f.update(false, false, 20000000));
}

void test_setup_from_scratch_then_connected() {
  SetupFallback f(600000);
  TEST_ASSERT_TRUE(f.update(false, false, 0));
  TEST_ASSERT_FALSE(f.update(true, true, 60000));
  TEST_ASSERT_FALSE(f.update(true, false, 9000000));
}

void test_default_wait_is_ten_minutes() {
  SetupFallback f;
  TEST_ASSERT_FALSE(f.update(true, false, 0));
  TEST_ASSERT_FALSE(f.update(true, false, 599999));
  TEST_ASSERT_TRUE(f.update(true, false, 600000));
}

void test_wraparound() {
  const unsigned long start = std::numeric_limits<unsigned long>::max() - 100;
  SetupFallback f(1000);
  TEST_ASSERT_FALSE(f.update(true, false, start));
  TEST_ASSERT_FALSE(f.update(true, false, start + 999));
  TEST_ASSERT_TRUE(f.update(true, false, start + 1000));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_no_saved_network_opens_at_once);
  RUN_TEST(test_saved_network_waits);
  RUN_TEST(test_closes_when_connected_and_never_reopens);
  RUN_TEST(test_connected_first_never_opens);
  RUN_TEST(test_setup_from_scratch_then_connected);
  RUN_TEST(test_default_wait_is_ten_minutes);
  RUN_TEST(test_wraparound);
  return UNITY_END();
}
