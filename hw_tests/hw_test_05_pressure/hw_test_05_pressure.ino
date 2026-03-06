/*!
 * hw_test_05_pressure.ino - Pressure Reading Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Read pressure 5 times
 * - All readings between -10.0 and 20.0 kPa (near-zero gauge at ambient)
 * - Readings are stable (max-min < 5.0 kPa)
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>

#define NUM_READINGS 5
#define PRESSURE_MIN (-10.0)
#define PRESSURE_MAX 20.0
#define PRESSURE_STABILITY 5.0

Adafruit_WF100DPZ sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_05_pressure ==="));
  Serial.println(F("Testing: Pressure readings (5 samples)"));

  bool allPass = true;

  // Test 1: begin()
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Read pressure 5 times
  Serial.println(F("\n2. Pressure readings:"));
  float readings[NUM_READINGS];
  float minPressure = 1000.0;
  float maxPressure = -1000.0;
  bool rangeOK = true;

  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = sensor.readPressure();
    Serial.print(F("   Reading "));
    Serial.print(i + 1);
    Serial.print(F(": "));

    if (isnan(readings[i])) {
      Serial.println(F("NaN - FAIL"));
      allPass = false;
      rangeOK = false;
    } else {
      Serial.print(readings[i]);
      Serial.print(F(" kPa"));

      if (readings[i] < PRESSURE_MIN || readings[i] > PRESSURE_MAX) {
        Serial.println(F(" - OUT OF RANGE"));
        rangeOK = false;
      } else {
        Serial.println(F(" - OK"));
      }

      if (readings[i] < minPressure)
        minPressure = readings[i];
      if (readings[i] > maxPressure)
        maxPressure = readings[i];
    }
    delay(100);
  }

  // Test 3: Range check
  Serial.print(F("\n3. Range check ("));
  Serial.print(PRESSURE_MIN);
  Serial.print(F(" to "));
  Serial.print(PRESSURE_MAX);
  Serial.print(F(" kPa): "));
  if (!rangeOK) {
    Serial.println(F("FAIL"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 4: Stability check
  Serial.print(F("4. Stability check (max-min < "));
  Serial.print(PRESSURE_STABILITY);
  Serial.print(F(" kPa): "));
  float spread = maxPressure - minPressure;
  Serial.print(F("spread="));
  Serial.print(spread);
  Serial.print(F(" kPa - "));
  if (spread >= PRESSURE_STABILITY) {
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
