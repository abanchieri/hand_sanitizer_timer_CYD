#pragma once

#include <stdint.h>

struct Settings {
  uint32_t durationSeconds[2];
};

Settings loadSettings();
void saveSettings(const Settings& settings);