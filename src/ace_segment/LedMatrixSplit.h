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
#include "Hardware.h"

namespace ace_segment {

/**
 * An LedMatrix that writes to group pins separately from the element pins.
 */
class LedMatrixSplit: public LedMatrix {
  public:
    LedMatrixSplit(
        const Hardware* hardware,
        bool cathodeOnGroup,
        bool transistorsOnGroups,
        bool transistorsOnElements,
        uint8_t numGroups,
        const uint8_t* groupPins
    ) :
        LedMatrix(
            cathodeOnGroup,
            transistorsOnGroups,
            transistorsOnElements),
        mHardware(hardware),
        mNumGroups(numGroups),
        mGroupPins(groupPins)
    {}

    void begin() override {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware->pinMode(pin, OUTPUT);
        mHardware->digitalWrite(pin, mGroupOff);
      }
    }

    void end() override {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware->pinMode(pin, INPUT);
      }
    }

    void draw(uint8_t group, uint8_t elementPattern) override {
      if (group != mPrevGroup) {
        disableGroup(mPrevGroup);
      }

      drawElements(elementPattern);
      enableGroup(group);
      mPrevGroup = group;
    }

    void enableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOn);
      mPrevGroup = group;
    }

    void disableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOff);
      mPrevGroup = group;
    }

    void clear() override {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        disableGroup(group);
      }
      drawElements(0);
    }

  protected:
    virtual void drawElements(uint8_t pattern) = 0;

  private:
    /** Write to group pin identified by 'group'. */
    void writeGroupPin(uint8_t group, uint8_t output) {
      uint8_t groupPin = mGroupPins[group];
      mHardware->digitalWrite(groupPin, output);
    }

  protected:
    const Hardware* const mHardware;
    uint8_t const mNumGroups;
    const uint8_t* const mGroupPins;

    uint8_t mPrevGroup = 0;
};

}

#endif
