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

#include "Util.h"
#include "Hardware.h"
#include "LedMatrix.h"
#include "ModulatingDigitDriver.h"

namespace ace_segment {

void ModulatingDigitDriver::displayCurrentField() {
  DimmingDigit& dimmingDigit = mDimmingDigits[mCurrentDigit];
  uint8_t brightness = dimmingDigit.brightness;
  if (mCurrentDigit != mPrevDigit) {
    mLedMatrix->disableGroup(mPrevDigit);
    mIsCurrentDigitOn = false;
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  }

  // No matter how small the mNumSubFields, we want:
  // * If brightness == 0, then subfield 0 should be OFF.
  // * If birghtness == 255, then special case that to be ON.
  if (brightness < 255 && mCurrentSubField >= mCurrentSubFieldMax) {
    // turn off
    if (mIsCurrentDigitOn) {
      mLedMatrix->disableGroup(mCurrentDigit);
      mIsCurrentDigitOn = false;
    }
  } else {
    // turn on
    if (!mIsCurrentDigitOn) {
      SegmentPatternType segmentPattern = dimmingDigit.pattern;
      if (segmentPattern != mSegmentPattern) {
        mLedMatrix->drawElements(segmentPattern);
        mSegmentPattern = segmentPattern;
      }
      mLedMatrix->enableGroup(mCurrentDigit);
      mIsCurrentDigitOn = true;
    }
  }

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  if (mCurrentSubField >= mNumSubFields) {
    Util::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }
}

}
