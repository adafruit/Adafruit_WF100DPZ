/*!
 * @file fulltest.ino
 *
 * Full example for the WF100DPZ pressure sensor.
 * Displays sensor info, shows every measurement mode
 * and sleep interval setting, then loops with readings.
 */

#include <Adafruit_WF100DPZ.h>
#include <Wire.h>

Adafruit_WF100DPZ wf100dpz;

void printStatus(uint8_t status);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("WF100DPZ Full Test"));
  Serial.println(F("=================="));

  if (!wf100dpz.begin()) {
    Serial.println(F("Failed to find WF100DPZ sensor!"));
    while (1) {
      delay(10);
    }
  }

  Serial.print(F("Part ID: 0x"));
  Serial.println(wf100dpz.getPartID(), HEX);

  // --- Status register flags ---
  uint8_t status = wf100dpz.getStatus();
  printStatus(status);

  // --- Measurement modes ---
  Serial.println(F("\n--- Measurement Modes ---"));

  // Temperature only
  Serial.println(F("WF100DPZ_MODE_TEMP_ONLY:"));
  float temp = wf100dpz.readTemperature();
  Serial.print(F("  Temperature: "));
  Serial.print(temp, 2);
  Serial.println(F(" C"));

  // Pressure only
  Serial.println(F("WF100DPZ_MODE_PRESSURE_ONLY:"));
  float pres = wf100dpz.readPressure();
  Serial.print(F("  Pressure: "));
  Serial.print(pres, 2);
  Serial.println(F(" kPa"));

  // Combined (recommended)
  Serial.println(F("WF100DPZ_MODE_COMBINED:"));
  float pressure, temperature;
  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("  Temperature: "));
    Serial.print(temperature, 2);
    Serial.print(F(" C, Pressure: "));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  }

  // --- Sleep mode intervals ---
  Serial.println(F("\n--- Sleep Mode Intervals ---"));
  Serial.println(F("Code  Interval"));
  Serial.println(F("0     0 ms"));
  Serial.println(F("1     62.5 ms"));
  Serial.println(F("2     125 ms"));
  Serial.println(F("3     187.5 ms"));
  Serial.println(F("4     250 ms"));
  Serial.println(F("5     312.5 ms"));
  Serial.println(F("6     375 ms"));
  Serial.println(F("7     437.5 ms"));
  Serial.println(F("8     500 ms"));
  Serial.println(F("9     562.5 ms"));
  Serial.println(F("10    625 ms"));
  Serial.println(F("11    687.5 ms"));
  Serial.println(F("12    750 ms"));
  Serial.println(F("13    812.5 ms"));
  Serial.println(F("14    875 ms"));
  Serial.println(F("15    1000 ms"));

  // Demo periodic mode at a few intervals
  uint8_t test_intervals[] = {1, 4, 8, 15};
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t iv = test_intervals[i];
    Serial.print(F("\nWF100DPZ_MODE_SLEEP_PERIODIC interval="));
    Serial.print(iv);
    Serial.println(F(":"));
    wf100dpz.setSleepMode(iv);
    delay(iv * 63 + 100); // wait for at least one conversion
    if (wf100dpz.readTempPressure(&pressure, &temperature)) {
      Serial.print(F("  T: "));
      Serial.print(temperature, 2);
      Serial.print(F(" C  P: "));
      Serial.print(pressure, 2);
      Serial.println(F(" kPa"));
    }
    wf100dpz.stopSleepMode();
  }

  // --- Soft reset ---
  Serial.println(F("\n--- Soft Reset ---"));
  wf100dpz.softReset();
  delay(10);
  Serial.print(F("Part ID after reset: 0x"));
  Serial.println(wf100dpz.getPartID(), HEX);

  Serial.println();
  Serial.println(F("Continuous readings..."));
}

void loop() {
  float pressure, temperature;

  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("T: "));
    Serial.print(temperature, 2);
    Serial.print(F(" C  P: "));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  } else {
    Serial.println(F("Read failed!"));
  }

  delay(100);
}

void printStatus(uint8_t status) {
  Serial.print(F("Status: 0x"));
  Serial.println(status, HEX);
  Serial.print(F("  DRDY:           "));
  Serial.println((status & WF100DPZ_STATUS_DRDY) ? F("yes") : F("no"));
  Serial.print(F("  VINP short VDD: "));
  Serial.println((status & WF100DPZ_STATUS_VINP_VDD) ? F("yes") : F("no"));
  Serial.print(F("  VINP short GND: "));
  Serial.println((status & WF100DPZ_STATUS_VINP_GND) ? F("yes") : F("no"));
  Serial.print(F("  VINN short VDD: "));
  Serial.println((status & WF100DPZ_STATUS_VINN_VDD) ? F("yes") : F("no"));
  Serial.print(F("  VINN short GND: "));
  Serial.println((status & WF100DPZ_STATUS_VINN_GND) ? F("yes") : F("no"));
  Serial.print(F("  hasError:       "));
  Serial.println((status & WF100DPZ_STATUS_ERROR_MASK) ? F("YES") : F("no"));
}
