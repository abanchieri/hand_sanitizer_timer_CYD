#include "timer.h"

#include <Arduino.h>

void BottleTimer::setDuration(uint32_t durationSeconds) {
  durationSeconds_ = durationSeconds > 0 ? durationSeconds : 60;
}

void BottleTimer::update(bool removed, uint32_t now) {
  if (removed == removed_) return;

  if (removed) {
    removed_ = true;
    started_ = true;
    frozenElapsed_ = 0;
    startedAt_ = now;
  } else {
    if (removed_) {
      frozenElapsed_ = min((now - startedAt_) / 1000, durationSeconds_);
    }
    removed_ = false;
  }
}

bool BottleTimer::isRemoved() const { return removed_; }

bool BottleTimer::hasStarted() const { return started_; }

uint32_t BottleTimer::remainingSeconds(uint32_t now) const {
  uint32_t elapsed = frozenElapsed_;
  if (removed_) elapsed = min((now - startedAt_) / 1000, durationSeconds_);
  return elapsed >= durationSeconds_ ? 0 : durationSeconds_ - elapsed;
}

uint8_t BottleTimer::visibleDots(uint32_t now) const {
  if (!started_ || durationSeconds_ == 0) return 6;
  const uint32_t remaining = remainingSeconds(now);
  if (remaining == 0) return 0;
  return (remaining * 6 + durationSeconds_ - 1) / durationSeconds_;
}