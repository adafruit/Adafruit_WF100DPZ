/*!
 * hw_test_09_conversion_timing.ino - Conversion Timing Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Trigger combined conversion
 * - Measure time until DRDY goes high (in microseconds)
 * - Verify it completes within 15ms
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>

#define WF100DPZ_REG_CMD 0x30
#define WF100DPZ_REG_STATUS 0x02
#define WF100DPZ_CMD_COMBINED 0x0A
#define MAX_CONVERSION_US 15000

Adafruit_WF100DPZ sensor;

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(0x6D);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)0x6D, (uint8_t)1);
  return Wire.read();
}

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(0x6D);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_09_conversion_timing ==="));
  Serial.println(F("Testing: Conversion timing measurement"));

  bool allPass = true;

  // Test 1: begin()
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Measure conversion timing
  Serial.println(F("\n2. Conversion timing measurement:"));

  // Run 5 timing measurements
  unsigned long totalTime = 0;
  unsigned long maxTime = 0;
  unsigned long minTime = 0xFFFFFFFF;
  int validReadings = 0;

  for (int i = 0; i < 5; i++) {
    // Clear any pending conversion by reading status
    readRegister(WF100DPZ_REG_STATUS);
    delay(20); // Ensure clean state

    // Trigger combined conversion
    unsigned long startTime = micros();
    writeRegister(WF100DPZ_REG_CMD, WF100DPZ_CMD_COMBINED);

    // Poll for DRDY
    bool drdy = false;
    unsigned long elapsed = 0;
    while (elapsed < MAX_CONVERSION_US + 5000) {
      uint8_t status = readRegister(WF100DPZ_REG_STATUS);
      elapsed = micros() - startTime;
      if (status & 0x01) {
        drdy = true;
        break;
      }
      delayMicroseconds(50);
    }

    Serial.print(F("   Trial "));
    Serial.print(i + 1);
    Serial.print(F(": "));

    if (!drdy) {
      Serial.println(F("TIMEOUT - FAIL"));
      allPass = false;
    } else {
      Serial.print(elapsed);
      Serial.print(F(" us"));

      if (elapsed > MAX_CONVERSION_US) {
        Serial.println(F(" - FAIL (>15ms)"));
        allPass = false;
      } else {
        Serial.println(F(" - OK"));
        validReadings++;
        totalTime += elapsed;
        if (elapsed > maxTime)
          maxTime = elapsed;
        if (elapsed < minTime)
          minTime = elapsed;
      }
    }
  }

  // Test 3: Statistics
  if (validReadings > 0) {
    Serial.println(F("\n3. Timing statistics:"));
    Serial.print(F("   Min: "));
    Serial.print(minTime);
    Serial.println(F(" us"));
    Serial.print(F("   Max: "));
    Serial.print(maxTime);
    Serial.println(F(" us"));
    Serial.print(F("   Avg: "));
    Serial.print(totalTime / validReadings);
    Serial.println(F(" us"));
  }

  // Test 4: Verify all within limit
  Serial.print(F("\n4. All conversions < 15ms: "));
  if (maxTime > MAX_CONVERSION_US || validReadings != 5) {
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
