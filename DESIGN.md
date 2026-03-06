# Adafruit_WF100DPZ Design Document

## Chip Overview
- **Part:** WF100DPZ (Weifengheng) — low-cost MEMS gauge pressure sensor
- **Variant on breakout:** WF100DPZ 2BG S6 DT (200 kPa gauge, SOP-6)
- **Interface:** I2C only (see SPI Investigation Notes below)
- **I2C address:** 0x6D (confirmed by bus scan; not documented in datasheet)
- **Part_ID:** 0x49 (OTP-programmed, read from register 0x01)
- **Second device on bus:** 0x32 — appears to be onboard EEPROM (not used by driver)
- **Datasheet:** Product brief only; no programming guide. Conversion formulas
  derived from CodeyNacke/WF100DPZ reference library
  (https://github.com/CodeyNacke/WF100DPZ, MIT license).
  Credit to CodeyNacke for documenting the undocumented output format.
  Also referenced: lessthan-3/v2devapollo pressure_sensor.h (2BG variant,
  different scaling approach — uses range-specific multiplier without offset,
  but produces incorrect ambient readings; CodeyNacke formula verified correct).

## Pressure Specifications
- **Range:** 200 kPa gauge (2B variant)
- **Available variants:** 5/10/20/40/100/200/300 kPa
- **Type:** Gauge (measures relative to atmospheric)
- **Resolution:** 0.05 Pa (ultra-high resolution mode)
- **Sensitivity:** 0.027 Pa/LSB (per datasheet; actual output uses different encoding)
- **Overpressure:** 3× full scale (600 kPa)

## Temperature Specifications
- **Resolution:** 0.01°C (ultra-high resolution mode)
- **Accuracy:** ±0.5°C at 25°C, ±1.0°C over 0–65°C
- **Compensation range:** -20°C to 85°C
- **Operating range:** -40°C to 125°C

## Register Map

| Addr | Name      | R/W | Description                          | Default |
|------|-----------|-----|--------------------------------------|---------|
| 0x00 | SPI_CTRL  | RW  | SPI config + soft reset              | 0x00    |
| 0x01 | PART_ID   | R   | OTP part ID (expect 0x49)            | 0x49    |
| 0x02 | STATUS    | R   | Error flags [7:4], DRDY [0]          | 0x00    |
| 0x06 | DATA_MSB  | R   | Pressure data [23:16]                | 0x00    |
| 0x07 | DATA_CSB  | R   | Pressure data [15:8]                 | 0x00    |
| 0x08 | DATA_LSB  | R   | Pressure data [7:0]                  | 0x00    |
| 0x09 | TEMP_MSB  | R   | Temperature data [15:8]              | 0x00    |
| 0x0A | TEMP_LSB  | R   | Temperature data [7:0]               | 0x00    |
| 0x30 | CMD       | RW  | Sleep_time [7:4], Sco [3], Mode [2:0]| 0x00    |

### STATUS Register (0x02)
| Bit | Name           | Description                    |
|-----|----------------|--------------------------------|
| 7   | Error_code[3]  | VINP short to VDD              |
| 6   | Error_code[2]  | VINP short to GND              |
| 5   | Error_code[1]  | VINN short to VDD              |
| 4   | Error_code[0]  | VINN short to GND              |
| 3:1 | Reserved       | —                              |
| 0   | DRDY           | 1 = conversion complete        |

### CMD Register (0x30)
| Bit | Name             | Description                            |
|-----|------------------|----------------------------------------|
| 7:4 | Sleep_time[3:0]  | Periodic interval (N × 62.5 ms approx) |
| 3   | Sco              | Start conversion (auto-clears)         |
| 2:0 | Measurement[2:0] | Mode select (see below)                |

### Measurement Modes
| Value | Mode                | Description                            |
|-------|---------------------|----------------------------------------|
| 0b000 | TEMP_ONLY          | Single temperature conversion          |
| 0b001 | PRESSURE_ONLY      | Single pressure conversion             |
| 0b010 | COMBINED           | Temperature then pressure (recommended)|
| 0b011 | SLEEP_PERIODIC     | Periodic combined at Sleep_time rate   |

### Sleep Time Values
| Code | Nominal Interval | Measured Interval |
|------|------------------|-------------------|
| 0x0  | 0 ms             | —                 |
| 0x1  | 62.5 ms          | —                 |
| 0x2  | 125 ms           | ~96 ms            |
| ...  | ...              | ...               |
| 0xF  | 937.5 ms (15×62.5) | ~693 ms         |

**Note:** Actual sleep intervals are ~25-30% shorter than the nominal
values calculated from the datasheet formula (N × 62.5 ms). The
proportional scaling between intervals is correct. This appears to be
a chip characteristic, not a measurement error — verified with
hw_test_13 which polls DRDY and measures wall-clock time between
consecutive conversion-ready events.

### Soft Reset
Write 0x24 to register 0x00 (sets bits 5 and 2). Bits auto-clear when reset completes.

## Data Conversion Formulas

### Pressure (24-bit signed → kPa)

Manufacturer documentation confirms:
- **Zero condition:** 8388608 (2^23)
- **Below zero (raw < 2^23):** `v = raw / 8388608`
- **Above zero (raw >= 2^23):** `v = (raw - 16777216) / 8388608`
- This is equivalent to signed 24-bit interpretation divided by 2^23

The manufacturer doc gives a normalized value `v` (range -1.0 to +1.0) but does
NOT document the final conversion to kPa. The scaling constants below are from
the CodeyNacke reference library:

```
raw = (DATA_MSB << 16) | (DATA_CSB << 8) | DATA_LSB
signed_raw = (raw >= 0x800000) ? (raw - 0x1000000) : raw
normalized = signed_raw / 8388608.0          // divide by 2^23
pressure_kPa = normalized * 250.0 + 25.0
```

Source: CodeyNacke/WF100DPZ (https://github.com/CodeyNacke/WF100DPZ, MIT).
Scale/offset constants (250.0, 25.0) are #defined so they can be tuned.

Verified on hardware: at ambient (0 gauge), reads ~1.9 kPa — reasonable for
uncalibrated gauge sensor on bench.

### Temperature (16-bit → °C)
```
MSB_signed = (TEMP_MSB >= 0x80) ? (TEMP_MSB - 256) : TEMP_MSB
temp_C = -1.2 + MSB_signed + TEMP_LSB * 0.004
```
Source: CodeyNacke/WF100DPZ (MIT). Note: datasheet claims
`temp = raw / 256.0` but this gives a ~1°C offset; the formula above is
what the reference library uses and produces correct room-temp readings.

## I2C Protocol

From manufacturer documentation:
```
I2C Address: 1101101 (0x6D) — 7-bit address + R/W bit

Write command:
  S DeviceAddress W A 0x30 A 0x0A A P

Read pressure (burst read 3 bytes):
  S DeviceAddress W A 0x06 A
  S DeviceAddress R A PRESSURE_MSB A PRESSURE_CSB A PRESSURE_LSB N P
```

- Max clock: 3.4 MHz (per datasheet at 3V supply)
- DRDY polling: read STATUS register (0x02), check bit 0
  - **Status is auto-cleared after read** (read-once)
- Conversion timeout: ~12 ms typical (per reference library)
- DRDY poll interval: 50 µs (per reference library)

## Conversion Sequence (Combined Mode — Recommended)
1. Write 0x0A to CMD register (Sco=1, Mode=COMBINED)
2. Poll STATUS register until DRDY=1 (timeout ~12 ms)
3. Read DATA_MSB, DATA_CSB, DATA_LSB (pressure)
4. Read TEMP_MSB, TEMP_LSB (temperature)
5. Apply conversion formulas above

## Library API Design

### Class: Adafruit_WF100DPZ

```cpp
class Adafruit_WF100DPZ {
public:
  Adafruit_WF100DPZ();

  // Initialize sensor. Returns false if Part_ID != 0x49 or I2C fails.
  bool begin(uint8_t addr = 0x6D, TwoWire *wire = &Wire);

  // Single-shot readings
  float readPressure();         // Returns pressure in kPa (NaN on error)
  float readTemperature();      // Returns temperature in °C (NaN on error)
  bool readTempPressure(float *pressure, float *temperature);

  // Sensor info
  uint8_t getPartID();
  uint8_t getStatus();
  bool hasError();

  // Periodic (sleep) mode
  bool setSleepMode(uint8_t interval);  // 0-15, interval × ~62.5 ms
  bool stopSleepMode();

  // Reset
  bool softReset();

private:
  Adafruit_I2CDevice *_i2c_dev;
  Adafruit_BusIO_Register *_cmd_reg;
  Adafruit_BusIO_Register *_status_reg;

  bool _waitDRDY(uint16_t timeout_ms = 15);
  bool _triggerCombined();
  bool _readRawPressure(int32_t *raw);
  bool _readRawTemperature(int16_t *raw_msb, uint8_t *raw_lsb);
};
```

### Dependencies
- Adafruit_BusIO (I2CDevice + Register)

### Conversion Constants
```cpp
#define WF100DPZ_DEFAULT_ADDR     0x6D
#define WF100DPZ_PART_ID          0x49

#define WF100DPZ_REG_SPI_CTRL     0x00
#define WF100DPZ_REG_PART_ID      0x01
#define WF100DPZ_REG_STATUS       0x02
#define WF100DPZ_REG_DATA_MSB     0x06
#define WF100DPZ_REG_DATA_CSB     0x07
#define WF100DPZ_REG_DATA_LSB     0x08
#define WF100DPZ_REG_TEMP_MSB     0x09
#define WF100DPZ_REG_TEMP_LSB     0x0A
#define WF100DPZ_REG_CMD          0x30

#define WF100DPZ_CMD_COMBINED     0x0A  // Sco=1, Mode=COMBINED
#define WF100DPZ_CMD_TEMP_ONLY    0x08  // Sco=1, Mode=TEMP_ONLY
#define WF100DPZ_CMD_PRESSURE     0x09  // Sco=1, Mode=PRESSURE_ONLY
#define WF100DPZ_SOFT_RESET       0x24

#define WF100DPZ_PRESSURE_SCALE   250.0
#define WF100DPZ_PRESSURE_OFFSET  25.0
#define WF100DPZ_PRESSURE_DIV     8388608.0  // 2^23
#define WF100DPZ_TEMP_OFFSET      -1.2
#define WF100DPZ_TEMP_LSB_SCALE   0.004
```

## Hardware Test Plan
1. I2C detect at 0x6D
2. Part_ID read (expect 0x49)
3. Soft reset + verify registers return to defaults
4. Temperature reading (sanity check 15–35°C room temp)
5. Pressure reading (sanity check -5 to +10 kPa at ambient for gauge)
6. Combined reading (both valid simultaneously)
7. Status register / error flags (should be clean with no shorts)
8. Sleep mode start/stop (verify periodic conversions)
9. Conversion timing (DRDY goes high within expected window)
10. Repeated reads stability (low noise over 10 samples)

## Examples
1. **simpletest** — combined temp + pressure reading in loop
2. **sleepmode** — periodic conversion with configurable interval

## SPI Investigation Notes (2026-03-06)

The WF100DPZ datasheet describes a 3-wire SPI interface. Hardware testing
confirmed CSB is functional (pulling CSB LOW disables I2C — verified by
hw_test_12), but SPI data reads returned 0xFF in all configurations tested:

- **All 4 SPI modes** (CPOL/CPHA combinations)
- **Both addressing formats:** 2-byte (CMD + ADDR per CodeyNacke protocol)
  and 1-byte (ADDR | 0x80)
- **Turnaround clocks** (0-4 extra clock cycles between address and data)
- **Various clock speeds** (fast port-manipulation to 50kHz bit-bang)
- **With and without SPI_CTRL init** (writeReg 0x00 = 0x01 per CodeyNacke)

The SDA line remains at pullup level during all read attempts, indicating
the sensor's SPI output driver is not driving the pin.

**SPI protocol reference** (CodeyNacke/WF100DPZ):
- Read: CS LOW → 0x80 (CMD_READ) → addr → [switch to input] → read byte → CS HIGH
- Write: CS LOW → 0x00 (CMD_WRITE) → addr → data byte → CS HIGH
- Note: separate CMD and ADDR bytes, NOT combined

**Conclusion:** The SOP-6 package variant (S6) may not implement the 3-wire
SPI output driver. The 6th pin is NC (no SDO for 4-wire SPI). The sensor
correctly enters SPI mode (I2C stops responding) but never drives data out.
Library ships as I2C-only. SPI support code preserved in git history
(commit ae7df75) for future investigation with other package variants.
