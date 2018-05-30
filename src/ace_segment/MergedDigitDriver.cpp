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
#include "MergedDigitDriver.h"
#include "LedMatrixMerged.h"
#include "Util.h"

namespace ace_segment {

// TODO: add code to perform PWM
void MergedDigitDriver::displayCurrentField() {
  if (mPreparedToSleep) return;

  DigitPatternType digitPattern = 0x0;
  SegmentPatternType segmentPattern = 0x0;

  const DimmablePattern& dimmablePattern = mDimmablePatterns[mCurrentDigit];
  if (dimmablePattern.brightness != 0) {
    segmentPattern = dimmablePattern.pattern;
    digitPattern = 0x1 << mCurrentDigit;
  }

  LedMatrixMerged* ledMatrix = static_cast<LedMatrixMerged*>(mLedMatrix);
  ledMatrix->draw(digitPattern, segmentPattern);

  Util::incrementMod(mCurrentDigit, mNumDigits);
}

void MergedDigitDriver::prepareToSleep() {
  Driver::prepareToSleep();
  LedMatrixMerged* ledMatrix = static_cast<LedMatrixMerged*>(mLedMatrix);
  ledMatrix->draw(0, 0);
}

}
