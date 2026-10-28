#include <unity.h>

#include <limits>

#include "reconnect.h"

using namespace aquarium;

void setUp() {}
void tearDown() {}

void test_backoff_schedule() {
  TEST_ASSERT_EQUAL_UINT32(1000, reconnectDelayMs(0));
  TEST_ASSERT_EQUAL_UINT32(1000, reconnectDelayMs(1));
  TEST_ASSERT_EQUAL_UINT32(2000, reconnectDelayMs(2));
  TEST_ASSERT_EQUAL_UINT32(4000, reconnectDelayMs(3));
  TEST_ASSERT_EQUAL_UINT32(32000, reconnectDelayMs(6));
  TEST_ASSERT_EQUAL_UINT32(60000, reconnectDelayMs(7));
  TEST_ASSERT_EQUAL_UINT32(60000, reconnectDelayMs(100));
  TEST_ASSERT_EQUAL_UINT32(60000, reconnectDelayMs(4000000000u));
}

void test_connected_never_due() {
  ReconnectTimer t;
  TEST_ASSERT_FALSE(t.due(true, 0));
  TEST_ASSERT_FALSE(t.due(true, 100000));
  TEST_ASSERT_EQUAL_UINT32(0, t.attempts());
  TEST_ASSERT_FALSE(t.reconnected());
}

void test_retries_with_backoff() {
  ReconnectTimer t;
  t.due(true, 0);
  TEST_ASSERT_FALSE(t.due(false, 10000));  // drop noticed
  TEST_ASSERT_FALSE(t.due(false, 10999));
  TEST_ASSERT_TRUE(t.due(false, 11000));   // attempt 1 after 1 s
  TEST_ASSERT_EQUAL_UINT32(1, t.attempts());
  TEST_ASSERT_FALSE(t.due(false, 11001));
  TEST_ASSERT_FALSE(t.due(false, 12999));
  TEST_ASSERT_TRUE(t.due(false, 13000));   // attempt 2 after 2 s more
  TEST_ASSERT_FALSE(t.due(false, 16999));
  TEST_ASSERT_TRUE(t.due(false, 17000));   // attempt 3 after 4 s more
  TEST_ASSERT_EQUAL_UINT32(3, t.attempts());
}

void test_reconnected_once_then_resets() {
  ReconnectTimer t;
  t.due(false, 0);
  t.due(false, 1000);
  TEST_ASSERT_FALSE(t.reconnected());
  TEST_ASSERT_FALSE(t.due(true, 1500));
  TEST_ASSERT_EQUAL_UINT32(0, t.attempts());
  TEST_ASSERT_TRUE(t.reconnected());
  TEST_ASSERT_FALSE(t.reconnected());
  // A new drop starts the backoff from the beginning.
  TEST_ASSERT_FALSE(t.due(false, 5000));
  TEST_ASSERT_TRUE(t.due(false, 6000));
  TEST_ASSERT_EQUAL_UINT32(1, t.attempts());
}

void test_no_reconnected_without_drop() {
  ReconnectTimer t;
  t.due(true, 0);
  t.due(true, 1000);
  TEST_ASSERT_FALSE(t.reconnected());
}

void test_wraparound() {
  const unsigned long start = std::numeric_limits<unsigned long>::max() - 200;
  ReconnectTimer t;
  TEST_ASSERT_FALSE(t.due(false, start));
  TEST_ASSERT_FALSE(t.due(false, start + 999));
  TEST_ASSERT_TRUE(t.due(false, start + 1000));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_backoff_schedule);
  RUN_TEST(test_connected_never_due);
  RUN_TEST(test_retries_with_backoff);
  RUN_TEST(test_reconnected_once_then_resets);
  RUN_TEST(test_no_reconnected_without_drop);
  RUN_TEST(test_wraparound);
  return UNITY_END();
}
