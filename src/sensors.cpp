#include "sensors.h"

#include <Arduino.h>

namespace {
// Candidate free input pins for the CYD. Confirm against the exact board revision.
constexpr uint8_t SENSOR_PINS[2] = {34, 35};
constexpr uint32_t DEBOUNCE_MS = 80;
bool stableRemoved[2] = {false, false};
bool lastSampleRemoved[2] = {false, false};
uint32_t lastChangeAt[2] = {0, 0};
}

void sensorsBegin() {
  for (uint8_t index = 0; index < 2; ++index) {
    pinMode(SENSOR_PINS[index], INPUT_PULLUP);
    const bool removed = digitalRead(SENSOR_PINS[index]) == HIGH;
    stableRemoved[index] = removed;
    lastSampleRemoved[index] = removed;
  }
}

void sensorsUpdate(uint32_t now) {
  for (uint8_t index = 0; index < 2; ++index) {
    const bool sampleRemoved = digitalRead(SENSOR_PINS[index]) == HIGH;
    if (sampleRemoved != lastSampleRemoved[index]) {
      lastSampleRemoved[index] = sampleRemoved;
      lastChangeAt[index] = now;
    }
    if (now - lastChangeAt[index] >= DEBOUNCE_MS) {
      stableRemoved[index] = sampleRemoved;
    }
  }
}

bool sensorsBottleRemoved(uint8_t bottleIndex) {
  return bottleIndex < 2 && stableRemoved[bottleIndex];
}