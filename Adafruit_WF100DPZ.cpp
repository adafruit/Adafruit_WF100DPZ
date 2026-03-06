/*!
 * @file Adafruit_WF100DPZ.cpp
 *
 * @mainpage Adafruit WF100DPZ Pressure Sensor Library
 *
 * @section intro_sec Introduction
 *
 * This is a library for the WF100DPZ low-cost MEMS gauge pressure sensor
 * with integrated temperature output.
 *
 * Conversion formulas derived from CodeyNacke/WF100DPZ (MIT license)
 * https://github.com/CodeyNacke/WF100DPZ
 *
 * @section author Author
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries.
 *
 * @section license License
 *
 * MIT license, all text above must be included in any redistribution.
 */

#include "Adafruit_WF100DPZ.h"

/**
 * @brief Construct a new Adafruit_WF100DPZ object
 */
Adafruit_WF100DPZ::Adafruit_WF100DPZ() {
  _i2c_dev = nullptr;
}

/**
 * @brief Destroy the Adafruit_WF100DPZ object
 */
Adafruit_WF100DPZ::~Adafruit_WF100DPZ() {
  if (_i2c_dev) {
    delete _i2c_dev;
  }
}

/**
 * @brief Initialize the sensor over I2C
 * @param addr I2C address (default 0x6D)
 * @param wire Pointer to TwoWire instance (default &Wire)
 * @return true if initialization successful (Part_ID verified)
 * @return false if I2C communication failed or Part_ID mismatch
 */
bool Adafruit_WF100DPZ::begin(uint8_t addr, TwoWire* wire) {
  if (_i2c_dev) {
    delete _i2c_dev;
  }

  _i2c_dev = new Adafruit_I2CDevice(addr, wire);

  if (!_i2c_dev->begin()) {
    return false;
  }

  // Perform soft reset — sensor NACKs during reset, so ignore the result
  softReset();

  // Wait for reset to complete
  delay(10);

  // Verify Part_ID to confirm sensor is alive and reset succeeded
  uint8_t part_id = getPartID();
  if (part_id != WF100DPZ_PART_ID) {
    return false;
  }

  return true;
}

/**
 * @brief Read the Part ID from the sensor
 * @return Part ID value (expect 0x49)
 */
uint8_t Adafruit_WF100DPZ::getPartID() {
  Adafruit_BusIO_Register part_id_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_PART_ID, 1);
  return part_id_reg.read();
}

/**
 * @brief Read the status register
 * @return Status register value (DRDY in bit 0, errors in bits 7:4)
 */
uint8_t Adafruit_WF100DPZ::getStatus() {
  Adafruit_BusIO_Register status_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_STATUS, 1);
  return status_reg.read();
}

/**
 * @brief Check if any error flags are set
 * @return true if any error flags (bits 7:4) are set
 * @return false if no errors
 */
bool Adafruit_WF100DPZ::hasError() {
  return (getStatus() & WF100DPZ_STATUS_ERROR_MASK) != 0;
}

/**
 * @brief Set the measurement mode
 * @param mode Measurement mode (TEMP_ONLY, PRESSURE_ONLY, COMBINED,
 *             SLEEP_PERIODIC)
 * @return true if write succeeded
 * @return false if write failed
 */
bool Adafruit_WF100DPZ::setMeasurementMode(wf100dpz_mode_t mode) {
  Adafruit_BusIO_Register cmd_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_CMD, 1);
  Adafruit_BusIO_RegisterBits mode_bits =
      Adafruit_BusIO_RegisterBits(&cmd_reg, 3, 0); // bits [2:0]
  return mode_bits.write((uint8_t)mode);
}

/**
 * @brief Get the current measurement mode
 * @return Current measurement mode
 */
wf100dpz_mode_t Adafruit_WF100DPZ::getMeasurementMode() {
  Adafruit_BusIO_Register cmd_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_CMD, 1);
  Adafruit_BusIO_RegisterBits mode_bits =
      Adafruit_BusIO_RegisterBits(&cmd_reg, 3, 0); // bits [2:0]
  return (wf100dpz_mode_t)mode_bits.read();
}

