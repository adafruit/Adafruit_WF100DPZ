/*!
 * @file Adafruit_SPI3Device.cpp
 *
 * Implementation of bit-banged 3-wire SPI device class.
 *
 * Written by Limor 'ladyada' Fried for Adafruit Industries.
 * MIT license, all text above must be included in any redistribution.
 */

#include "Adafruit_SPI3Device.h"

/**
 * @brief Construct a new 3-wire SPI device
 * @param cs Chip select pin
 * @param clk Clock pin
 * @param data Bidirectional data pin
 * @param freq Clock frequency in Hz (default 1MHz)
 */
Adafruit_SPI3Device::Adafruit_SPI3Device(int8_t cs, int8_t clk, int8_t data,
                                         uint32_t freq) {
  _cs = cs;
  _clk = clk;
  _data = data;
  _freq = freq;
  _generic_dev = nullptr;

  // Calculate bit delay in microseconds (half period)
  // freq = 1 / (2 * delay), so delay = 1000000 / (2 * freq)
  if (_freq >= 500000) {
    _bitdelay_us = 1;
  } else {
    _bitdelay_us = 500000 / _freq;
  }
}

/**
 * @brief Destroy the 3-wire SPI device
 */
Adafruit_SPI3Device::~Adafruit_SPI3Device() {
  if (_generic_dev) {
    delete _generic_dev;
  }
}

/**
 * @brief Initialize the 3-wire SPI device
 * @return true always (pin setup cannot fail)
 */
bool Adafruit_SPI3Device::begin() {
  // Set up pins
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH); // CS idle high

  pinMode(_clk, OUTPUT);
  digitalWrite(_clk, LOW); // Clock idles low (Mode 0)

  pinMode(_data, OUTPUT);
  digitalWrite(_data, LOW);

  // Create GenericDevice with our callbacks
  _generic_dev = new Adafruit_GenericDevice(this, nullptr, nullptr, _readreg_cb,
                                            _writereg_cb);
  _generic_dev->begin();

  return true;
}

/**
 * @brief Get pointer to the GenericDevice for use with BusIO registers
 * @return Pointer to Adafruit_GenericDevice
 */
Adafruit_GenericDevice* Adafruit_SPI3Device::getGenericDevice() {
  return _generic_dev;
}

/**
 * @brief Clock out a byte MSB first (Mode 0: set on falling, sample on rising)
 * @param b Byte to send
 */
void Adafruit_SPI3Device::_clockOutByte(uint8_t b) {
  pinMode(_data, OUTPUT);

  for (uint8_t i = 0; i < 8; i++) {
    // Set data bit (MSB first)
    if (b & 0x80) {
      digitalWrite(_data, HIGH);
    } else {
      digitalWrite(_data, LOW);
    }
    b <<= 1;

    // Rising edge (data is sampled by device)
    digitalWrite(_clk, HIGH);
    delayMicroseconds(_bitdelay_us);

    // Falling edge
    digitalWrite(_clk, LOW);
    delayMicroseconds(_bitdelay_us);
  }
}

/**
 * @brief Clock in a byte MSB first (Mode 0: sample on rising edge)
 * @return Byte read
 */
uint8_t Adafruit_SPI3Device::_clockInByte() {
  uint8_t b = 0;

  pinMode(_data, INPUT_PULLUP);

  for (uint8_t i = 0; i < 8; i++) {
    b <<= 1;

    // Rising edge - sample data
    digitalWrite(_clk, HIGH);
    delayMicroseconds(_bitdelay_us);

    if (digitalRead(_data)) {
      b |= 1;
    }

    // Falling edge
    digitalWrite(_clk, LOW);
    delayMicroseconds(_bitdelay_us);
  }

  return b;
}

/**
 * @brief Static callback for GenericDevice readRegister
 * @param obj Pointer to Adafruit_SPI3Device instance
 * @param addr_buf Register address buffer
 * @param addrsiz Address size (should be 1)
 * @param data Data buffer to fill
 * @param datalen Number of bytes to read
 * @return true on success
 */
bool Adafruit_SPI3Device::_readreg_cb(void* obj, uint8_t* addr_buf,
                                      uint8_t addrsiz, uint8_t* data,
                                      uint16_t datalen) {
  Adafruit_SPI3Device* dev = (Adafruit_SPI3Device*)obj;

  // CS low
  digitalWrite(dev->_cs, LOW);

  // Clock out address with read bit (bit 7 set)
  dev->_clockOutByte(addr_buf[0] | 0x80);

  // Clock in data bytes
  for (uint16_t i = 0; i < datalen; i++) {
    data[i] = dev->_clockInByte();
  }

  // CS high
  digitalWrite(dev->_cs, HIGH);

  return true;
}

/**
 * @brief Static callback for GenericDevice writeRegister
 * @param obj Pointer to Adafruit_SPI3Device instance
 * @param addr_buf Register address buffer
 * @param addrsiz Address size (should be 1)
 * @param data Data buffer to write
 * @param datalen Number of bytes to write
 * @return true on success
 */
bool Adafruit_SPI3Device::_writereg_cb(void* obj, uint8_t* addr_buf,
                                       uint8_t addrsiz, const uint8_t* data,
                                       uint16_t datalen) {
  Adafruit_SPI3Device* dev = (Adafruit_SPI3Device*)obj;

  // CS low
  digitalWrite(dev->_cs, LOW);

  // Clock out address with write bit (bit 7 clear)
  dev->_clockOutByte(addr_buf[0] & 0x7F);

  // Clock out data bytes
  for (uint16_t i = 0; i < datalen; i++) {
    dev->_clockOutByte(data[i]);
  }

  // CS high
  digitalWrite(dev->_cs, HIGH);

  return true;
}
