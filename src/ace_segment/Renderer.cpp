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
#include "Renderer.h"
#include "LedMatrix.h"

namespace ace_segment {

void Renderer::displayCurrentField() {
  if (mPreparedToSleep) return;

  if (mNumSubFields == 1) {
    displayCurrentFieldPlain();
  } else {
    displayCurrentFieldModulated();
  }
}

void Renderer::displayCurrentFieldPlain() {
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

void Renderer::displayCurrentFieldModulated() {
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
      (brightness == 255 || mCurrentSubField < mCurrentSubFieldMax)
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