/**
 * @brief Set the sleep interval for periodic mode
 * @param interval Sleep interval enum value
 * @return true if write succeeded
 * @return false if write failed
 */
bool Adafruit_WF100DPZ::setSleepInterval(wf100dpz_sleep_t interval) {
  Adafruit_BusIO_Register cmd_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_CMD, 1);
  Adafruit_BusIO_RegisterBits sleep_bits =
      Adafruit_BusIO_RegisterBits(&cmd_reg, 4, 4); // bits [7:4]
  return sleep_bits.write((uint8_t)interval);
}

/**
 * @brief Get the current sleep interval setting
 * @return Current sleep interval enum value
 */
wf100dpz_sleep_t Adafruit_WF100DPZ::getSleepInterval() {
  Adafruit_BusIO_Register cmd_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_CMD, 1);
  Adafruit_BusIO_RegisterBits sleep_bits =
      Adafruit_BusIO_RegisterBits(&cmd_reg, 4, 4); // bits [7:4]
  return (wf100dpz_sleep_t)sleep_bits.read();
}

/**
 * @brief Perform a soft reset of the sensor
 * @return true if reset command was written successfully
 * @return false if write failed
 * @note Sensor NACKs during reset (drops off I2C bus momentarily).
 *       This is expected behavior.
 */
bool Adafruit_WF100DPZ::softReset() {
  Adafruit_BusIO_Register spi_ctrl_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_SPI_CTRL, 1);
  return spi_ctrl_reg.write(WF100DPZ_SOFT_RESET);
}

/**
 * @brief Wait for Data Ready (DRDY) flag
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return true if DRDY became set within timeout
 * @return false if timeout expired
 */
bool Adafruit_WF100DPZ::_waitDRDY(uint16_t timeout_ms) {
  uint32_t start = millis();

  Adafruit_BusIO_Register status_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_STATUS, 1);
  Adafruit_BusIO_RegisterBits drdy_bit =
      Adafruit_BusIO_RegisterBits(&status_reg, 1, 0); // bit [0]

  while ((millis() - start) < timeout_ms) {
    if (drdy_bit.read()) {
      return true;
    }
    delayMicroseconds(50);
  }
  return false;
}

/**
 * @brief Trigger a conversion in the specified mode
 * @param mode Measurement mode to trigger
 * @return true if command was written successfully
 * @return false if write failed
 */
bool Adafruit_WF100DPZ::_triggerConversion(wf100dpz_mode_t mode) {
  Adafruit_BusIO_Register cmd_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_CMD, 1);
  // Sco=1 (bit 3) | mode in [2:0]
  return cmd_reg.write(0x08 | ((uint8_t)mode & 0x07));
}

/**
 * @brief Read raw 24-bit pressure value
 * @param raw Pointer to store the signed 24-bit value
 * @return true if read successful
 * @return false if read failed
 */
bool Adafruit_WF100DPZ::_readRawPressure(int32_t* raw) {
  uint8_t buffer[3];

  Adafruit_BusIO_Register data_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_DATA_MSB, 3);
  if (!data_reg.read(buffer, 3)) {
    return false;
  }

  // Assemble 24-bit unsigned value (MSB first)
  uint32_t unsigned_raw =
      ((uint32_t)buffer[0] << 16) | ((uint32_t)buffer[1] << 8) | buffer[2];

  // Convert to signed 24-bit
  if (unsigned_raw >= 0x800000) {
    *raw = (int32_t)unsigned_raw - 0x1000000;
  } else {
    *raw = (int32_t)unsigned_raw;
  }

  return true;
}

/**
 * @brief Read raw temperature values
 * @param raw_msb Pointer to store the signed MSB
 * @param raw_lsb Pointer to store the unsigned LSB
 * @return true if read successful
 * @return false if read failed
 */
