#include <Arduino.h>

#include "display.h"
#include "sensors.h"
#include "settings.h"
#include "timer.h"

namespace {
Settings settings;
BottleTimer timers[2];
bool settingsScreen = false;
bool touchWasDown = false;
bool touchArmed = false;
bool touchCandidate = false;
uint32_t touchCandidateSince = 0;
uint32_t lastDisplayUpdate = 0;

void onTouch(const TouchPoint& point) {
  if (!settingsScreen) {
    if (displayMainSettingsHit(point)) {
      Serial.printf("Touch action: open settings at (%d,%d)\n", point.x, point.y);
      settingsScreen = true;
      displayShowSettings(settings);
    }
    return;
  }

  bool changed = false;
  for (uint8_t bottle = 0; bottle < 2; ++bottle) {
    const int rowTop = 58 + bottle * 92;
    if (point.y >= rowTop && point.y < rowTop + 65) {
      if (point.x < 82 && settings.durationSeconds[bottle] > 60) {
        settings.durationSeconds[bottle] -= 60;
        changed = true;
      } else if (point.x > 155 && settings.durationSeconds[bottle] < 24 * 60 * 60) {
        settings.durationSeconds[bottle] += 60;
        changed = true;
      }
    }
  }

  if (changed) {
    saveSettings(settings);
    displayShowSettings(settings);
  }
  if (displaySettingsDoneHit(point)) {
    Serial.printf("Touch action: close settings at (%d,%d)\n", point.x, point.y);
    settingsScreen = false;
    displayShowMain(timers, settings);
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  settings = loadSettings();
  sensorsBegin();
  displayBegin();
  for (uint8_t bottle = 0; bottle < 2; ++bottle) {
    timers[bottle].setDuration(settings.durationSeconds[bottle]);
  }
  displayShowMain(timers, settings);
}

void loop() {
  const uint32_t now = millis();
  sensorsUpdate(now);

  for (uint8_t bottle = 0; bottle < 2; ++bottle) {
    timers[bottle].update(sensorsBottleRemoved(bottle), now);
  }

  TouchPoint point{};
  const bool touchDown = displayReadTouch(point);
  if (!touchDown) {
    touchCandidate = false;
    touchWasDown = false;
    touchArmed = true;
  } else if (!touchCandidate) {
    touchCandidate = true;
    touchCandidateSince = now;
  } else if (touchArmed && !touchWasDown && now - touchCandidateSince >= 25) {
    onTouch(point);
    touchWasDown = true;
  }

  if (!settingsScreen && now - lastDisplayUpdate >= 1000) {
    displayRefreshMain(timers);
    lastDisplayUpdate = now;
  }
}