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

#include <Arduino.h>
#include <AceCommon.h> // incrementMod(), TimingStats
#include "Hardware.h"
#include "SegmentDisplay.h"

namespace ace_segment {

bool SegmentDisplay::renderFieldWhenReady() {
  uint16_t now = mHardware->micros();
  uint16_t elapsedMicros = now - mLastRenderFieldMicros;
  if (elapsedMicros >= mMicrosPerField) {
    renderField();
    mLastRenderFieldMicros = now;
    return true;
  } else {
    return false;
  }
}

void SegmentDisplay::renderField() {
  uint16_t now = mHardware->micros();
  displayCurrentField();
  uint16_t duration = mHardware->micros() - now;
  if (mTimingStats) mTimingStats->update(duration);
}

void SegmentDisplay::displayCurrentFieldPlain() {
  const uint8_t brightness = (mBrightnesses)
      ? mBrightnesses[mCurrentDigit]
      : 0xFF;
  if (brightness == 0) {
    mLedMatrix->disableGroup(mCurrentDigit);
  } else {
    const uint8_t pattern = mPatterns[mCurrentDigit];
    mLedMatrix->draw(mCurrentDigit, pattern);
  }

  mPrevDigit = mCurrentDigit;
  ace_common::incrementMod(mCurrentDigit, mNumDigits);
}

void SegmentDisplay::displayCurrentFieldModulated() {
  // Calculate the maximum subfield duration for a given digit.
  const uint8_t brightness = (mBrightnesses)
      ? mBrightnesses[mCurrentDigit]
      : 0xFF;
  if (mCurrentDigit != mPrevDigit) {
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  }

  // No matter how small the mNumSubFields, we want:
  // * If brightness == 0, then subfield 0 should be OFF.
  // * If brightness == 255, then special case that to be ON.
  const uint8_t pattern =
      (brightness == 0xFF || mCurrentSubField < mCurrentSubFieldMax)
      ? mPatterns[mCurrentDigit]
      : 0;

  if (pattern != mPattern || mCurrentDigit != mPrevDigit) {
    mLedMatrix->draw(mCurrentDigit, pattern);
    mPattern = pattern;
  }

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  if (mCurrentSubField >= mNumSubFields) {
    ace_common::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }
}

}
