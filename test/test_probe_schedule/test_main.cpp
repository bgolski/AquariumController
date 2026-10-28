#include <unity.h>

#include <limits>

#include "probe_schedule.h"

using namespace aquarium;

void setUp() {}
void tearDown() {}

static int action(ProbeAction a) { return static_cast<int>(a); }

void test_first_call_requests() {
  ProbeSchedule s(5000, 750);
  TEST_ASSERT_EQUAL(action(ProbeAction::Request), action(s.next(1000)));
}

void test_reads_after_conversion_time() {
  ProbeSchedule s(5000, 750);
  s.next(1000);
  TEST_ASSERT_EQUAL(action(ProbeAction::None), action(s.next(1749)));
  TEST_ASSERT_EQUAL(action(ProbeAction::Read), action(s.next(1750)));
}

void test_reads_only_once() {
  ProbeSchedule s(5000, 750);
  s.next(1000);
  s.next(1750);
  TEST_ASSERT_EQUAL(action(ProbeAction::None), action(s.next(1751)));
  TEST_ASSERT_EQUAL(action(ProbeAction::None), action(s.next(3000)));
}

void test_requests_again_after_refresh() {
  ProbeSchedule s(5000, 750);
  s.next(1000);
  s.next(1750);
  TEST_ASSERT_EQUAL(action(ProbeAction::None), action(s.next(5999)));
  TEST_ASSERT_EQUAL(action(ProbeAction::Request), action(s.next(6000)));
  TEST_ASSERT_EQUAL(action(ProbeAction::Read), action(s.next(6750)));
}

void test_late_loop_reads_before_requesting() {
  ProbeSchedule s(5000, 750);
  s.next(1000);
  TEST_ASSERT_EQUAL(action(ProbeAction::Read), action(s.next(20000)));
  TEST_ASSERT_EQUAL(action(ProbeAction::Request), action(s.next(20001)));
}

void test_wraparound() {
  const unsigned long start = std::numeric_limits<unsigned long>::max() - 100;
  ProbeSchedule s(5000, 750);
  TEST_ASSERT_EQUAL(action(ProbeAction::Request), action(s.next(start)));
  TEST_ASSERT_EQUAL(action(ProbeAction::None), action(s.next(start + 500)));
  TEST_ASSERT_EQUAL(action(ProbeAction::Read), action(s.next(start + 750)));
  TEST_ASSERT_EQUAL(action(ProbeAction::Request), action(s.next(start + 5000)));
}

void test_fresh_reading() {
  TEST_ASSERT_TRUE(readingFresh(true, 1000, 1000, 60000));
  TEST_ASSERT_TRUE(readingFresh(true, 1000, 61000, 60000));
  TEST_ASSERT_FALSE(readingFresh(true, 1000, 61001, 60000));
}

void test_no_reading_is_never_fresh() {
  TEST_ASSERT_FALSE(readingFresh(false, 1000, 1000, 60000));
}

void test_fresh_across_wraparound() {
  const unsigned long last = std::numeric_limits<unsigned long>::max() - 10;
  TEST_ASSERT_TRUE(readingFresh(true, last, last + 30000, 60000));
  TEST_ASSERT_FALSE(readingFresh(true, last, last + 70000, 60000));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_first_call_requests);
  RUN_TEST(test_reads_after_conversion_time);
  RUN_TEST(test_reads_only_once);
  RUN_TEST(test_requests_again_after_refresh);
  RUN_TEST(test_late_loop_reads_before_requesting);
  RUN_TEST(test_wraparound);
  RUN_TEST(test_fresh_reading);
  RUN_TEST(test_no_reading_is_never_fresh);
  RUN_TEST(test_fresh_across_wraparound);
  return UNITY_END();
}
