/*
MIT License

Copyright (c) 2021 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ACE_SEGMENT_HARD_SPI_INTERFACE_H
#define ACE_SEGMENT_HARD_SPI_INTERFACE_H

#include <stdint.h>
#include <Arduino.h> // digitalWrite()
#include <SPI.h>

namespace ace_segment {

/**
 * Hardware SPI interface to talk to one or two 74HC595 Shift Register chip(s),
 * using the predefined `SPI` global instance. This is currently not meant to be
 * general-purpose SPI interface. For different SPI configurations, it is
 * probably easiest to just copy this file, make the necessary changes, then
 * substitute the new class in places where this class is used.
 *
 * The maximum speed of MAX7219 is 16MHz so this class sets the SPI speed to
 * 8MHz. It's not clear if the SPI speed is worth making into a configurable
 * parameter. Such a change needs to done a bit carefully, because it should
 * be a template parameter so that `SPISettings` is a compile-time constant
 * which allows compile-time optimizations to happen.
 *
 * The ESP32 has 2 user-accessible SPI buses (HSPI and VSPI), and so does the
 * STM32F1 (SPI1 and SPI2). By default, the predefined SPI instance is used, but
 * a user-defined secondary SPI instance can be passed into the constructor.
 *
 * @tparam T_SPI the class of the hardware SPI instance, usually SPIClass
 */
template <typename T_SPI>
class HardSpiInterface {
  private:
    // The following constants are defined without including <SPI.h> to avoid
    // pulling in the global SPI instance into applications which don't use SPI.
    // They may become template parameters in the future.

    /** MAX7219 has a maximum clock of 16 MHz, so set this to 8 MHz. */
    static const uint32_t kClockSpeed = 8000000;

    /** MSB first or LSB first */
  #if defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_ARCH_SAMD)
    static const BitOrder kBitOrder = MSBFIRST;
  #else
    static const uint8_t kBitOrder = MSBFIRST;
  #endif

    /** SPI mode */
    static const uint8_t kSpiMode = SPI_MODE0;

  public:
    HardSpiInterface(T_SPI& spi, uint8_t latchPin) :
        mSpi(spi),
        mLatchPin(latchPin)
    {}

    /**
     * Initialize the HardSpiInterface. The hardware SPI object must be
     * initialized using `SPI.begin()` as well.
     */
    void begin() const {
      // To use Hardware SPI on ESP8266, we must set the SCK and MOSI pins to
      // 'SPECIAL' instead of 'OUTPUT'. This is performed by calling
      // SPI.begin(). Also, unlike other Arduino platforms, the SPIClass on
      // the ESP8266 defaults to controlling the SS/CS pin itself, instead of
      // letting the application code control it. The setHwCs(false) let's
      // HardSpiInterface control the CS/SS pin.
      // https://www.esp8266.com/wiki/doku.php?id=esp8266_gpio_pin_allocations
      #if defined(ESP8266)
        mSpi.setHwCs(false);
      #endif

      pinMode(mLatchPin, OUTPUT);
    }

    void end() const {
      pinMode(mLatchPin, INPUT);
    }

    /** Send 8 bits, including latching LOW and HIGH. */
    void send8(uint8_t value) const {
      mSpi.beginTransaction(SPISettings(kClockSpeed, kBitOrder, kSpiMode));
      digitalWrite(mLatchPin, LOW);
      mSpi.transfer(value);
      digitalWrite(mLatchPin, HIGH);
      mSpi.endTransaction();
    }

    /** Send 16 bits, including latching LOW and HIGH. */
    void send16(uint16_t value) const {
      mSpi.beginTransaction(SPISettings(kClockSpeed, kBitOrder, kSpiMode));
      digitalWrite(mLatchPin, LOW);
      mSpi.transfer16(value);
      digitalWrite(mLatchPin, HIGH);
      mSpi.endTransaction();
    }

    /**
     * Send two 8-bit bytes as a single 16-bit stream, including latching LOW
     * and HIGH.
     */
    void send16(uint8_t msb, uint8_t lsb) const {
      uint16_t value = ((uint16_t) msb) << 8 | (uint16_t) lsb;
      send16(value);
    }

  private:
    T_SPI& mSpi;
    uint8_t const mLatchPin;
};

} // ace_segment

#endif
