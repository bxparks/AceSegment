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

#ifndef ACE_SEGMENT_HW_SPI_ADAPTER_H
#define ACE_SEGMENT_HW_SPI_ADAPTER_H

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>

namespace ace_segment {

/** Hardware SPI. */
class HwSpiAdapter : public SpiAdapter {
  public:
    HwSpiAdapter(
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
      SPI.begin();
    }

    void spiEnd() const override {
      pinMode(mLatchPin, INPUT);
      pinMode(mDataPin, INPUT);
      pinMode(mClockPin, INPUT);
      SPI.end();
    }

    void spiTransfer(uint8_t value) const override {
      digitalWrite(mLatchPin, LOW);
      SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
      SPI.transfer(value);
      SPI.endTransaction();
      digitalWrite(mLatchPin, HIGH);
    }

    void spiTransfer16(uint16_t value) const override {
      digitalWrite(mLatchPin, LOW);
      SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
      SPI.transfer16(value);
      SPI.endTransaction();
      digitalWrite(mLatchPin, HIGH);
    }

  private:
    uint8_t const mLatchPin;
    uint8_t const mDataPin;
    uint8_t const mClockPin;
};

} // ace_segment

#endif
