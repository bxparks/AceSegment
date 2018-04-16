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
  if (mPreparedToSleep) return;

  bool isCurrentDigitOn;
  DimmablePattern& dimmablePattern = mDimmablePatterns[mCurrentDigit];
  uint8_t brightness = dimmablePattern.brightness;
  if (mCurrentDigit != mPrevDigit) {
    // NOTE: The following could be optimized away by wrapping it around an 'if
    // (mIsPrevDigitOn)' statement. But I think it's safer to issue a redundant
    // disable-digit command just in case some else (interrupts or other
    // unpredicatble things) causes the mIsPrevDigitOn state to be wrong. We
    // want to be absolutely sure that 2 digits cannot be turned on at the same
    // time.
    mLedMatrix->disableGroup(mPrevDigit);

    isCurrentDigitOn = false;
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  } else {
    isCurrentDigitOn = mIsPrevDigitOn;
  }

  // No matter how small the mNumSubFields, we want:
  // * If brightness == 0, then subfield 0 should be OFF.
  // * If birghtness == 255, then special case that to be ON.
  if (brightness < 255 && mCurrentSubField >= mCurrentSubFieldMax) {
    // turn off the current digit
    if (isCurrentDigitOn) {
      mLedMatrix->disableGroup(mCurrentDigit);
      isCurrentDigitOn = false;
    }
  } else {
    // turn on the current digit
    if (!isCurrentDigitOn) {
      SegmentPatternType segmentPattern = dimmablePattern.pattern;
      if (segmentPattern != mSegmentPattern) {
        mLedMatrix->drawElements(segmentPattern);
        mSegmentPattern = segmentPattern;
      }
      mLedMatrix->enableGroup(mCurrentDigit);
      isCurrentDigitOn = true;
    }
  }

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  mIsPrevDigitOn = isCurrentDigitOn;
  if (mCurrentSubField >= mNumSubFields) {
    Util::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }
}

}
