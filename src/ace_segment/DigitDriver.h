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

#ifndef ACE_SEGMENT_DIGIT_DRIVER_H
#define ACE_SEGMENT_DIGIT_DRIVER_H

#include <stdint.h>
#include "DirectDriver.h"

namespace ace_segment {

class DimmingDigit;
class Hardware;

class DigitDriver: public DirectDriver {
  public:
    /** Constructor. */
    explicit DigitDriver(Hardware* hardware, DimmingDigit* dimmingDigits,
            int8_t numDigits):
        DirectDriver(hardware, dimmingDigits, numDigits)
    {}

    virtual void configure() override {
      DirectDriver::configure();

      mCurrentDigit = 0;
      mPrevDigit = mNumDigits - 1;
      mSegmentPattern = 0;
    }

    virtual uint16_t getFieldsPerFrame() override { return mNumDigits; }

    virtual bool isBrightnessSupported() override { return false; }

    virtual void displayCurrentField() override;

  protected:
    // disable copy-constructor and assignment operator
    DigitDriver(const DigitDriver&) = delete;
    DigitDriver& operator=(const DigitDriver&) = delete;

    /** Draw the specified segments. */
    void drawSegments(SegmentPatternType segmentPattern);

    uint8_t mCurrentDigit;
    uint8_t mPrevDigit;
    bool mIsCurrentDigitOn; // whether the current digit is on or off
    SegmentPatternType mSegmentPattern;
};

}

#endif
