#pragma once

#include <stdint.h>

class BottleTimer {
 public:
  void setDuration(uint32_t durationSeconds);
  void update(bool removed, uint32_t now);
  bool isRemoved() const;
  bool hasStarted() const;
  uint32_t remainingSeconds(uint32_t now) const;
  uint8_t visibleDots(uint32_t now) const;

 private:
  uint32_t durationSeconds_ = 600;
  uint32_t startedAt_ = 0;
  uint32_t frozenElapsed_ = 0;
  bool removed_ = false;
  bool started_ = false;
};