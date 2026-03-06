/*!
 * @file fulltest.ino
 *
 * Full example for the WF100DPZ pressure sensor.
 * Displays sensor info, exercises all API methods, then
 * loops with continuous combined readings.
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

  // --- Status register ---
  uint8_t status = wf100dpz.getStatus();
  printStatus(status);

  // --- Individual pressure reading ---
  Serial.print(F("readPressure: "));
  Serial.print(wf100dpz.readPressure(), 2);
  Serial.println(F(" kPa"));

  // --- Individual temperature reading ---
  Serial.print(F("readTemperature: "));
  Serial.print(wf100dpz.readTemperature(), 2);
  Serial.println(F(" C"));

  // --- Combined reading ---
  float pressure, temperature;
  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("readTempPressure: "));
    Serial.print(temperature, 2);
    Serial.print(F(" C, "));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  }

  // --- Sleep mode ---
  Serial.println(F("setSleepMode(2): "));
  Serial.println(wf100dpz.setSleepMode(2) ? F("  OK") : F("  FAIL"));
  delay(200);
  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("  periodic read: "));
    Serial.print(temperature, 2);
    Serial.print(F(" C, "));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  }
  Serial.println(wf100dpz.stopSleepMode() ? F("  stopped") : F("  stop FAIL"));

  // --- Soft reset ---
  Serial.print(F("softReset: "));
  wf100dpz.softReset();
  delay(10);
  Serial.print(F("Part ID after: 0x"));
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
  Serial.print(F("  DRDY:          "));
  Serial.println((status & WF100DPZ_STATUS_DRDY) ? F("yes") : F("no"));
  Serial.print(F("  VINP short VDD: "));
  Serial.println((status & WF100DPZ_STATUS_VINP_VDD) ? F("yes") : F("no"));
  Serial.print(F("  VINP short GND: "));
  Serial.println((status & WF100DPZ_STATUS_VINP_GND) ? F("yes") : F("no"));
  Serial.print(F("  VINN short VDD: "));
  Serial.println((status & WF100DPZ_STATUS_VINN_VDD) ? F("yes") : F("no"));
  Serial.print(F("  VINN short GND: "));
  Serial.println((status & WF100DPZ_STATUS_VINN_GND) ? F("yes") : F("no"));
  Serial.print(F("  hasError:      "));
  Serial.println((status & WF100DPZ_STATUS_ERROR_MASK) ? F("YES") : F("no"));
}
