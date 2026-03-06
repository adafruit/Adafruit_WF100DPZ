/*!
 * hw_test_02_part_id.ino - Part ID Test for WF100DPZ
 *
 * Verifies:
 * - begin() succeeds at 0x6D
 * - Part ID equals 0x49
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>

Adafruit_WF100DPZ sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_02_part_id ==="));
  Serial.println(F("Testing: Library begin() and Part ID verification"));

  bool allPass = true;

  // Test 1: begin() succeeds
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Part ID
  Serial.print(F("2. Part ID: "));
  uint8_t partId = sensor.getPartID();
  Serial.print(F("0x"));
  if (partId < 16)
    Serial.print(F("0"));
  Serial.print(partId, HEX);

  if (partId != 0x49) {
    Serial.println(F(" - FAIL (expected 0x49)"));
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
