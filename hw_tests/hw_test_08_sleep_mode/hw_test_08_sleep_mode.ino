/*!
 * hw_test_08_sleep_mode.ino - Sleep Mode Test for WF100DPZ
 *
 * Verifies:
 * - Initialize sensor
 * - Start sleep mode with interval 2 (~125ms)
 * - Wait 300ms
 * - Check that DRDY has been set (periodic conversion occurred)
 * - Read pressure, verify it's in valid range
 * - Stop sleep mode
 * - Verify CMD register is 0x00
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>

#define WF100DPZ_REG_CMD 0x30
#define WF100DPZ_REG_STATUS 0x02
#define PRESSURE_MIN (-10.0)
#define PRESSURE_MAX 20.0

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

  Serial.println(F("=== hw_test_08_sleep_mode ==="));
  Serial.println(F("Testing: Sleep mode periodic conversions"));

  bool allPass = true;

  // Test 1: begin()
  Serial.print(F("1. begin() at 0x6D: "));
  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 2: Start sleep mode with interval 2 (~125ms)
  Serial.print(F("2. setSleepMode(2): "));
  if (!sensor.setSleepMode(2)) {
    Serial.println(F("FAIL - setSleepMode() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 3: Wait and check DRDY
  Serial.print(F("3. Wait 300ms, check DRDY: "));
  delay(300);
  uint8_t status = readRegister(WF100DPZ_REG_STATUS);
  bool drdy = (status & 0x01) != 0;
  Serial.print(F("status=0x"));
  if (status < 16)
    Serial.print(F("0"));
  Serial.print(status, HEX);
  Serial.print(F(", DRDY="));
  Serial.print(drdy ? F("1") : F("0"));

  if (!drdy) {
    Serial.println(F(" - FAIL (expected DRDY=1)"));
    allPass = false;
  } else {
    Serial.println(F(" - OK"));
  }

  // Test 4: Read pressure in sleep mode
  Serial.print(F("4. Read pressure: "));
  float pressure = sensor.readPressure();

  if (isnan(pressure)) {
    Serial.println(F("NaN - FAIL"));
    allPass = false;
  } else {
    Serial.print(pressure);
    Serial.print(F(" kPa"));

    if (pressure < PRESSURE_MIN || pressure > PRESSURE_MAX) {
      Serial.println(F(" - FAIL (out of range)"));
      allPass = false;
    } else {
      Serial.println(F(" - OK"));
    }
  }

  // Test 5: Stop sleep mode
  Serial.print(F("5. stopSleepMode(): "));
  if (!sensor.stopSleepMode()) {
    Serial.println(F("FAIL - stopSleepMode() returned false"));
    allPass = false;
  } else {
    Serial.println(F("OK"));
  }

  // Test 6: Verify CMD register is 0x00
  Serial.print(F("6. CMD register: "));
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
