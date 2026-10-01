#pragma once

#include <stdint.h>

#include "settings.h"
#include "timer.h"

struct TouchPoint {
  int16_t x;
  int16_t y;
};

void displayBegin();
bool displayReadTouch(TouchPoint& point);
bool displayMainSettingsHit(const TouchPoint& point);
bool displaySettingsDoneHit(const TouchPoint& point);
void displayShowMain(const BottleTimer timers[2], const Settings& settings);
void displayRefreshMain(const BottleTimer timers[2]);
void displayShowSettings(const Settings& settings);