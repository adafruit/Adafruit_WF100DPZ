# Adafruit WF100DPZ [![Arduino Library CI](https://github.com/adafruit/Adafruit_WF100DPZ/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_WF100DPZ/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](https://adafruit.github.io/Adafruit_WF100DPZ/html/index.html)

Arduino library for the WF100DPZ low-cost MEMS gauge pressure sensor with integrated temperature output.

## Features

- I2C interface (address 0x6D)
- 24-bit pressure reading in kPa (gauge mode)
- 16-bit temperature reading in °C
- Single-shot and periodic conversion modes
- 16 configurable sleep/periodic intervals
- Status register with error flag decoding

## Installation

Search for **Adafruit WF100DPZ** in the Arduino Library Manager, or clone this repository into your Arduino libraries folder.

### Dependencies

- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO)

## Usage

### Periodic Mode (simplest)

```cpp
#include <Adafruit_WF100DPZ.h>

Adafruit_WF100DPZ wf100dpz;

void setup() {
  Serial.begin(115200);
  wf100dpz.begin();
  wf100dpz.setSleepMode(WF100DPZ_SLEEP_125MS);
  delay(150);
}

void loop() {
  float pressure, temperature;
  wf100dpz.readTempPressure(&pressure, &temperature);
  Serial.print(temperature, 2);
  Serial.print(" C, ");
  Serial.print(pressure, 2);
  Serial.println(" kPa");
  delay(125);
}
```

### Single-Shot Mode

```cpp
wf100dpz.triggerConversion();
wf100dpz.waitDRDY();
float pressure = wf100dpz.readPressure();
float temperature = wf100dpz.readTemperature();
```

## Sensor Variants

The WF100DPZ comes in multiple pressure ranges (5–300 kPa). The conversion constants are defined as macros and can be adjusted for different variants:

```cpp
#define WF100DPZ_PRESSURE_SCALE   250.0
#define WF100DPZ_PRESSURE_OFFSET  25.0
```

## Credits

Conversion formulas derived from [CodeyNacke/WF100DPZ](https://github.com/CodeyNacke/WF100DPZ) (MIT license).

## License

MIT license — see [license.txt](license.txt) for details.
