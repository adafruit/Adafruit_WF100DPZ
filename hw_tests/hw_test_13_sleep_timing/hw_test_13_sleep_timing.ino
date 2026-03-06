/*!
 * hw_test_13_sleep_timing.ino - Sleep Mode Timing Test for WF100DPZ
 *
 * Verifies:
 * - Sleep periodic mode produces conversions at the expected interval
 * - Tests 1000ms and 125ms intervals
 * - Measures actual time between DRDY events
 * - Verifies stopSleepMode halts periodic conversions
 *
 * Note: DRDY clears when data registers are read, NOT on status read.
 * So we must read data to clear DRDY, then time until next DRDY.
 * We read data via raw I2C to avoid triggering a new conversion
 * (readTempPressure() triggers a single-shot which conflicts).
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>
#include <Wire.h>

#define WF_ADDR 0x6D

Adafruit_WF100DPZ sensor;

// Read data registers via raw I2C to clear DRDY without triggering
void clearDRDY() {
  uint8_t buf[5];
  Wire.beginTransmission(WF_ADDR);
  Wire.write((uint8_t)WF100DPZ_REG_DATA_MSB);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)WF_ADDR, (uint8_t)5); // pressure(3) + temp(2)
  for (uint8_t i = 0; i < 5; i++) {
    buf[i] = Wire.read();
  }
}

// Wait for DRDY, return true if found within timeout
bool waitDRDY(uint16_t timeout_ms) {
  uint32_t start = millis();
  while ((millis() - start) < timeout_ms) {
    if (sensor.getStatus() & WF100DPZ_STATUS_DRDY) {
      return true;
    }
    delayMicroseconds(500);
  }
  return false;
}

// Measure average interval between DRDY events
float measureInterval(uint8_t numSamples, uint16_t timeout_ms) {
  uint32_t total_ms = 0;

  for (uint8_t i = 0; i < numSamples; i++) {
    // Wait for DRDY then clear it by reading data
    if (!waitDRDY(timeout_ms)) {
      Serial.print(F("  Timeout waiting for DRDY #"));
      Serial.println(i);
      return -1;
    }
    clearDRDY();

    // Time until next DRDY
    uint32_t start = millis();
    if (!waitDRDY(timeout_ms)) {
      Serial.print(F("  Timeout on interval #"));
      Serial.println(i);
      return -1;
    }
    total_ms += (millis() - start);
    clearDRDY();
  }

  return (float)total_ms / (float)numSamples;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_13_sleep_timing ==="));
  Serial.println(F("Testing: Sleep mode periodic conversion timing"));

  Wire.begin();

  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    while (1)
      delay(10);
  }

  bool allPass = true;

  // Test 1: 1000ms interval
  Serial.println(F("\n1. Sleep interval 1000ms:"));
  sensor.setSleepMode(WF100DPZ_SLEEP_1000MS);

  float avg = measureInterval(3, 3000);
  sensor.stopSleepMode();

  Serial.print(F("   Average interval: "));
  Serial.print(avg, 0);
  Serial.print(F(" ms"));

  if (avg > 0 && avg >= 500 && avg <= 1300) {
    Serial.println(F(" - OK (500-1300ms)"));
  } else {
    Serial.println(F(" - FAIL"));
    allPass = false;
  }

  // Test 2: 125ms interval
  Serial.println(F("\n2. Sleep interval 125ms:"));
  sensor.setSleepMode(WF100DPZ_SLEEP_125MS);

  avg = measureInterval(5, 1000);
  sensor.stopSleepMode();

  Serial.print(F("   Average interval: "));
  Serial.print(avg, 0);
  Serial.print(F(" ms"));

  if (avg > 0 && avg >= 80 && avg <= 200) {
    Serial.println(F(" - OK (80-200ms)"));
  } else {
    Serial.println(F(" - FAIL"));
    allPass = false;
  }

  // Test 3: Verify stopSleepMode stops conversions
  Serial.println(F("\n3. Stop sleep mode:"));
  sensor.setSleepMode(WF100DPZ_SLEEP_125MS);
  delay(200);
  // Clear any pending DRDY
  if (sensor.getStatus() & WF100DPZ_STATUS_DRDY) {
    clearDRDY();
  }

  sensor.stopSleepMode();
  delay(500); // wait well beyond one interval

  uint8_t status = sensor.getStatus();
  Serial.print(F("   DRDY after stop + 500ms: "));
  if (!(status & WF100DPZ_STATUS_DRDY)) {
    Serial.println(F("no - OK"));
  } else {
    Serial.println(F("yes - FAIL (still converting)"));
    allPass = false;
  }

  // Final result
  Serial.println();
  if (allPass) {
    Serial.println(F("=== PASS ==="));
  } else {
    Serial.println(F("=== FAIL ==="));
  }
}

void loop() {}
