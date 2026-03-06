/*!
 * @file hw_test_11_spi3.ino
 *
 * Hardware test: 3-wire SPI communication with WF100DPZ
 *
 * Tests:
 * 1. Initialize sensor over 3-wire SPI (verifies Part_ID)
 * 2. Read temperature (sanity check 15-40°C)
 * 3. Read pressure (sanity check -10 to 20 kPa gauge)
 *
 * Wiring:
 * - CS   = Pin 10
 * - CLK  = A5 (shared with I2C SCL)
 * - DATA = A4 (shared with I2C SDA)
 *
 * The sensor's SDA/SCL pins serve double duty for I2C and SPI.
 * Only CS is a new connection for SPI mode.
 */

#include <Adafruit_WF100DPZ.h>

// 3-wire SPI pins (sensor shares SDA/SCL for both modes)
#define SPI3_CS 10
#define SPI3_CLK A5  // Same as SCL
#define SPI3_DATA A4 // Same as SDA

Adafruit_WF100DPZ wf100dpz;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("=== HW Test 11: 3-Wire SPI ==="));
  Serial.println();

  // Test 1: Initialize over 3-wire SPI
  Serial.println(F("Test 1: SPI initialization..."));
  if (!wf100dpz.beginSPI(SPI3_CS, SPI3_CLK, SPI3_DATA)) {
    Serial.println(F("  FAIL: beginSPI() returned false"));
    Serial.println(F("  (Part_ID mismatch or communication error)"));
    Serial.println();
    Serial.println(F("=== RESULT: FAIL ==="));
    while (1) {
      delay(1000);
    }
  }
  Serial.println(F("  PASS: Sensor initialized, Part_ID = 0x49"));
  Serial.println();

  // Test 2: Read Part_ID explicitly
  Serial.println(F("Test 2: Read Part_ID..."));
  uint8_t partId = wf100dpz.getPartID();
  Serial.print(F("  Part_ID = 0x"));
  Serial.println(partId, HEX);
  if (partId != 0x49) {
    Serial.println(F("  FAIL: Expected 0x49"));
    Serial.println();
    Serial.println(F("=== RESULT: FAIL ==="));
    while (1) {
      delay(1000);
    }
  }
  Serial.println(F("  PASS"));
  Serial.println();

  // Test 3: Read temperature
  Serial.println(F("Test 3: Read temperature..."));
  float temp = wf100dpz.readTemperature();
  Serial.print(F("  Temperature = "));
  Serial.print(temp);
  Serial.println(F(" C"));

  if (isnan(temp)) {
    Serial.println(F("  FAIL: Temperature read returned NaN"));
    Serial.println();
    Serial.println(F("=== RESULT: FAIL ==="));
    while (1) {
      delay(1000);
    }
  }

  if (temp < 15.0 || temp > 40.0) {
    Serial.println(
        F("  WARN: Temperature outside expected room range (15-40C)"));
  } else {
    Serial.println(F("  PASS: Temperature in expected range"));
  }
  Serial.println();

  // Test 4: Read pressure
  Serial.println(F("Test 4: Read pressure..."));
  float pressure = wf100dpz.readPressure();
  Serial.print(F("  Pressure = "));
  Serial.print(pressure);
  Serial.println(F(" kPa"));

  if (isnan(pressure)) {
    Serial.println(F("  FAIL: Pressure read returned NaN"));
    Serial.println();
    Serial.println(F("=== RESULT: FAIL ==="));
    while (1) {
      delay(1000);
    }
  }

  if (pressure < -10.0 || pressure > 20.0) {
    Serial.println(
        F("  WARN: Pressure outside expected ambient range (-10 to 20 kPa)"));
  } else {
    Serial.println(F("  PASS: Pressure in expected range"));
  }
  Serial.println();

  // Test 5: Combined read
  Serial.println(F("Test 5: Combined temp+pressure read..."));
  float p, t;
  if (!wf100dpz.readTempPressure(&p, &t)) {
    Serial.println(F("  FAIL: readTempPressure() returned false"));
    Serial.println();
    Serial.println(F("=== RESULT: FAIL ==="));
    while (1) {
      delay(1000);
    }
  }
  Serial.print(F("  Temperature = "));
  Serial.print(t);
  Serial.println(F(" C"));
  Serial.print(F("  Pressure = "));
  Serial.print(p);
  Serial.println(F(" kPa"));
  Serial.println(F("  PASS"));
  Serial.println();

  // All tests passed
  Serial.println(F("=== RESULT: PASS ==="));
}

void loop() {
  // Nothing to do
  delay(1000);
}
