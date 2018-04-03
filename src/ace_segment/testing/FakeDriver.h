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

#ifndef ACE_SEGMENT_FAKE_DRIVER_H
#define ACE_SEGMENT_FAKE_DRIVER_H

#include "../Driver.h"

namespace ace_segment {

namespace testing {

/**
 * A fake instance of Driver so that Renderer can be tested in isolation.
 * Acts kind of like a ModulatingDigitDriver.
 */
class FakeDriver: public Driver {
  public:
    /** Constructor. */
    explicit FakeDriver(Hardware* hardware, DimmingDigit* dimmingDigits,
            uint8_t numDigits):
        Driver(hardware, dimmingDigits, numDigits)
    {}

    virtual void displayCurrentField() {
      mIsDisplayCurrentFieldCalled = true;
    }

    virtual uint16_t getFieldsPerFrame() override {
      return (uint16_t) mNumSubFields * mNumDigits;
    }

    virtual bool isBrightnessSupported() override {
      return (mNumSubFields > 1);
    }

    void clear() {
      mIsDisplayCurrentFieldCalled = false;
      mIsGetFieldsPerSecondCalled = false;
    }

    void setNumSubFields(uint8_t numSubFields) {
      mNumSubFields = numSubFields;
    }

    bool mIsDisplayCurrentFieldCalled;
    bool mIsGetFieldsPerSecondCalled;
    uint8_t mNumSubFields;
};

}

}

#endif
