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

#ifndef ACE_SEGMENT_LED_MATRIX_DIRECT_H
#define ACE_SEGMENT_LED_MATRIX_DIRECT_H

#include "Hardware.h"
#include "LedMatrix.h"

namespace ace_segment {

class LedMatrixDirect: public LedMatrix {
  public:
    LedMatrixDirect(Hardware* hardware, bool cathodeOnGroup,
            bool useTransistorsOnGroups, bool useTransistorsOnElements,
            uint8_t numGroups, uint8_t numElements,
            const uint8_t* groupPins, const uint8_t* elementPins):
        LedMatrix(hardware, cathodeOnGroup, useTransistorsOnGroups,
            useTransistorsOnElements, numGroups, numElements),
        mGroupPins(groupPins),
        mElementPins(elementPins)
    {}

    virtual void configure() override;

    virtual void finish() override;

    virtual void enableGroup(uint8_t group) override;

    virtual void disableGroup(uint8_t group) override;

    virtual void drawElements(uint8_t pattern) override;

  private:
    /** Write to group pin identified by 'group'. */
    void writeGroupPin(uint8_t group, uint8_t output) {
      uint8_t groupPin = mGroupPins[group];
      mHardware->digitalWrite(groupPin, output);
    }

    /** Write to the element pin identified by 'element'. */
    void writeElementPin(uint8_t element, uint8_t output) {
      uint8_t elementPin = mElementPins[element];
      mHardware->digitalWrite(elementPin, output);
    }

    const uint8_t* const mGroupPins;
    const uint8_t* const mElementPins;
};

}

#endif
