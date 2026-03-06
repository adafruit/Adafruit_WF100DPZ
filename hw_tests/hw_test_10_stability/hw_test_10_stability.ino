/*!
 * hw_test_10_stability.ino - Pressure Stability Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Take 20 consecutive pressure readings
 * - Calculate mean, min, max, and standard deviation
 * - Verify std dev < 0.5 kPa (reasonable noise floor)
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>
#include <math.h>

#define NUM_READINGS 20
#define MAX_STD_DEV 0.5

Adafruit_WF100DPZ sensor;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_10_stability ==="));
  Serial.println(F("Testing: Pressure reading stability (20 samples)"));

  bool allPass = true;

  // Test 1: begin()
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Collect readings
  Serial.println(F("\n2. Collecting pressure readings:"));
  float readings[NUM_READINGS];
  float sum = 0;
  float minVal = 1000.0;
  float maxVal = -1000.0;
  int validCount = 0;

  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = sensor.readPressure();

    if (!isnan(readings[i])) {
      sum += readings[i];
      if (readings[i] < minVal)
        minVal = readings[i];
      if (readings[i] > maxVal)
        maxVal = readings[i];
      validCount++;

      Serial.print(F("   ["));
      if (i < 10)
        Serial.print(F(" "));
      Serial.print(i);
      Serial.print(F("] "));
      Serial.print(readings[i], 3);
      Serial.println(F(" kPa"));
    } else {
      Serial.print(F("   ["));
      if (i < 10)
        Serial.print(F(" "));
      Serial.print(i);
      Serial.println(F("] NaN"));
      allPass = false;
    }

    delay(50);
  }

  // Test 3: Calculate statistics
  Serial.println(F("\n3. Statistics:"));

  if (validCount < NUM_READINGS) {
    Serial.print(F("   Valid readings: "));
    Serial.print(validCount);
    Serial.print(F("/"));
    Serial.println(NUM_READINGS);
    allPass = false;
  }

  if (validCount > 0) {
    float mean = sum / validCount;

    // Calculate standard deviation
    float sumSqDiff = 0;
    for (int i = 0; i < NUM_READINGS; i++) {
      if (!isnan(readings[i])) {
        float diff = readings[i] - mean;
        sumSqDiff += diff * diff;
      }
    }
    float variance = sumSqDiff / validCount;
    float stdDev = sqrt(variance);

    Serial.print(F("   Mean: "));
    Serial.print(mean, 3);
    Serial.println(F(" kPa"));
    Serial.print(F("   Min:  "));
    Serial.print(minVal, 3);
    Serial.println(F(" kPa"));
    Serial.print(F("   Max:  "));
    Serial.print(maxVal, 3);
    Serial.println(F(" kPa"));
    Serial.print(F("   Range: "));
    Serial.print(maxVal - minVal, 3);
    Serial.println(F(" kPa"));
    Serial.print(F("   Std Dev: "));
    Serial.print(stdDev, 4);
    Serial.println(F(" kPa"));

    // Test 4: Verify std dev
    Serial.print(F("\n4. Std dev < "));
    Serial.print(MAX_STD_DEV);
    Serial.print(F(" kPa: "));
    if (stdDev >= MAX_STD_DEV) {
      Serial.println(F("FAIL"));
      allPass = false;
    } else {
      Serial.println(F("OK"));
    }
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
