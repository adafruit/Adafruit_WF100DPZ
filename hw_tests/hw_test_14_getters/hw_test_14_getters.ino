/*!
 * hw_test_14_getters.ino - Getter/Setter Roundtrip Test for WF100DPZ
 *
 * Verifies:
 * - setMeasurementMode / getMeasurementMode for all 4 modes
 * - setSleepInterval / getSleepInterval for all 16 values
 *
 * Hardware: Metro Mini, WF100DPZ at 0x6D
 */

#include <Adafruit_WF100DPZ.h>

Adafruit_WF100DPZ sensor;

bool allPass = true;

void testMode(wf100dpz_mode_t mode, const __FlashStringHelper* name) {
  sensor.setMeasurementMode(mode);
  wf100dpz_mode_t got = sensor.getMeasurementMode();
  Serial.print(F("  "));
  Serial.print(name);
  Serial.print(F(": "));
  if (got == mode) {
    Serial.println(F("OK"));
  } else {
    Serial.print(F("FAIL (got "));
    Serial.print(got);
    Serial.println(F(")"));
    allPass = false;
  }
}

void testSleep(wf100dpz_sleep_t interval, const __FlashStringHelper* name) {
  sensor.setSleepInterval(interval);
  wf100dpz_sleep_t got = sensor.getSleepInterval();
  Serial.print(F("  "));
  Serial.print(name);
  Serial.print(F(": "));
  if (got == interval) {
    Serial.println(F("OK"));
  } else {
    Serial.print(F("FAIL (got "));
    Serial.print(got);
    Serial.println(F(")"));
    allPass = false;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("=== hw_test_14_getters ==="));
  Serial.println(F("Testing: Getter/setter roundtrip"));

  if (!sensor.begin()) {
    Serial.println(F("FAIL - begin() returned false"));
    while (1)
      delay(10);
  }

  // Test 1: Measurement modes
  Serial.println(F("\n1. Measurement modes:"));
  testMode(WF100DPZ_MODE_TEMP_ONLY, F("TEMP_ONLY"));
  testMode(WF100DPZ_MODE_PRESSURE_ONLY, F("PRESSURE_ONLY"));
  testMode(WF100DPZ_MODE_COMBINED, F("COMBINED"));
  testMode(WF100DPZ_MODE_SLEEP_PERIODIC, F("SLEEP_PERIODIC"));

  // Test 2: Sleep intervals (all 16)
  Serial.println(F("\n2. Sleep intervals:"));
  testSleep(WF100DPZ_SLEEP_0MS, F("0ms"));
  testSleep(WF100DPZ_SLEEP_62MS, F("62ms"));
  testSleep(WF100DPZ_SLEEP_125MS, F("125ms"));
  testSleep(WF100DPZ_SLEEP_187MS, F("187ms"));
  testSleep(WF100DPZ_SLEEP_250MS, F("250ms"));
  testSleep(WF100DPZ_SLEEP_312MS, F("312ms"));
  testSleep(WF100DPZ_SLEEP_375MS, F("375ms"));
  testSleep(WF100DPZ_SLEEP_437MS, F("437ms"));
  testSleep(WF100DPZ_SLEEP_500MS, F("500ms"));
  testSleep(WF100DPZ_SLEEP_562MS, F("562ms"));
  testSleep(WF100DPZ_SLEEP_625MS, F("625ms"));
  testSleep(WF100DPZ_SLEEP_687MS, F("687ms"));
  testSleep(WF100DPZ_SLEEP_750MS, F("750ms"));
  testSleep(WF100DPZ_SLEEP_812MS, F("812ms"));
  testSleep(WF100DPZ_SLEEP_875MS, F("875ms"));
  testSleep(WF100DPZ_SLEEP_1000MS, F("1000ms"));

  // Test 3: Mode survives interval change
  Serial.println(F("\n3. Mode survives interval change:"));
  sensor.setMeasurementMode(WF100DPZ_MODE_COMBINED);
  sensor.setSleepInterval(WF100DPZ_SLEEP_500MS);
  wf100dpz_mode_t m = sensor.getMeasurementMode();
  Serial.print(F("  Mode after interval change: "));
  if (m == WF100DPZ_MODE_COMBINED) {
    Serial.println(F("OK (still COMBINED)"));
  } else {
    Serial.print(F("FAIL (got "));
    Serial.print(m);
    Serial.println(F(")"));
    allPass = false;
  }

  // Test 4: Interval survives mode change
  Serial.println(F("\n4. Interval survives mode change:"));
  sensor.setSleepInterval(WF100DPZ_SLEEP_750MS);
  sensor.setMeasurementMode(WF100DPZ_MODE_TEMP_ONLY);
  wf100dpz_sleep_t s = sensor.getSleepInterval();
  Serial.print(F("  Interval after mode change: "));
  if (s == WF100DPZ_SLEEP_750MS) {
    Serial.println(F("OK (still 750ms)"));
  } else {
    Serial.print(F("FAIL (got "));
    Serial.print(s);
    Serial.println(F(")"));
    allPass = false;
  }

  // Clean up
  sensor.stopSleepMode();

  Serial.println();
  if (allPass) {
    Serial.println(F("=== PASS ==="));
  } else {
    Serial.println(F("=== FAIL ==="));
  }
}

void loop() {}
