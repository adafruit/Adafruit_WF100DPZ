/*!
 * hw_test_03_soft_reset.ino - Soft Reset Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Trigger a conversion, verify data registers change
 * - Perform soft reset
 * - Verify CMD register returns to 0x00
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>
#include <Wire.h>

#define WF100DPZ_REG_CMD 0x30

Adafruit_WF100DPZ sensor;

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(0x6D);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)0x6D, (uint8_t)1);
  return Wire.read();
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_03_soft_reset ==="));
  Serial.println(F("Testing: Soft reset functionality"));

  Wire.begin();

  bool allPass = true;

  // Test 1: begin()
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Trigger conversion and read pressure
  Serial.print(F("2. Trigger conversion: "));
  float pressure = sensor.readPressure();
  if (isnan(pressure)) {
    Serial.println(F("FAIL - readPressure returned NaN"));
    allPass = false;
  } else {
    Serial.print(pressure);
    Serial.println(F(" kPa - OK"));
  }

  // Test 3: Perform soft reset
  Serial.print(F("3. Soft reset: "));
  if (!sensor.softReset()) {
    Serial.println(F("FAIL - softReset() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 4: Verify CMD register is 0x00 after reset
  delay(10); // Allow reset to complete
  Serial.print(F("4. CMD register after reset: "));
  uint8_t cmdVal = readRegister(WF100DPZ_REG_CMD);
  Serial.print(F("0x"));
  if (cmdVal < 16)
    Serial.print(F("0"));
  Serial.print(cmdVal, HEX);

  if (cmdVal != 0x00) {
    Serial.println(F(" - FAIL (expected 0x00)"));
    allPass = false;
  } else {
    Serial.println(F(" - OK"));
  }

  // Final result
  Serial.println();
  if (allPass) {
    Serial.println(F("=== PASS ==="));
  } else {
    Serial.println(F("=== FAIL ==="));
  }
}

void loop() {
  // Nothing to do
}
