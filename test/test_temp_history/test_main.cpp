#include <unity.h>

#include <limits>
#include <vector>

#include "temp_history.h"

using namespace aquarium;

void setUp() {}
void tearDown() {}

void test_empty() {
  TempHistory h;
  TEST_ASSERT_EQUAL_UINT32(0, h.values().size());
  TEST_ASSERT_EQUAL_STRING("[]", h.json().c_str());
}

void test_one_sample_per_minute() {
  TempHistory h;
  TEST_ASSERT_TRUE(h.add(77.4f, 5000));
  TEST_ASSERT_FALSE(h.add(77.9f, 64999));
  TEST_ASSERT_TRUE(h.add(77.5f, 65000));
  TEST_ASSERT_FALSE(h.add(70.0f, 65001));
  TEST_ASSERT_TRUE(h.add(78.0f, 200000));
  std::vector<float> v = h.values();
  TEST_ASSERT_EQUAL_UINT32(3, v.size());
  TEST_ASSERT_EQUAL_FLOAT(77.4f, v[0]);
  TEST_ASSERT_EQUAL_FLOAT(77.5f, v[1]);
  TEST_ASSERT_EQUAL_FLOAT(78.0f, v[2]);
  TEST_ASSERT_EQUAL_STRING("[77.4,77.5,78.0]", h.json().c_str());
}

void test_first_sample_at_time_zero() {
  TempHistory h;
  TEST_ASSERT_TRUE(h.add(20.0f, 0));
  TEST_ASSERT_FALSE(h.add(21.0f, 1000));
  TEST_ASSERT_TRUE(h.add(21.0f, 60000));
}

void test_keeps_last_hour() {
  TempHistory h;
  for (int i = 0; i < 75; i++) h.add(static_cast<float>(i), 60000UL * i);
  std::vector<float> v = h.values();
  TEST_ASSERT_EQUAL_UINT32(60, v.size());
  TEST_ASSERT_EQUAL_FLOAT(15.0f, v.front());
  TEST_ASSERT_EQUAL_FLOAT(74.0f, v.back());
}

void test_negative_and_rounding() {
  TempHistory h;
  h.add(-1.24f, 0);
  h.add(25.06f, 60000);
  TEST_ASSERT_EQUAL_STRING("[-1.2,25.1]", h.json().c_str());
}

void test_wraparound() {
  const unsigned long start = std::numeric_limits<unsigned long>::max() - 100;
  TempHistory h;
  TEST_ASSERT_TRUE(h.add(1.0f, start));
  TEST_ASSERT_FALSE(h.add(2.0f, start + 59999));
  TEST_ASSERT_TRUE(h.add(2.0f, start + 60000));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_empty);
  RUN_TEST(test_one_sample_per_minute);
  RUN_TEST(test_first_sample_at_time_zero);
  RUN_TEST(test_keeps_last_hour);
  RUN_TEST(test_negative_and_rounding);
  RUN_TEST(test_wraparound);
  return UNITY_END();
}
