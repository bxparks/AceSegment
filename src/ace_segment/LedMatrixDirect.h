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
#include "LedMatrixSplit.h"

namespace ace_segment {

/**
 * An LedMatrix that whose group pins and element pins are wired directly to the
 * MCU.
 */
class LedMatrixDirect: public LedMatrixSplit {
  public:
    LedMatrixDirect(
        const Hardware* hardware,
        uint8_t groupActivePattern,
        uint8_t elementActivePattern,
        uint8_t numGroups,
        const uint8_t* groupPins,
        uint8_t numElements,
        const uint8_t* elementPins
    ) :
        LedMatrixSplit(
            hardware,
            groupActivePattern,
            elementActivePattern,
            numGroups,
            groupPins),
        mNumElements(numElements),
        mElementPins(elementPins)
    {}

    void begin() override {
      LedMatrixSplit::begin();

      for (uint8_t element = 0; element < mNumElements; element++) {
        uint8_t elementPin = mElementPins[element];
        mHardware->pinMode(elementPin, OUTPUT);
        mHardware->digitalWrite(elementPin, (0x00 ^ mElementXorMask) & 0x1);
      }
    }

    void end() override {
      LedMatrixSplit::end();

      for (uint8_t element = 0; element < mNumElements; element++) {
        uint8_t elementPin = mElementPins[element];
        mHardware->pinMode(elementPin, INPUT);
      }
    }

  protected:
    void drawElements(uint8_t pattern) override {
      for (uint8_t element = 0; element < mNumElements; element++) {
        uint8_t output = pattern /*& 0x1*/; // bit mask not needed
        writeElementPin(element, output);
        pattern >>= 1;
      }
    }

  private:
    /** Write to the element pin identified by 'element'. */
    void writeElementPin(uint8_t element, uint8_t output) {
      uint8_t elementPin = mElementPins[element];
      mHardware->digitalWrite(elementPin, (output ^ mElementXorMask) & 0x1);
    }

  private:
    uint8_t const mNumElements;
    const uint8_t* const mElementPins;
};

}

#endif
