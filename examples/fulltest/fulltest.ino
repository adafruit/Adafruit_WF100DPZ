/*!
 * @file fulltest.ino
 *
 * Full example for the WF100DPZ pressure sensor.
 * Demonstrates all library features: single readings,
 * combined readings, status checks, and sleep mode.
 */

#include <Adafruit_WF100DPZ.h>
#include <Wire.h>

Adafruit_WF100DPZ wf100dpz;

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
  Serial.println(F("WF100DPZ Found!"));
  Serial.println();

  // --- Individual readings ---
  Serial.println(F("--- Individual Readings ---"));

  float pressure = wf100dpz.readPressure();
  Serial.print(F("Pressure: "));
  Serial.print(pressure, 2);
  Serial.println(F(" kPa"));

  float temperature = wf100dpz.readTemperature();
  Serial.print(F("Temperature: "));
  Serial.print(temperature, 2);
  Serial.println(F(" C"));
  Serial.println();

  // --- Combined reading ---
  Serial.println(F("--- Combined Reading ---"));

  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("Temperature: "));
    Serial.print(temperature, 2);
    Serial.print(F(" C, Pressure: "));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  } else {
    Serial.println(F("Combined read failed!"));
  }
  Serial.println();

  // --- Status check ---
  Serial.println(F("--- Status Check ---"));
  uint8_t status = wf100dpz.getStatus();
  Serial.print(F("Status: 0x"));
  Serial.println(status, HEX);
  Serial.print(F("Errors: "));
  Serial.println(wf100dpz.hasError() ? F("YES") : F("none"));
  Serial.println();

  // --- Sleep mode demo ---
  Serial.println(F("--- Sleep Mode (interval=2, ~125ms) ---"));
  Serial.println(F("Reading 5 periodic conversions..."));

  if (wf100dpz.setSleepMode(2)) {
    for (int i = 0; i < 5; i++) {
      delay(200); // wait for periodic conversion
      if (wf100dpz.readTempPressure(&pressure, &temperature)) {
        Serial.print(i + 1);
        Serial.print(F(": T="));
        Serial.print(temperature, 2);
        Serial.print(F(" C, P="));
        Serial.print(pressure, 2);
        Serial.println(F(" kPa"));
      }
    }
    wf100dpz.stopSleepMode();
    Serial.println(F("Sleep mode stopped."));
  } else {
    Serial.println(F("Failed to enter sleep mode!"));
  }
  Serial.println();

  // --- Soft reset ---
  Serial.println(F("--- Soft Reset ---"));
  wf100dpz.softReset();
  delay(10);
  Serial.print(F("Part ID after reset: 0x"));
  Serial.println(wf100dpz.getPartID(), HEX);
  Serial.println();

  Serial.println(F("Setup complete. Continuous readings in loop..."));
  Serial.println();
}

void loop() {
  float pressure, temperature;

  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("T="));
    Serial.print(temperature, 2);
    Serial.print(F(" C  P="));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  } else {
    Serial.println(F("Read failed!"));
  }

  delay(1000);
}
