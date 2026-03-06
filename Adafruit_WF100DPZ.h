/*!
 * @file Adafruit_WF100DPZ.h
 *
 * Arduino library for the WF100DPZ pressure sensor.
 *
 * This is a library for the WF100DPZ low-cost MEMS gauge pressure sensor
 * with integrated temperature output.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries.
 *
 * MIT license, all text above must be included in any redistribution.
 *
 * Conversion formulas derived from CodeyNacke/WF100DPZ (MIT license)
 * https://github.com/CodeyNacke/WF100DPZ
 */

#ifndef ADAFRUIT_WF100DPZ_H
#define ADAFRUIT_WF100DPZ_H

#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Arduino.h>
#include <Wire.h>

/** Default I2C address (confirmed by bus scan; not in datasheet) */
#define WF100DPZ_DEFAULT_ADDR 0x6D
/** Expected Part ID value from OTP */
#define WF100DPZ_PART_ID 0x49

/** @name Register Addresses */
/**@{*/
/** SPI config and soft reset register */
#define WF100DPZ_REG_SPI_CTRL 0x00
/** OTP part ID register */
#define WF100DPZ_REG_PART_ID 0x01
/** Status register with error flags and DRDY */
#define WF100DPZ_REG_STATUS 0x02
/** Pressure data MSB [23:16] */
#define WF100DPZ_REG_DATA_MSB 0x06
/** Pressure data CSB [15:8] */
#define WF100DPZ_REG_DATA_CSB 0x07
/** Pressure data LSB [7:0] */
#define WF100DPZ_REG_DATA_LSB 0x08
/** Temperature data MSB [15:8] */
#define WF100DPZ_REG_TEMP_MSB 0x09
/** Temperature data LSB [7:0] */
#define WF100DPZ_REG_TEMP_LSB 0x0A
/** Command register for measurement control */
#define WF100DPZ_REG_CMD 0x30
/**@}*/

/** @name Command Values */
/**@{*/
/** Combined temp + pressure conversion (Sco=1, Mode=COMBINED) */
#define WF100DPZ_CMD_COMBINED 0x0A
/** Temperature only conversion (Sco=1, Mode=TEMP_ONLY) */
#define WF100DPZ_CMD_TEMP_ONLY 0x08
/** Pressure only conversion (Sco=1, Mode=PRESSURE_ONLY) */
#define WF100DPZ_CMD_PRESSURE 0x09
/** Soft reset command value */
#define WF100DPZ_SOFT_RESET 0x24
/**@}*/

/** @name Conversion Constants
 *  May need tuning for specific hardware variants */
/**@{*/
/** Pressure scale factor (kPa) */
#define WF100DPZ_PRESSURE_SCALE 250.0
/** Pressure offset (kPa) */
#define WF100DPZ_PRESSURE_OFFSET 25.0
/** Pressure divisor (2^23) */
#define WF100DPZ_PRESSURE_DIV 8388608.0
/** Temperature offset (°C) */
#define WF100DPZ_TEMP_OFFSET (-1.2)
/** Temperature LSB scale factor */
#define WF100DPZ_TEMP_LSB_SCALE 0.004
/**@}*/

/** @name Status Register Bits */
/**@{*/
/** DRDY bit in status register */
#define WF100DPZ_STATUS_DRDY 0x01
/** Error flag mask (bits 7:4) */
#define WF100DPZ_STATUS_ERROR_MASK 0xF0
/**@}*/

/**
 * @brief Measurement mode enumeration
 */
typedef enum {
  WF100DPZ_MODE_TEMP_ONLY = 0x00,     /**< Single temperature conversion */
  WF100DPZ_MODE_PRESSURE_ONLY = 0x01, /**< Single pressure conversion */
  WF100DPZ_MODE_COMBINED = 0x02,      /**< Temperature then pressure */
  WF100DPZ_MODE_SLEEP_PERIODIC = 0x03 /**< Periodic combined conversions */
} wf100dpz_mode_t;

/**
 * @brief Class for interfacing with the WF100DPZ pressure sensor
 */
class Adafruit_WF100DPZ {
 public:
  Adafruit_WF100DPZ();
  ~Adafruit_WF100DPZ();

  bool begin(uint8_t addr = WF100DPZ_DEFAULT_ADDR, TwoWire* wire = &Wire);

  float readPressure();
  float readTemperature();
  bool readTempPressure(float* pressure, float* temperature);

  uint8_t getPartID();
  uint8_t getStatus();
  bool hasError();

  bool setSleepMode(uint8_t interval);
  bool stopSleepMode();
  bool softReset();

 private:
  Adafruit_I2CDevice* _i2c_dev;

  bool _waitDRDY(uint16_t timeout_ms = 15);
  bool _triggerCombined();
  bool _readRawPressure(int32_t* raw);
  bool _readRawTemperature(int8_t* raw_msb, uint8_t* raw_lsb);
};

#endif // ADAFRUIT_WF100DPZ_H
