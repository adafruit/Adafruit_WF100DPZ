/*!
 * hw_test_01_i2c_detect.ino - I2C Detect Test for WF100DPZ
 *
 * Verifies:
 * - Device responds at I2C address 0x6D
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Wire.h>

#define SENSOR_ADDR 0x6D

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_01_i2c_detect ==="));
  Serial.println(F("Testing: I2C bus scan for WF100DPZ at 0x6D"));

  Wire.begin();

  bool found = false;

  Serial.print(F("Scanning I2C bus for 0x6D... "));

  Wire.beginTransmission(SENSOR_ADDR);
  uint8_t error = Wire.endTransmission();

  if (error == 0) {
    Serial.println(F("FOUND"));
    found = true;
  } else {
    Serial.print(F("NOT FOUND (error="));
    Serial.print(error);
    Serial.println(F(")"));
  }

  // Also show all devices on bus
  Serial.println(F("\nFull I2C bus scan:"));
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  0x"));
      if (addr < 16)
        Serial.print(F("0"));
      Serial.println(addr, HEX);
    }
  }

  Serial.println();
  if (found) {
    Serial.println(F("=== PASS ==="));
  } else {
    Serial.println(F("=== FAIL ==="));
  }
}

void loop() {
  // Nothing to do
}
