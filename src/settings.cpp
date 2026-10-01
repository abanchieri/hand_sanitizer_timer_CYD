#include "settings.h"

#include <Preferences.h>

namespace {
constexpr uint32_t DEFAULT_DURATION_1 = 10 * 60;
constexpr uint32_t DEFAULT_DURATION_2 = 12 * 60;
constexpr uint32_t MIN_DURATION = 60;
constexpr uint32_t MAX_DURATION = 24 * 60 * 60;
Preferences preferences;

uint32_t boundedDuration(uint32_t value, uint32_t fallback) {
  return value >= MIN_DURATION && value <= MAX_DURATION ? value : fallback;
}
}

Settings loadSettings() {
  Settings settings{DEFAULT_DURATION_1, DEFAULT_DURATION_2};
  preferences.begin("sanitizer", true);
  settings.durationSeconds[0] = boundedDuration(
      preferences.getUInt("duration1", DEFAULT_DURATION_1), DEFAULT_DURATION_1);
  settings.durationSeconds[1] = boundedDuration(
      preferences.getUInt("duration2", DEFAULT_DURATION_2), DEFAULT_DURATION_2);
  preferences.end();
  return settings;
}

void saveSettings(const Settings& settings) {
  preferences.begin("sanitizer", false);
  preferences.putUInt("duration1", boundedDuration(settings.durationSeconds[0], DEFAULT_DURATION_1));
  preferences.putUInt("duration2", boundedDuration(settings.durationSeconds[1], DEFAULT_DURATION_2));
  preferences.end();
}