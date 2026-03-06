/*!
 * hw_test_07_status.ino - Status Register Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Read status register
 * - No error flags set (bits 7:4 should be 0)
 * - hasError() returns false
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>
#include <Wire.h>

Adafruit_WF100DPZ sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_07_status ==="));
  Serial.println(F("Testing: Status register and error flags"));

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

  // Trigger a conversion first so status register has valid data
  sensor.readPressure();

  // Test 2: Read status register
  Serial.print(F("2. Status register: "));
  uint8_t status = sensor.getStatus();
  Serial.print(F("0x"));
  if (status < 16)
    Serial.print(F("0"));
  Serial.println(status, HEX);

  // Test 3: Error flags (bits 7:4)
  Serial.print(F("3. Error flags (bits 7:4): "));
  uint8_t errorFlags = (status >> 4) & 0x0F;
  Serial.print(F("0x"));
  Serial.print(errorFlags, HEX);

  if (errorFlags != 0) {
    Serial.println(F(" - FAIL (expected 0)"));
    if (errorFlags & 0x08)
      Serial.println(F("   - VINP short to VDD"));
    if (errorFlags & 0x04)
      Serial.println(F("   - VINP short to GND"));
    if (errorFlags & 0x02)
      Serial.println(F("   - VINN short to VDD"));
    if (errorFlags & 0x01)
      Serial.println(F("   - VINN short to GND"));
    allPass = false;
  } else {
    Serial.println(F(" - OK (no errors)"));
  }

  // Test 4: hasError() function
  Serial.print(F("4. hasError(): "));
  bool hasErr = sensor.hasError();
  Serial.print(hasErr ? F("true") : F("false"));

  if (hasErr) {
    Serial.println(F(" - FAIL (expected false)"));
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