bool Adafruit_WF100DPZ::_readRawTemperature(int8_t* raw_msb, uint8_t* raw_lsb) {
  uint8_t buffer[2];

  Adafruit_BusIO_Register temp_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_TEMP_MSB, 2);
  if (!temp_reg.read(buffer, 2)) {
    return false;
  }

  // MSB is signed, LSB is unsigned
  *raw_msb = (int8_t)buffer[0];
  *raw_lsb = buffer[1];

  return true;
}

/**
 * @brief Read pressure from the sensor
 * @return Pressure in kPa, or NaN if read failed
 */
float Adafruit_WF100DPZ::readPressure() {
  if (!_triggerConversion(WF100DPZ_MODE_COMBINED)) {
    return NAN;
  }
  if (!_waitDRDY()) {
    return NAN;
  }

  int32_t raw;
  if (!_readRawPressure(&raw)) {
    return NAN;
  }

  float normalized = (float)raw / WF100DPZ_PRESSURE_DIV;
  return normalized * WF100DPZ_PRESSURE_SCALE + WF100DPZ_PRESSURE_OFFSET;
}

/**
 * @brief Read temperature from the sensor
 * @return Temperature in °C, or NaN if read failed
 */
float Adafruit_WF100DPZ::readTemperature() {
  if (!_triggerConversion(WF100DPZ_MODE_COMBINED)) {
    return NAN;
  }
  if (!_waitDRDY()) {
    return NAN;
  }

  int8_t raw_msb;
  uint8_t raw_lsb;
  if (!_readRawTemperature(&raw_msb, &raw_lsb)) {
    return NAN;
  }

  return WF100DPZ_TEMP_OFFSET + (float)raw_msb +
         (float)raw_lsb * WF100DPZ_TEMP_LSB_SCALE;
}

/**
 * @brief Read both temperature and pressure in a single conversion
 * @param pressure Pointer to store pressure in kPa
 * @param temperature Pointer to store temperature in °C
 * @return true if both readings successful
 * @return false if conversion or read failed
 */
bool Adafruit_WF100DPZ::readTempPressure(float* pressure, float* temperature) {
  if (!_triggerConversion(WF100DPZ_MODE_COMBINED)) {
    return false;
  }
  if (!_waitDRDY()) {
    return false;
  }

  int32_t raw_pressure;
  if (!_readRawPressure(&raw_pressure)) {
    return false;
  }

  int8_t raw_msb;
  uint8_t raw_lsb;
  if (!_readRawTemperature(&raw_msb, &raw_lsb)) {
    return false;
  }

  float normalized = (float)raw_pressure / WF100DPZ_PRESSURE_DIV;
  *pressure = normalized * WF100DPZ_PRESSURE_SCALE + WF100DPZ_PRESSURE_OFFSET;

  *temperature = WF100DPZ_TEMP_OFFSET + (float)raw_msb +
                 (float)raw_lsb * WF100DPZ_TEMP_LSB_SCALE;

  return true;
}

/**
 * @brief Enable sleep mode with periodic conversions
 * @param interval Sleep interval enum value
 * @return true if command was written successfully
 * @return false if write failed
 */
bool Adafruit_WF100DPZ::setSleepMode(wf100dpz_sleep_t interval) {
  Adafruit_BusIO_Register cmd_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_CMD, 1);
  // Sleep_time in upper nibble, Sco=1, Mode=SLEEP_PERIODIC (0x03)
  uint8_t cmd =
      (((uint8_t)interval & 0x0F) << 4) | 0x08 | WF100DPZ_MODE_SLEEP_PERIODIC;
  return cmd_reg.write(cmd);
}

/**
 * @brief Stop sleep mode and return to idle
 * @return true if command was written successfully
 * @return false if write failed
 */
bool Adafruit_WF100DPZ::stopSleepMode() {
  Adafruit_BusIO_Register cmd_reg =
      Adafruit_BusIO_Register(_i2c_dev, WF100DPZ_REG_CMD, 1);
  return cmd_reg.write(0x00);
}
