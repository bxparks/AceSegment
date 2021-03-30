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

#include <AceCommon.h> // incrementMod()
#include "Hardware.h"
#include "SplitSegmentDriver.h"
#include "LedMatrixSplit.h"

namespace ace_segment {

void SplitSegmentDriver::displayCurrentField() {
  if (mPreparedToSleep) return;
  LedMatrixSplit* ledMatrix = static_cast<LedMatrixSplit*>(mLedMatrix);

  if (mCurrentSegment != mPrevSegment) {
    ledMatrix->disableGroup(mPrevSegment);
  }

  DigitPatternType digitPattern = getDigitBitPattern(mCurrentSegment);
  if (digitPattern != mDigitPattern) {
    ledMatrix->drawElements(digitPattern);
    mDigitPattern = digitPattern;
  }
  if (mCurrentSegment != mPrevSegment) {
    ledMatrix->enableGroup(mCurrentSegment);
  }

  mPrevSegment = mCurrentSegment;
  ace_common::incrementMod(mCurrentSegment, kNumSegments);
}

Driver::DigitPatternType SplitSegmentDriver::getDigitBitPattern(
    uint8_t segment) {
  SegmentPatternType segmentMask = (0x1 << segment);
  DigitPatternType digitMask = 0x1;
  DigitPatternType digitPattern = 0;
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    const DimmablePattern& dimmablePattern = mDimmablePatterns[digit];
    SegmentPatternType pattern = (dimmablePattern.brightness != 0)
        ? dimmablePattern.pattern : 0;
    if (pattern & segmentMask) {
      digitPattern |= digitMask;
    }
    digitMask <<= 1;
  }
  return digitPattern;
}

void SplitSegmentDriver::prepareToSleep() {
  Driver::prepareToSleep();
  // TODO: Should we just remove this if-statement? Like
  // SplitDigitDriver::prepareToSleep?
  if (mCurrentSegment != mPrevSegment) {
    LedMatrixSplit* ledMatrix = static_cast<LedMatrixSplit*>(mLedMatrix);
    ledMatrix->disableGroup(mPrevSegment);
  }
}

}
