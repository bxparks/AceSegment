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

#include "Hardware.h"
#include "SegmentDriver.h"
#include "Util.h"

namespace ace_segment {

void SegmentDriver::displayCurrentField() {
  if (mCurrentSegment != mPrevSegment) {
    writeSegmentPin(mPrevSegment, mSegmentOff);
  }

  DigitPatternType digitPattern = getDigitBitPattern(mCurrentSegment);
  if (digitPattern != mDigitPattern) {
    drawDigits(digitPattern);
    mDigitPattern = digitPattern;
  }
  if (mCurrentSegment != mPrevSegment) {
    writeSegmentPin(mCurrentSegment, mSegmentOn);
  }

  mPrevSegment = mCurrentSegment;
  Util::incrementMod(mCurrentSegment, kNumSegments);
}

Driver::DigitPatternType SegmentDriver::getDigitBitPattern(uint8_t segment) {
  SegmentPatternType segmentMask = (0x1 << segment);
  DigitPatternType digitMask = 0x1;
  DigitPatternType digitPattern = 0;
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    DimmingDigit& dimmingDigit = mDimmingDigits[digit];
    SegmentPatternType pattern = (dimmingDigit.brightness != 0) ? dimmingDigit.pattern : 0;
    if (pattern & segmentMask) {
      digitPattern |= digitMask;
    }
    digitMask <<= 1;
  }
  return digitPattern;
}

void SegmentDriver::drawDigits(DigitPatternType digitPattern) {
  DigitPatternType digitMask = 0x1;
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    uint8_t output = (digitPattern & digitMask) ? mDigitOn : mDigitOff;
    writeDigitPin(digit, output);
    digitMask <<= 1;
  }
}

}
