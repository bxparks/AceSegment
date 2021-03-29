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

#ifndef ACE_SEGMENT_SW_SPI_ADAPTER_H
#define ACE_SEGMENT_SW_SPI_ADAPTER_H

#include <stdint.h>
#include <Arduino.h>

namespace ace_segment {

/** Software SPI using shiftOut(). */
class SwSpiAdapter : public SpiAdapter {
  public:
    SwSpiAdapter(
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        mLatchPin(latchPin),
        mDataPin(dataPin),
        mClockPin(clockPin)
    {}

    void spiBegin() const override {
      pinMode(mLatchPin, OUTPUT);
      pinMode(mDataPin, OUTPUT);
      pinMode(mClockPin, OUTPUT);
    }

    void spiEnd() const override {
      pinMode(mLatchPin, INPUT);
      pinMode(mDataPin, INPUT);
      pinMode(mClockPin, INPUT);
    }

    void spiTransfer(uint8_t value) const override {
      digitalWrite(mLatchPin, LOW);
      shiftOut(mDataPin, mClockPin, MSBFIRST, value);
      digitalWrite(mLatchPin, HIGH);
    }

    void spiTransfer16(uint16_t value) const override {
      uint8_t msb = (value & 0xff00) >> 8;
      uint8_t lsb = (value & 0xff);
      digitalWrite(mLatchPin, LOW);
      shiftOut(mDataPin, mClockPin, MSBFIRST, msb);
      shiftOut(mDataPin, mClockPin, MSBFIRST, lsb);
      digitalWrite(mLatchPin, HIGH);
    }

  private:
    uint8_t const mLatchPin;
    uint8_t const mDataPin;
    uint8_t const mClockPin;
};

} // ace_segment

#endif
