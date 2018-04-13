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

#ifndef ACE_SEGMENT_MODULATING_DIGIT_DRIVER_H
#define ACE_SEGMENT_MODULATING_DIGIT_DRIVER_H

#include <stdint.h>
#include "DigitDriver.h"

namespace ace_segment {

class DimmingDigit;

class ModulatingDigitDriver: public DigitDriver {
  public:
    /** Constructor. */
    explicit ModulatingDigitDriver(LedMatrix* ledMatrix,
            DimmingDigit* dimmingDigits, uint8_t numDigits,
            uint8_t numSubFields, bool ownsLedMatrix = false):
        DigitDriver(ledMatrix, dimmingDigits, numDigits, ownsLedMatrix),
        mNumSubFields(numSubFields)
    {}

    virtual void configure() override {
      DigitDriver::configure();
      mCurrentSubField = 0;
      mCurrentSubFieldMax = 0;
      mIsPrevDigitOn = true; // setting true forces it off on next interation
    }

    virtual uint16_t getFieldsPerFrame() override {
      return mNumDigits * mNumSubFields;
    }

    virtual bool isBrightnessSupported() override { return true; }

    virtual void displayCurrentField() override;

  protected:
    uint8_t const mNumSubFields;
    uint8_t mCurrentSubField;
    uint8_t mCurrentSubFieldMax;

    /** Whether the previous digit was turned on or off. */
    bool mIsPrevDigitOn;

  private:
    // disable copy-constructor and assignment operator
    ModulatingDigitDriver(const ModulatingDigitDriver&) = delete;
    ModulatingDigitDriver& operator=(const ModulatingDigitDriver&) = delete;
};

}

#endif
