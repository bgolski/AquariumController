#include <unity.h>

#include <cstdint>
#include <string>

#include "update_password.h"

using namespace aquarium;

static uint32_t counter = 0;
static uint32_t counting() { return counter++; }

static uint32_t fixedValue = 0;
static uint32_t fixed() { return fixedValue; }

static uint32_t lcgState = 1;
static uint32_t lcg() {
  lcgState = lcgState * 1664525u + 1013904223u;
  return lcgState;
}

void setUp() {}
void tearDown() {}

void test_alphabet_has_no_look_alikes() {
  TEST_ASSERT_EQUAL_UINT32(57, kPasswordAlphabetSize);
  const std::string alphabet(kPasswordAlphabet);
  for (char c : std::string("0O1lI")) {
    TEST_ASSERT_EQUAL(std::string::npos, alphabet.find(c));
  }
}

void test_picks_characters_in_order() {
  counter = 0;
  TEST_ASSERT_EQUAL_STRING("ABCDEFGHJKLMNPQRSTUV", makePassword(counting).c_str());
  TEST_ASSERT_EQUAL_UINT32(20, counter);
}

void test_wraps_large_values() {
  fixedValue = 57;
  TEST_ASSERT_EQUAL_STRING("AAAAAAAAAAAA", makePassword(fixed, 12).c_str());
  fixedValue = 56;
  TEST_ASSERT_EQUAL_STRING("999999999999", makePassword(fixed, 12).c_str());
  fixedValue = 0xFFFFFFFFu;  // 4294967295 % 57 == 24
  TEST_ASSERT_EQUAL_STRING("aaaaaaaaaaaa", makePassword(fixed, 12).c_str());
}

void test_length() {
  counter = 0;
  TEST_ASSERT_EQUAL_UINT32(32, makePassword(counting, 32).size());
  TEST_ASSERT_EQUAL_UINT32(0, makePassword(counting, 0).size());
}

void test_generated_passwords_are_valid() {
  lcgState = 1;
  for (int i = 0; i < 200; i++) {
    std::string p = makePassword(lcg);
    TEST_ASSERT_EQUAL_UINT32(20, p.size());
    TEST_ASSERT_TRUE(validPassword(p));
    for (char c : p) {
      TEST_ASSERT_NOT_EQUAL(std::string::npos, std::string(kPasswordAlphabet).find(c));
    }
  }
}

void test_valid_password_length() {
  TEST_ASSERT_FALSE(validPassword(""));
  TEST_ASSERT_FALSE(validPassword(std::string(11, 'a')));
  TEST_ASSERT_TRUE(validPassword(std::string(12, 'a')));
  TEST_ASSERT_TRUE(validPassword(std::string(64, 'a')));
  TEST_ASSERT_FALSE(validPassword(std::string(65, 'a')));
}

void test_valid_password_characters() {
  TEST_ASSERT_TRUE(validPassword("!abcdefghij~"));
  TEST_ASSERT_FALSE(validPassword("abcdef ghijkl"));
  TEST_ASSERT_FALSE(validPassword("abcdefghijkl\n"));
  TEST_ASSERT_FALSE(validPassword(std::string("abcdefghijkl") + '\x7f'));
  TEST_ASSERT_FALSE(validPassword(std::string("abcdefghijkl") + '\xe9'));
  TEST_ASSERT_FALSE(validPassword(std::string("abcdef\0ghijkl", 13)));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_alphabet_has_no_look_alikes);
  RUN_TEST(test_picks_characters_in_order);
  RUN_TEST(test_wraps_large_values);
  RUN_TEST(test_length);
  RUN_TEST(test_generated_passwords_are_valid);
  RUN_TEST(test_valid_password_length);
  RUN_TEST(test_valid_password_characters);
  return UNITY_END();
}
