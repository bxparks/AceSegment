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

#ifndef ACE_SEGMENT_MERGED_DIGIT_DRIVER_H
#define ACE_SEGMENT_MERGED_DIGIT_DRIVER_H

#include <stdint.h>
#include "LedMatrixMerged.h"
#include "Driver.h"

namespace ace_segment {

class DimmablePattern;

/**
 * A Driver that assumes that the resistors are on the segments so the
 * multiplexing occurs by scanning through the digits.
 */
class MergedDigitDriver: public Driver {
  public:
    /** Constructor. */
    explicit MergedDigitDriver(LedMatrixMerged* ledMatrix,
            DimmablePattern* dimmablePatterns, uint8_t numDigits,
            uint8_t numSubFields, bool ownsLedMatrix = false):
        Driver(ledMatrix, dimmablePatterns, numDigits, ownsLedMatrix),
        mNumSubFields(numSubFields)
    {}

    virtual void configure() override {
      Driver::configure();
      mCurrentDigit = 0;
    }

    virtual uint16_t getFieldsPerFrame() override {
      return mNumDigits * mNumSubFields;
    }

    virtual bool isBrightnessSupported() override { return mNumSubFields > 1; }

    virtual void displayCurrentField() override;

    virtual void prepareToSleep() override;

  protected:
    /** If this is greater than 1, use displayCurrentFieldModulated(). */
    uint8_t const mNumSubFields;

    /**
     * Within the displayCurrentField() method, mCurrentDigit is the current
     * digit that is being drawn. It is incremented to the next digit just
     * before returning from that method.
     */
    uint8_t mCurrentDigit;

  private:
    // disable copy-constructor and assignment operator
    MergedDigitDriver(const MergedDigitDriver&) = delete;
    MergedDigitDriver& operator=(const MergedDigitDriver&) = delete;
};

}

#endif
