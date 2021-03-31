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
 *
 * @tparam H class that provides access to hardware pins and timing functions
 */
template<typename H>
class LedMatrixSplit : public LedMatrix {
  public:
    LedMatrixSplit(
        const H& hardware,
        uint8_t groupOnPattern,
        uint8_t elementOnPattern,
        uint8_t numGroups,
        const uint8_t* groupPins
    ) :
        LedMatrix(groupOnPattern, elementOnPattern),
        mHardware(hardware),
        mGroupPins(groupPins),
        mNumGroups(numGroups)
    {}

    void begin() override {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware.pinMode(pin, OUTPUT);
        mHardware.digitalWrite(pin, (0x00 ^ mGroupXorMask) & 0x1);
      }
    }

    void end() override {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware.pinMode(pin, INPUT);
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
      writeGroupPin(group, 0x1);
      mPrevGroup = group;
    }

    void disableGroup(uint8_t group) override {
      writeGroupPin(group, 0x0);
      mPrevGroup = group;
    }

    void clear() override {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        disableGroup(group);
      }
      drawElements(0);
    }

  private:
    /** Write to group pin identified by 'group'. */
    void writeGroupPin(uint8_t group, uint8_t output) {
      uint8_t groupPin = mGroupPins[group];
      mHardware.digitalWrite(groupPin, (output ^ mGroupXorMask) & 0x1);
    }

  protected:
    // Arranged to save space on 32-bit processors.
    const H& mHardware;
    const uint8_t* const mGroupPins;
    uint8_t const mNumGroups;

    /**
     * Remember the previous group so that we can turn it off after
     * moving to the new group.
     */
    uint8_t mPrevGroup = 0;
};

}

#endif
