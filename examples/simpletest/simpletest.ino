/*!
 * @file simpletest.ino
 *
 * Simple example for the WF100DPZ pressure sensor.
 * Uses periodic mode to continuously read temperature and pressure.
 */

#include <Adafruit_WF100DPZ.h>

Adafruit_WF100DPZ wf100dpz;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("WF100DPZ Simple Test"));
  Serial.println(F(""));

  if (!wf100dpz.begin()) {
    Serial.println(F("Failed to find WF100DPZ sensor!"));
    while (1) {
      delay(10);
    }
  }

  Serial.print(F("Part ID: 0x"));
  Serial.println(wf100dpz.getPartID(), HEX);
  Serial.println(F("WF100DPZ Found!"));
  Serial.println(F(""));

  // Start periodic conversions at ~125ms interval
  wf100dpz.setSleepMode(WF100DPZ_SLEEP_125MS);
  delay(150); // wait for first conversion
}

void loop() {
  float pressure, temperature;

  if (wf100dpz.readTempPressure(&pressure, &temperature)) {
    Serial.print(F("Temperature: "));
    Serial.print(temperature, 2);
    Serial.print(F(" C, Pressure: "));
    Serial.print(pressure, 2);
    Serial.println(F(" kPa"));
  } else {
    Serial.println(F("Failed to read sensor!"));
  }

  delay(100);
}
