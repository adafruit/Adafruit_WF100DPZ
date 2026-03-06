/*!
 * hw_test_06_combined.ino - Combined Reading Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Use readTempPressure() to get both values
 * - Temperature in range 15-40 °C
 * - Pressure in range -10 to 20 kPa
 * - Repeat 5 times
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>

#define NUM_READINGS 5
#define TEMP_MIN 15.0
#define TEMP_MAX 40.0
#define PRESSURE_MIN (-10.0)
#define PRESSURE_MAX 20.0

Adafruit_WF100DPZ sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_06_combined ==="));
  Serial.println(F("Testing: Combined readTempPressure() (5 samples)"));

  bool allPass = true;

  // Test 1: begin()
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Read combined 5 times
  Serial.println(F("\n2. Combined readings:"));

  for (int i = 0; i < NUM_READINGS; i++) {
    float pressure, temperature;
    bool readOK = sensor.readTempPressure(&pressure, &temperature);

    Serial.print(F("   Reading "));
    Serial.print(i + 1);
    Serial.print(F(": "));

    if (!readOK) {
      Serial.println(F("readTempPressure() failed - FAIL"));
      allPass = false;
    } else {
      Serial.print(F("T="));
      Serial.print(temperature);
      Serial.print(F(" C, P="));
      Serial.print(pressure);
      Serial.print(F(" kPa"));

      bool tempOK = (temperature >= TEMP_MIN && temperature <= TEMP_MAX);
      bool pressOK = (pressure >= PRESSURE_MIN && pressure <= PRESSURE_MAX);

      if (!tempOK || !pressOK) {
        Serial.print(F(" - "));
        if (!tempOK)
          Serial.print(F("TEMP OUT "));
        if (!pressOK)
          Serial.print(F("PRESS OUT"));
        Serial.println();
        allPass = false;
      } else {
        Serial.println(F(" - OK"));
      }
    }
    delay(100);
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
