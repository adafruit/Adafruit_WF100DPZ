/*!
 * hw_test_04_temperature.ino - Temperature Reading Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Read temperature 5 times
 * - All readings between 15.0 and 40.0 °C (room temp sanity)
 * - Readings are stable (max-min < 2.0 °C)
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>

#define NUM_READINGS 5
#define TEMP_MIN 15.0
#define TEMP_MAX 40.0
#define TEMP_STABILITY 2.0

Adafruit_WF100DPZ sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_04_temperature ==="));
  Serial.println(F("Testing: Temperature readings (5 samples)"));

  bool allPass = true;

  // Test 1: begin()
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Read temperature 5 times
  Serial.println(F("\n2. Temperature readings:"));
  float readings[NUM_READINGS];
  float minTemp = 1000.0;
  float maxTemp = -1000.0;
  bool rangeOK = true;

  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = sensor.readTemperature();
    Serial.print(F("   Reading "));
    Serial.print(i + 1);
    Serial.print(F(": "));

    if (isnan(readings[i])) {
      Serial.println(F("NaN - FAIL"));
      allPass = false;
      rangeOK = false;
    } else {
      Serial.print(readings[i]);
      Serial.print(F(" C"));

      if (readings[i] < TEMP_MIN || readings[i] > TEMP_MAX) {
        Serial.println(F(" - OUT OF RANGE"));
        rangeOK = false;
      } else {
        Serial.println(F(" - OK"));
      }

      if (readings[i] < minTemp)
        minTemp = readings[i];
      if (readings[i] > maxTemp)
        maxTemp = readings[i];
    }
    delay(100);
  }

  // Test 3: Range check
  Serial.print(F("\n3. Range check ("));
  Serial.print(TEMP_MIN);
  Serial.print(F("-"));
  Serial.print(TEMP_MAX);
  Serial.print(F(" C): "));
  if (!rangeOK) {
    Serial.println(F("FAIL"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 4: Stability check
  Serial.print(F("4. Stability check (max-min < "));
  Serial.print(TEMP_STABILITY);
  Serial.print(F(" C): "));
  float spread = maxTemp - minTemp;
  Serial.print(F("spread="));
  Serial.print(spread);
  Serial.print(F(" C - "));
  if (spread >= TEMP_STABILITY) {
    Serial.println(F("FAIL"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
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
