/*!
 * @file Adafruit_SPI3Device.h
 *
 * Bit-banged 3-wire SPI device class for use with BusIO's GenericDevice.
 *
 * 3-wire SPI uses a bidirectional DATA pin instead of separate MOSI/MISO.
 * This class implements SPI Mode 0 (CPOL=0, CPHA=0).
 *
 * Written by Limor 'ladyada' Fried for Adafruit Industries.
 * MIT license, all text above must be included in any redistribution.
 */

#ifndef ADAFRUIT_SPI3DEVICE_H
#define ADAFRUIT_SPI3DEVICE_H

#include <Adafruit_GenericDevice.h>
#include <Arduino.h>

/**
 * @brief Bit-banged 3-wire SPI device class
 *
 * Provides a GenericDevice interface for sensors that use 3-wire SPI
 * with a bidirectional data pin. Protocol:
 * - Write: CS low, clock out (reg & 0x7F), clock out data, CS high
 * - Read: CS low, clock out (reg | 0x80), clock in data, CS high
 */
class Adafruit_SPI3Device {
 public:
  Adafruit_SPI3Device(int8_t cs, int8_t clk, int8_t data,
                      uint32_t freq = 1000000);
  ~Adafruit_SPI3Device();
  bool begin();
  Adafruit_GenericDevice* getGenericDevice();

 private:
  int8_t _cs, _clk, _data;
  uint32_t _freq;
  uint8_t _bitdelay_us;
  Adafruit_GenericDevice* _generic_dev;

  void _clockOutByte(uint8_t b);
  uint8_t _clockInByte();

  static bool _readreg_cb(void* obj, uint8_t* addr_buf, uint8_t addrsiz,
                          uint8_t* data, uint16_t datalen);
  static bool _writereg_cb(void* obj, uint8_t* addr_buf, uint8_t addrsiz,
                           const uint8_t* data, uint16_t datalen);
};

#endif // ADAFRUIT_SPI3DEVICE_H
