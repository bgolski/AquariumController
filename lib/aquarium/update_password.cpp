#include "update_password.h"

namespace aquarium {

std::string makePassword(uint32_t (*randomFn)(), std::size_t length) {
  std::string result;
  result.reserve(length);
  for (std::size_t i = 0; i < length; i++) {
    result += kPasswordAlphabet[randomFn() % kPasswordAlphabetSize];
  }
  return result;
}

bool validPassword(const std::string& password) {
  if (password.size() < 12 || password.size() > 64) return false;
  for (unsigned char c : password) {
    if (c < 33 || c > 126) return false;
  }
  return true;
}

}  // namespace aquarium
