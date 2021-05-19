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
#include <Arduino.h>
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
 * The ESP32 has 2 user-accessible SPI buses: HSPI and VSPI. The default `SPI`
 * instance uses the VSPI bus with default pins on (mosi=23, miso=19, clk=18,
 * ss=5). To use the HSPI bus with default pins on (mosi=13, miso=12, clk=14,
 * ss=15), make a copy this class, create a manual instance of `SPIClass SPI1 =
 * SPIClass(HSPI)` and replace every instance of `SPI` below with `SPI1`. (TODO:
 * Maybe `SPI` should be a template parameter.)
 */
class HardSpiInterface {
  public:
    HardSpiInterface(
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        mLatchPin(latchPin),
        mDataPin(dataPin),
        mClockPin(clockPin)
    {}

    void begin() const {
      pinMode(mLatchPin, OUTPUT);
      pinMode(mDataPin, OUTPUT);
      pinMode(mClockPin, OUTPUT);
    #if defined(ESP32)
      SPI.begin(mClockPin, -1 /*miso*/, mDataPin, mLatchPin);
    #else
      SPI.begin();
    #endif
    }

    void end() const {
      pinMode(mLatchPin, INPUT);
      pinMode(mDataPin, INPUT);
      pinMode(mClockPin, INPUT);
      SPI.end();
    }

    /** Send 8 bits, including latching LOW and HIGH. */
    void send8(uint8_t value) const {
      SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
      digitalWrite(mLatchPin, LOW);
      SPI.transfer(value);
      digitalWrite(mLatchPin, HIGH);
      SPI.endTransaction();
    }

    /** Send 16 bits, including latching LOW and HIGH. */
    void send16(uint16_t value) const {
      SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
      digitalWrite(mLatchPin, LOW);
      SPI.transfer16(value);
      digitalWrite(mLatchPin, HIGH);
      SPI.endTransaction();
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
    uint8_t const mLatchPin;
    uint8_t const mDataPin;
    uint8_t const mClockPin;
};

} // ace_segment

#endif
