#include "display.h"

#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

namespace {
constexpr uint8_t TOUCH_CS_PIN = 33;
constexpr uint8_t TOUCH_IRQ_PIN = 36;
constexpr uint8_t TOUCH_CLK_PIN = 25;
constexpr uint8_t TOUCH_MISO_PIN = 39;
constexpr uint8_t TOUCH_MOSI_PIN = 32;
constexpr int SETTINGS_BUTTON_X = 192;
constexpr int SETTINGS_BUTTON_Y = 275;
constexpr int SETTINGS_BUTTON_W = 40;
constexpr int SETTINGS_BUTTON_H = 34;
constexpr int DONE_BUTTON_X = 192;
constexpr int DONE_BUTTON_Y = 275;
constexpr int DONE_BUTTON_W = 40;
constexpr int DONE_BUTTON_H = 34;
TFT_eSPI tft;
SPIClass touchSpi(VSPI);
XPT2046_Touchscreen touch(TOUCH_CS_PIN, TOUCH_IRQ_PIN);
bool touchLogState = false;

uint16_t timerColor(uint32_t remaining, bool removed) {
  if (remaining == 0) return TFT_RED;
  return removed ? TFT_GREEN : TFT_DARKGREY;
}

void drawGearIcon(int centerX, int centerY, uint16_t color) {
  tft.drawCircle(centerX, centerY, 8, color);
  tft.drawCircle(centerX, centerY, 3, color);
  for (uint8_t tooth = 0; tooth < 8; ++tooth) {
    const float angle = tooth * 0.785398163f;
    const int innerX = centerX + static_cast<int>(cosf(angle) * 9);
    const int innerY = centerY + static_cast<int>(sinf(angle) * 9);
    const int outerX = centerX + static_cast<int>(cosf(angle) * 13);
    const int outerY = centerY + static_cast<int>(sinf(angle) * 13);
    tft.drawLine(innerX, innerY, outerX, outerY, color);
  }
}

void drawCheckIcon(int centerX, int centerY, uint16_t color) {
  tft.drawLine(centerX - 8, centerY, centerX - 2, centerY + 6, color);
  tft.drawLine(centerX - 2, centerY + 6, centerX + 9, centerY - 7, color);
}

void drawDots(int centerX, int y, uint8_t count, uint16_t color) {
  constexpr int radius = 11;
  constexpr int spacing = 35;
  const int startX = centerX - (spacing * 5) / 2;
  tft.fillRect(startX - radius, y - radius, spacing * 5 + radius * 2,
               radius * 2 + 1, TFT_BLACK);
  for (uint8_t dot = 0; dot < 6; ++dot) {
    if (dot < count) tft.fillCircle(startX + dot * spacing, y, radius, color);
  }
}

void drawBottleFrame(uint8_t index) {
  const int top = 25 + index * 120;
  tft.drawRoundRect(8, top, 224, 112, 8, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(19, top + 9);
  tft.printf("BOTTLE %u", index + 1);
}

void refreshBottle(uint8_t index, const BottleTimer& timer) {
  const int top = 25 + index * 120;
  const uint32_t now = millis();
  const uint32_t remaining = timer.remainingSeconds(now);
  const uint8_t dots = timer.visibleDots(now);

  tft.setTextSize(1);
  tft.fillRect(18, top + 31, 205, 22, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(19, top + 34);
  tft.print(timer.isRemoved() ? "AWAY" : (timer.hasStarted() ? "RETURNED" : "READY"));
  tft.setCursor(158, top + 34);
  tft.printf("%lu:%02lu", remaining / 60, remaining % 60);
  drawDots(120, top + 77, dots, timerColor(remaining, timer.isRemoved()));
}
}  // namespace

void displayBegin() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  touchSpi.begin(TOUCH_CLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_CS_PIN);
  touch.begin(touchSpi);
  touch.setRotation(0);
}

bool displayReadTouch(TouchPoint& point) {
  const bool pressed = touch.touched();
  if (!pressed) {
    if (touchLogState) {
      Serial.println("Touch up");
      touchLogState = false;
    }
    return false;
  }

  const TS_Point raw = touch.getPoint();
  point.x = map(raw.x, 200, 3900, 0, 239);
  point.y = map(raw.y, 200, 3900, 0, 319);
  point.x = constrain(point.x, 0, 239);
  point.y = constrain(point.y, 0, 319);
  if (!touchLogState) {
    Serial.printf("Touch down raw=(%d,%d,%d) mapped=(%d,%d)\n", raw.x,
                  raw.y, raw.z, point.x, point.y);
    touchLogState = true;
  }
  return true;
}

bool displayMainSettingsHit(const TouchPoint& point) {
  return point.x >= SETTINGS_BUTTON_X &&
         point.x < SETTINGS_BUTTON_X + SETTINGS_BUTTON_W &&
         point.y >= SETTINGS_BUTTON_Y &&
         point.y < SETTINGS_BUTTON_Y + SETTINGS_BUTTON_H;
}

bool displaySettingsDoneHit(const TouchPoint& point) {
  return point.x >= DONE_BUTTON_X && point.x < DONE_BUTTON_X + DONE_BUTTON_W &&
         point.y >= DONE_BUTTON_Y && point.y < DONE_BUTTON_Y + DONE_BUTTON_H;
}

void displayShowMain(const BottleTimer timers[2], const Settings& settings) {
  tft.fillScreen(TFT_BLACK);
  drawBottleFrame(0);
  drawBottleFrame(1);
  tft.drawRoundRect(SETTINGS_BUTTON_X, SETTINGS_BUTTON_Y, SETTINGS_BUTTON_W,
                    SETTINGS_BUTTON_H, 5, TFT_CYAN);
  drawGearIcon(SETTINGS_BUTTON_X + SETTINGS_BUTTON_W / 2,
               SETTINGS_BUTTON_Y + SETTINGS_BUTTON_H / 2, TFT_CYAN);
  displayRefreshMain(timers);
  (void)settings;
}

void displayRefreshMain(const BottleTimer timers[2]) {
  refreshBottle(0, timers[0]);
  refreshBottle(1, timers[1]);
}

void displayShowSettings(const Settings& settings) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(18, 18);
  tft.print("TIMER SETTINGS");
  for (uint8_t bottle = 0; bottle < 2; ++bottle) {
    const int y = 68 + bottle * 92;
    const uint32_t minutes = settings.durationSeconds[bottle] / 60;
    tft.drawRoundRect(8, y - 10, 224, 70, 8, TFT_DARKGREY);
    tft.setTextSize(2);
    tft.setCursor(18, y);
    tft.printf("BOTTLE %u", bottle + 1);
    tft.setTextSize(3);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setCursor(86, y + 28);
    tft.printf("%lu min", minutes);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setCursor(22, y + 30);
    tft.print("-");
    tft.setCursor(203, y + 30);
    tft.print("+");
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  tft.drawRoundRect(DONE_BUTTON_X, DONE_BUTTON_Y, DONE_BUTTON_W,
                    DONE_BUTTON_H, 5, TFT_CYAN);
  drawCheckIcon(DONE_BUTTON_X + DONE_BUTTON_W / 2,
                DONE_BUTTON_Y + DONE_BUTTON_H / 2, TFT_CYAN);
}