/*
 * @file wf100dpz_tft_demo.ino
 *
 * TFT demo for WF100DPZ Gauge Pressure Sensor on Feather ESP32-S2 TFT
 * Displays pressure as a large color-coded readout with temperature.
 * Uses GFXcanvas16 to eliminate flicker.
 *
 * Hardware:
 *  - Adafruit Feather ESP32-S2 TFT
 *  - WF100DPZ Gauge Pressure Sensor breakout (I2C)
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code
 *
 * MIT license, all text above must be included in any redistribution.
 */

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_WF100DPZ.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Wire.h>
#include <math.h>

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_WF100DPZ wf100dpz;
float pressureOffset = 0.0f;

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

#define COLOR_TITLE 0xFFFF
#define COLOR_BAR_BG 0x2104

// Canvas for the pressure area (below title)
#define CONTENT_Y 28
#define CONTENT_W 240
#define CONTENT_H 107

#define PRESSURE_MIN -50.0f
#define PRESSURE_MAX 50.0f

GFXcanvas16 contentCanvas(CONTENT_W, CONTENT_H);

uint16_t getPressureColor(float pressure) {
  if (pressure < -50.0f)
    return 0x3BFF; // deep blue
  else if (pressure < -10.0f)
    return 0x07FF; // cyan
  else if (pressure < 10.0f)
    return 0x07E0; // green
  else if (pressure < 50.0f)
    return 0xFFE0; // yellow
  return 0xFD20;   // orange
}

void formatPressure(float pressure, char* buffer, size_t len) {
  char numberBuf[12];
  dtostrf(fabsf(pressure), 0, 1, numberBuf);
  if (pressure < 0.0f) {
    snprintf(buffer, len, "-%s", numberBuf);
  } else if (pressure > 10.0f) {
    snprintf(buffer, len, "+%s", numberBuf);
  } else {
    snprintf(buffer, len, "%s", numberBuf);
  }
  buffer[len - 1] = 0;
}

float clampPressure(float pressure) {
  if (pressure < PRESSURE_MIN)
    return PRESSURE_MIN;
  if (pressure > PRESSURE_MAX)
    return PRESSURE_MAX;
  return pressure;
}

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000))
    delay(10);

  Serial.println(F("WF100DPZ Pressure Sensor TFT Demo"));

  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  tft.init(135, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  const char title[] = "Adafruit WF100DPZ";
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(COLOR_TITLE);
  int16_t x1, y1;
  uint16_t tw, th;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &tw, &th);
  tft.setCursor((SCREEN_WIDTH - tw) / 2, 22);
  tft.print(title);

  Wire.begin();

  if (!wf100dpz.begin()) {
    Serial.println(F("Failed to find WF100DPZ sensor!"));
    tft.setFont(&FreeSansBold24pt7b);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(20, 90);
    tft.print(F("No Sensor!"));
    while (1)
      delay(100);
  }

  Serial.println(F("WF100DPZ found!"));
  Serial.print(F("Detected "));
  Serial.print(wf100dpz.getMaxPressure(), 0);
  Serial.println(F(" kPa variant"));

  wf100dpz.setSleepMode(WF100DPZ_SLEEP_0MS);

  // Auto-zero: average a few readings at startup to capture ambient offset
  delay(50);
  float sum = 0.0f;
  const uint8_t samples = 8;
  for (uint8_t i = 0; i < samples; i++) {
    wf100dpz.triggerConversion();
    wf100dpz.waitDRDY();
    sum += wf100dpz.readPressure();
    delay(10);
  }
  pressureOffset = sum / (float)samples;
  Serial.print(F("Auto-zero offset: "));
  Serial.print(pressureOffset, 2);
  Serial.println(F(" kPa"));

  // Re-enable periodic mode after manual trigger calls
  wf100dpz.setSleepMode(WF100DPZ_SLEEP_0MS);
}

