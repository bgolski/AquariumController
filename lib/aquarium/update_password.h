#pragma once

// Password that protects firmware updates over Wi-Fi. The board makes one the
// first time it starts and keeps it, so no password ever lives in the source.

#include <cstddef>
#include <cstdint>
#include <string>

namespace aquarium {

// Letters and digits for generated passwords, without look-alikes (0 O 1 l I).
constexpr char kPasswordAlphabet[] =
    "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
constexpr std::size_t kPasswordAlphabetSize = sizeof(kPasswordAlphabet) - 1;

// Builds a password of `length` characters, calling randomFn once per
// character: character i is kPasswordAlphabet[randomFn() % kPasswordAlphabetSize].
std::string makePassword(uint32_t (*randomFn)(), std::size_t length = 20);

// True when `password` can protect updates: 12 to 64 characters, each a
// printable ASCII character other than a space (codes 33 to 126).
bool validPassword(const std::string& password);

}  // namespace aquarium
