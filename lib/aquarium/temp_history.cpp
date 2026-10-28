#include "temp_history.h"
#include <cstdio>

namespace aquarium {

bool TempHistory::add(float value, unsigned long nowMs) {
  if (samples_.empty()) {
    samples_.push_back(value);
    lastMs_ = nowMs;
    return true;
  }

  if ((nowMs - lastMs_) < kIntervalMs) {
    return false;
  }

  samples_.push_back(value);
  while (samples_.size() > kCapacity) {
    samples_.pop_front();
  }
  lastMs_ = nowMs;
  return true;
}

std::vector<float> TempHistory::values() const {
  return std::vector<float>(samples_.begin(), samples_.end());
}

std::string TempHistory::json() const {
  std::string out;
  out.push_back('[');
  bool first = true;
  for (float v : samples_) {
    if (!first) out.push_back(',');
    first = false;
    char buf[48];
    std::snprintf(buf, sizeof buf, "%.1f", static_cast<double>(v));
    out.append(buf);
  }
  out.push_back(']');
  return out;
}

}  // namespace aquarium

