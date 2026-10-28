#pragma once

// Non-blocking timing for the DS18B20 temperature probe. The firmware calls
// next() on every loop iteration and acts on the result, so HTTP requests are
// never held up by a temperature conversion.

namespace aquarium {

enum class ProbeAction { None, Request, Read };

class ProbeSchedule {
 public:
  // refreshMs: time between the starts of two conversions.
  // conversionMs: how long a conversion takes before it can be read.
  ProbeSchedule(unsigned long refreshMs, unsigned long conversionMs);

  // Request on the first call. Read once, on the first call at least
  // conversionMs after a Request. After that Read, Request again on the first
  // call at least refreshMs after the previous Request. None otherwise.
  // Works across millis() wrapping around (use unsigned subtraction).
  ProbeAction next(unsigned long nowMs);

 private:
  unsigned long refreshMs_;
  unsigned long conversionMs_;
  unsigned long requestedAtMs_ = 0;
  bool started_ = false;
  bool converting_ = false;
};

// True when there is a valid reading (hasReading) taken at lastValidMs that is
// at most maxAgeMs old at nowMs. Works across millis() wrapping around.
bool readingFresh(bool hasReading, unsigned long lastValidMs, unsigned long nowMs,
                  unsigned long maxAgeMs);

}  // namespace aquarium
