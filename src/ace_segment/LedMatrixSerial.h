/*
MIT License

Copyright (c) 2018 Brian T. Park

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

#ifndef ACE_SEGMENT_LED_MATRIX_SERIAL_H
#define ACE_SEGMENT_LED_MATRIX_SERIAL_H

#include "Hardware.h"
#include "LedMatrix.h"

namespace ace_segment {

class LedMatrixSerial: public LedMatrix {
  public:
    LedMatrixSerial(Hardware* hardware, uint8_t numGroups, uint8_t numElements):
        LedMatrix(hardware, numGroups, numElements)
    {}

    void setGroupPins(const uint8_t* groupPins) {
      mGroupPins = groupPins;
    }

    void setElementPins(uint8_t latchPin, uint8_t dataPin, uint8_t clockPin) {
      mLatchPin = latchPin;
      mDataPin = dataPin;
      mClockPin = clockPin;
    }

    virtual void configure() override {
      LedMatrix::configure();

      // TODO: Do I need to set the initial values of the 74HC595?
      mHardware->pinMode(mLatchPin, OUTPUT);
      mHardware->pinMode(mDataPin, OUTPUT);
      mHardware->pinMode(mClockPin, OUTPUT);

      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware->pinMode(pin, OUTPUT);
        mHardware->digitalWrite(pin, mGroupOff);
      }
    }

    virtual void enableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOn);
    }

    virtual void disableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOff);
    }

    virtual void drawElements(uint8_t pattern) override {
      mHardware->digitalWrite(mLatchPin, LOW);
      uint8_t actualPattern = (mElementOn == HIGH) ? pattern : ~pattern;
      mHardware->shiftOut(mDataPin, mClockPin, MSBFIRST, actualPattern);
      mHardware->digitalWrite(mLatchPin, HIGH);
    }

  protected:
    /** Write to group pin identified by 'group'. VisibleForTesting. */
    void writeGroupPin(uint8_t group, uint8_t output) {
      uint8_t groupPin = mGroupPins[group];
      mHardware->digitalWrite(groupPin, output);
    }

    const uint8_t* mGroupPins;
    uint8_t mLatchPin;
    uint8_t mDataPin;
    uint8_t mClockPin;
};

}
#endif