void loop() {
  float pressure = 0.0f;
  float temperature = 0.0f;
  if (!wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.println(F("Failed to read WF100DPZ!"));
    contentCanvas.fillScreen(ST77XX_BLACK);
    contentCanvas.setFont(&FreeSansBold18pt7b);
    contentCanvas.setTextColor(ST77XX_RED);
    contentCanvas.setCursor(24, 70);
    contentCanvas.print(F("Read Error"));
    tft.drawRGBBitmap(0, CONTENT_Y, contentCanvas.getBuffer(), CONTENT_W,
                      CONTENT_H);
    delay(500);
    return;
  }

  pressure -= pressureOffset;

  Serial.print(F("Pressure: "));
  Serial.print(pressure, 4);
  Serial.print(F(" kPa, Temp: "));
  Serial.print(temperature, 4);
  Serial.println(F(" C"));

  contentCanvas.fillScreen(ST77XX_BLACK);

  uint16_t color = getPressureColor(pressure);

  char pressureBuf[16];
  formatPressure(pressure, pressureBuf, sizeof(pressureBuf));

  contentCanvas.setFont(&FreeSansBold24pt7b);
  contentCanvas.setTextSize(1);
  contentCanvas.setTextColor(color);

  int16_t x1, y1;
  uint16_t numW, numH;
  contentCanvas.getTextBounds(pressureBuf, 0, 0, &x1, &y1, &numW, &numH);

  contentCanvas.setFont(&FreeSansBold18pt7b);
  int16_t ux1, uy1;
  uint16_t unitW, unitH;
  contentCanvas.getTextBounds(" kPa", 0, 0, &ux1, &uy1, &unitW, &unitH);

  uint16_t gap = 4;
  uint16_t totalW = numW + gap + unitW;
  int16_t tx = (CONTENT_W - totalW) / 2;
  int16_t ty = (CONTENT_H / 2) + (numH / 2) - 24;

  contentCanvas.setFont(&FreeSansBold24pt7b);
  contentCanvas.setCursor(tx, ty);
  contentCanvas.print(pressureBuf);

  contentCanvas.setFont(&FreeSansBold18pt7b);
  contentCanvas.setCursor(contentCanvas.getCursorX() + gap, ty);
  contentCanvas.print(F(" kPa"));

  // Temperature line
  char tempBuf[12];
  dtostrf(temperature, 0, 1, tempBuf);

  contentCanvas.setFont(&FreeSansBold12pt7b);
  contentCanvas.setTextColor(0xBDF7); // dim gray

  int16_t tx1, ty1;
  uint16_t tempW, tempH;
  contentCanvas.getTextBounds(tempBuf, 0, 0, &tx1, &ty1, &tempW, &tempH);

  int16_t cx1, cy1;
  uint16_t cW, cH;
  contentCanvas.getTextBounds("C", 0, 0, &cx1, &cy1, &cW, &cH);

  uint16_t degGap = 3;
  uint16_t degR = 3;
  uint16_t tempTotalW = tempW + degGap + degR * 2 + 2 + cW;
  int16_t tempX = (CONTENT_W - tempTotalW) / 2;
  int16_t tempY = ty + tempH + 10;

  contentCanvas.setCursor(tempX, tempY);
  contentCanvas.print(tempBuf);

  int16_t degX = contentCanvas.getCursorX() + degGap + degR;
  int16_t degY2 = tempY - tempH + degR + 2;
  contentCanvas.drawCircle(degX, degY2, degR, 0xBDF7);
  contentCanvas.drawCircle(degX, degY2, degR - 1, 0xBDF7);

  contentCanvas.setCursor(degX + degR + 2, tempY);
  contentCanvas.print(F("C"));

  float clamped = clampPressure(pressure);
  const uint8_t barHeight = 14;
  const uint8_t barMargin = 4;
  int16_t barY = CONTENT_H - barHeight - barMargin;
  contentCanvas.fillRect(0, barY, CONTENT_W, barHeight, COLOR_BAR_BG);

  int16_t centerX = CONTENT_W / 2;
  if (clamped < 0.0f) {
    float ratio = fabsf(clamped) / fabsf(PRESSURE_MIN);
    if (ratio > 1.0f)
      ratio = 1.0f;
    int16_t fillW = (int16_t)(ratio * (float)centerX + 0.5f);
    contentCanvas.fillRect(centerX - fillW, barY, fillW, barHeight, color);
  } else if (clamped > 0.0f) {
    float ratio = clamped / PRESSURE_MAX;
    if (ratio > 1.0f)
      ratio = 1.0f;
    int16_t fillW = (int16_t)(ratio * (float)centerX + 0.5f);
    contentCanvas.fillRect(centerX, barY, fillW, barHeight, color);
  }

  tft.drawRGBBitmap(0, CONTENT_Y, contentCanvas.getBuffer(), CONTENT_W,
                    CONTENT_H);

  delay(100);
}
