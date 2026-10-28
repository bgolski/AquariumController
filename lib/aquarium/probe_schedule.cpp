#include "probe_schedule.h"

namespace aquarium {

ProbeSchedule::ProbeSchedule(unsigned long refreshMs, unsigned long conversionMs)
    : refreshMs_(refreshMs), conversionMs_(conversionMs) {}

ProbeAction ProbeSchedule::next(unsigned long nowMs) {
  if (!started_) {
    started_ = true;
    converting_ = true;
    requestedAtMs_ = nowMs;
    return ProbeAction::Request;
  }

  unsigned long elapsed = nowMs - requestedAtMs_;

  if (converting_) {
    if (elapsed >= conversionMs_) {
      converting_ = false;
      return ProbeAction::Read;
    }
    return ProbeAction::None;
  }

  if (elapsed >= refreshMs_) {
    converting_ = true;
    requestedAtMs_ = nowMs;
    return ProbeAction::Request;
  }
  return ProbeAction::None;
}

bool readingFresh(bool hasReading, unsigned long lastValidMs, unsigned long nowMs,
                  unsigned long maxAgeMs) {
  return hasReading && (nowMs - lastValidMs) <= maxAgeMs;
}

}  // namespace aquarium
