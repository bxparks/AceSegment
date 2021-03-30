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

void SegmentDisplay::begin() {
  mFieldsPerFrame = mRenderer->getFieldsPerFrame();

  // Counters for frames and fields.
  mCurrentField = 0;

  // Set up durations for polling.
  mMicrosPerField = 1000000UL / getFieldsPerSecond();
  mLastRenderFieldMicros = mHardware->micros();
}

void SegmentDisplay::writeDecimalPointAt(uint8_t digit, bool state) {
  if (digit >= mNumDigits) return;
  uint8_t pattern = mPatterns[digit];
  if (state) {
    pattern |= 0x80;
  } else {
    pattern &= ~0x80;
  }
  mPatterns[digit] = pattern;
}

void SegmentDisplay::clear() {
  for (uint8_t i = 0; i < mNumDigits; i++) {
    mPatterns[i] = 0;
  }
}

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
  mRenderer->displayCurrentField();
  ace_common::incrementMod(mCurrentField, mFieldsPerFrame);

  uint16_t duration = mHardware->micros() - now;
  if (mTimingStats) mTimingStats->update(duration);
}

}
