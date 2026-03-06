/*!
 * hw_test_12_cs_affects_i2c.ino - CS Pin Affects I2C Test for WF100DPZ
 *
 * Verifies:
 * - I2C works normally with CS floating (INPUT)
 * - I2C works normally with CS driven HIGH
 * - I2C FAILS with CS driven LOW (sensor switches to SPI mode)
 * - I2C recovers after CS released HIGH
 *
 * This confirms the CS pin is physically connected to the sensor's CSB
 * and that the sensor respects CSB for interface mode selection.
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D, CS wired to pin 10
 */

#include <Adafruit_WF100DPZ.h>
#include <Wire.h>

#define CS_PIN 10
#define WF_ADDR 0x6D
#define EXPECTED_PART_ID 0x49

Adafruit_WF100DPZ sensor;

bool i2cReadPartID(uint8_t* id) {
  Wire.beginTransmission(WF_ADDR);
  Wire.write((uint8_t)0x01);
  uint8_t err = Wire.endTransmission(false);
  if (err != 0)
    return false;
  Wire.requestFrom((uint8_t)WF_ADDR, (uint8_t)1);
  if (Wire.available()) {
    *id = Wire.read();
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_12_cs_affects_i2c ==="));
  Serial.println(F("Testing: CS pin controls I2C/SPI mode selection"));

  bool allPass = true;
  uint8_t id;

  // Test 1: I2C with CS floating
  Serial.print(F("1. I2C with CS floating (INPUT): "));
  pinMode(CS_PIN, INPUT);
  Wire.begin();
  delay(10);
  if (i2cReadPartID(&id) && id == EXPECTED_PART_ID) {
    Serial.println(F("OK (0x49)"));
  } else {
    Serial.println(F("FAIL"));
    allPass = false;
  }

  // Test 2: I2C with CS HIGH
  Serial.print(F("2. I2C with CS driven HIGH: "));
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  delay(10);
  if (i2cReadPartID(&id) && id == EXPECTED_PART_ID) {
    Serial.println(F("OK (0x49)"));
  } else {
    Serial.println(F("FAIL"));
    allPass = false;
  }

  // Test 3: I2C should FAIL with CS LOW
  Serial.print(F("3. I2C with CS driven LOW: "));
  digitalWrite(CS_PIN, LOW);
  delay(10);
  if (!i2cReadPartID(&id)) {
    Serial.println(F("OK (I2C failed as expected - SPI mode)"));
  } else {
    Serial.print(F("FAIL (I2C still works, got 0x"));
    Serial.print(id, HEX);
    Serial.println(F(" - CS not reaching sensor?)"));
    allPass = false;
  }

  // Test 4: I2C recovers after CS HIGH
  Serial.print(F("4. I2C recovery after CS HIGH: "));
  digitalWrite(CS_PIN, HIGH);
  delay(10);
  if (i2cReadPartID(&id) && id == EXPECTED_PART_ID) {
    Serial.println(F("OK (0x49)"));
  } else {
    Serial.println(F("FAIL (sensor stuck in SPI mode?)"));
    allPass = false;
  }

  // Test 5: Library begin() still works
  Serial.print(F("5. Library begin() after CS test: "));
  pinMode(CS_PIN, INPUT); // release CS
  delay(10);
  if (sensor.begin()) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("FAIL"));
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
