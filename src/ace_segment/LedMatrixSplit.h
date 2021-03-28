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

#ifndef ACE_SEGMENT_LED_MATRIX_SPLIT_H
#define ACE_SEGMENT_LED_MATRIX_SPLIT_H

#include "LedMatrix.h"

namespace ace_segment {

/**
 * An LedMatrix that writes to group pins separately from the element pins.
 */
class LedMatrixSplit: public LedMatrix {
  public:
    LedMatrixSplit(
        bool cathodeOnGroup,
        bool transistorsOnGroups,
        bool transistorsOnElements,
        uint8_t numGroups,
        uint8_t numElements,
        const uint8_t* groupPins
    ) :
        LedMatrix(
            cathodeOnGroup,
            transistorsOnGroups,
            transistorsOnElements,
            numGroups,
            numElements),
        mGroupPins(groupPins)
    {}

    virtual void configure() {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware->pinMode(pin, OUTPUT);
        mHardware->digitalWrite(pin, mGroupOff);
      }
    }

    virtual void finish() {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware->pinMode(pin, INPUT);
      }
    }

    void LedMatrixSplitSerial::enableGroup(uint8_t group) {
      writeGroupPin(group, mGroupOn);
    }

    void LedMatrixSplitSerial::disableGroup(uint8_t group) {
      writeGroupPin(group, mGroupOff);
    }

    virtual void drawElements(uint8_t pattern) = 0;

  protected:
    /** Write to group pin identified by 'group'. */
    void writeGroupPin(uint8_t group, uint8_t output) {
      uint8_t groupPin = mGroupPins[group];
      mHardware->digitalWrite(groupPin, output);
    }

  protected:
    const uint8_t* const mGroupPins;
};

}

#endif
