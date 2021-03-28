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

#ifndef ACE_SEGMENT_SEGMENT_DRIVER_H
#define ACE_SEGMENT_SEGMENT_DRIVER_H

#include <stdint.h>
#include "LedMatrixSplit.h"
#include "Driver.h"

namespace ace_segment {

class DimmablePattern;

/**
 * A Driver that assumes that the resistors are on the digits so the
 * multiplexing occurs by scanning through the segments.
 *
 * This was an interesting intellectual exercise, but multiplexing across the
 * segments is not very useful. But it does increase code complexity so I have
 * removed it from the main code base and archived it.
 */
// TODO: use mNumSegments, instead of relying on kNumSegments
class SplitSegmentDriver: public Driver {
  public:
    /**
     * Constructor. Note that subfields cannot be supported by this class
     * because the 'group' spans across multiple digit segments, so we cannot
     * control the brightness of just a single digit.
     *
     * @param ledMatrix instance of LedMatrix that understanding the wiring
     * @param dimmablePatterns array of digit patterns
     * @param numDigits number of dimmablePatterns
     */
    explicit SplitSegmentDriver(LedMatrixSplit* ledMatrix,
            DimmablePattern* dimmablePatterns, uint8_t numDigits):
        Driver(ledMatrix, dimmablePatterns, numDigits)
    {}

    void configure() override {
      Driver::configure();
      mCurrentSegment = 0;
      mPrevSegment = kNumSegments - 1;
      mDigitPattern = 0;
    }

    uint16_t getFieldsPerFrame() override { return kNumSegments; }

    bool isBrightnessSupported() override { return false; }

    void displayCurrentField() override;

    void prepareToSleep() override;

  private:
    // disable copy-constructor and assignment operator
    SplitSegmentDriver(const SplitSegmentDriver&) = delete;
    SplitSegmentDriver& operator=(const SplitSegmentDriver&) = delete;

    /** Get the digit bit pattern for the given segment. */
    DigitPatternType getDigitBitPattern(uint8_t segment);

    /**
     * Within the displayCurrentField() method, mCurrentSegment is the current
     * segment that is being drawn. It is incremented to the next segment just
     * before returning from that method.
     */
    uint8_t mCurrentSegment;

    /**
     * Within the displayCurrentField() method, the mPrevSegment is the segment
     * that was displayed on the previous call to displayCurrentField(). It is
     * set to the segment that was just displayed before returning.
     */
    uint8_t mPrevSegment;

    DigitPatternType mDigitPattern;
};

}

#endif
