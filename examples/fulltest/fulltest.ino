/*!
 * @file fulltest.ino
 *
 * Full test sketch for WF100DPZ Pressure and Temperature Sensor
 *
 * Limor 'ladyada' Fried with assistance from Claude Code
 * MIT License
 */

#include <Adafruit_WF100DPZ.h>
#include <Wire.h>

Adafruit_WF100DPZ wf100dpz;

void printStatus(uint8_t status);

void setup() {
  Serial.begin(115200);

  while (!Serial)
    delay(10);

  Serial.println(F("WF100DPZ Pressure and Temperature Sensor Test"));

  if (!wf100dpz.begin()) {
    Serial.println(F("Failed to find WF100DPZ chip"));
    while (1)
      delay(10);
  }

  Serial.println(F("WF100DPZ found!"));

  Serial.print(F("Part ID: 0x"));
  Serial.println(wf100dpz.getPartID(), HEX);

  uint8_t status = wf100dpz.getStatus();
  printStatus(status);

  // --- Measurement mode ---
  wf100dpz.setMeasurementMode(WF100DPZ_MODE_COMBINED);
  Serial.print(F("Measurement mode: "));
  wf100dpz_mode_t mode = wf100dpz.getMeasurementMode();
  switch (mode) {
    case WF100DPZ_MODE_TEMP_ONLY:
      Serial.println(F("Temperature only"));
      break;
    case WF100DPZ_MODE_PRESSURE_ONLY:
      Serial.println(F("Pressure only"));
      break;
    case WF100DPZ_MODE_COMBINED:
      Serial.println(F("Combined (temp + pressure)"));
      break;
    case WF100DPZ_MODE_SLEEP_PERIODIC:
      Serial.println(F("Sleep periodic"));
      break;
    default:
      Serial.println(F("Unknown"));
      break;
  }

  // --- Sleep interval ---
  wf100dpz.setSleepInterval(WF100DPZ_SLEEP_500MS);
  Serial.print(F("Sleep interval: "));
  wf100dpz_sleep_t interval = wf100dpz.getSleepInterval();
  switch (interval) {
    case WF100DPZ_SLEEP_0MS:
      Serial.println(F("0 ms (continuous)"));
      break;
    case WF100DPZ_SLEEP_62MS:
      Serial.println(F("62.5 ms"));
      break;
    case WF100DPZ_SLEEP_125MS:
      Serial.println(F("125 ms"));
      break;
    case WF100DPZ_SLEEP_187MS:
      Serial.println(F("187.5 ms"));
      break;
    case WF100DPZ_SLEEP_250MS:
      Serial.println(F("250 ms"));
      break;
    case WF100DPZ_SLEEP_312MS:
      Serial.println(F("312.5 ms"));
      break;
    case WF100DPZ_SLEEP_375MS:
      Serial.println(F("375 ms"));
      break;
    case WF100DPZ_SLEEP_437MS:
      Serial.println(F("437.5 ms"));
      break;
    case WF100DPZ_SLEEP_500MS:
      Serial.println(F("500 ms"));
      break;
    case WF100DPZ_SLEEP_562MS:
      Serial.println(F("562.5 ms"));
      break;
    case WF100DPZ_SLEEP_625MS:
      Serial.println(F("625 ms"));
      break;
    case WF100DPZ_SLEEP_687MS:
      Serial.println(F("687.5 ms"));
      break;
    case WF100DPZ_SLEEP_750MS:
      Serial.println(F("750 ms"));
      break;
    case WF100DPZ_SLEEP_812MS:
      Serial.println(F("812.5 ms"));
      break;
    case WF100DPZ_SLEEP_875MS:
      Serial.println(F("875 ms"));
      break;
    case WF100DPZ_SLEEP_1000MS:
      Serial.println(F("1000 ms"));
      break;
    default:
      Serial.println(F("Unknown"));
      break;
  }

  // --- Sleep mode demo ---
  wf100dpz.setSleepMode(WF100DPZ_SLEEP_125MS);
  Serial.println(F("Sleep mode started at 125 ms interval"));
  delay(200);
  float pressure, temperature;
  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("  Periodic read: T="));
    Serial.print(temperature, 2);
    Serial.print(F(" C, P="));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  }
  wf100dpz.stopSleepMode();
  Serial.println(F("Sleep mode stopped"));

  // --- Soft reset ---
  wf100dpz.softReset();
  delay(10);
  Serial.print(F("Part ID after reset: 0x"));
  Serial.println(wf100dpz.getPartID(), HEX);
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
