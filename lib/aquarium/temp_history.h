#pragma once

// The last hour of water temperatures, for the trend line on the home page.

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

namespace aquarium {

class TempHistory {
 public:
  static constexpr std::size_t kCapacity = 60;
  static constexpr unsigned long kIntervalMs = 60000UL;

  // Stores `value` when nothing is stored yet or at least kIntervalMs has
  // passed since the last stored sample (works across millis() wrapping
  // around); drops the oldest sample beyond kCapacity. Returns whether it stored.
  bool add(float value, unsigned long nowMs);

  // Stored samples, oldest first.
  std::vector<float> values() const;

  // Stored samples as a JSON array, oldest first, each with one decimal
  // (printf "%.1f"), no spaces: [77.4,77.5]. Empty history gives [].
  std::string json() const;

 private:
  std::deque<float> samples_;
  unsigned long lastMs_ = 0;
};

}  // namespace aquarium
