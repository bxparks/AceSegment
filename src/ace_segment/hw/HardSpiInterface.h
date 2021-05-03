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
      SPI.begin();
    }

    void end() const {
      pinMode(mLatchPin, INPUT);
      pinMode(mDataPin, INPUT);
      pinMode(mClockPin, INPUT);
      SPI.end();
    }

    /** Send 8 bits, including latching LOW and HIGH. */
    void send8(uint8_t value) const {
      digitalWrite(mLatchPin, LOW);
      SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
      SPI.transfer(value);
      SPI.endTransaction();
      digitalWrite(mLatchPin, HIGH);
    }

    /** Send 16 bits, including latching LOW and HIGH. */
    void send16(uint16_t value) const {
      digitalWrite(mLatchPin, LOW);
      SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
      SPI.transfer16(value);
      SPI.endTransaction();
      digitalWrite(mLatchPin, HIGH);
    }

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
