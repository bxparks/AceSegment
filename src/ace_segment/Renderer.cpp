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
#include "Util.h"
#include "DimmablePattern.h"
#include "Hardware.h"
#include "Renderer.h"

namespace ace_segment {

void Renderer::configure() {
  uint16_t nowMicros = mHardware->micros();

  // Extract driver specific info.
  mIsBrightnessEnabled = mDriver->isBrightnessSupported();
  mFieldsPerFrame = mDriver->getFieldsPerFrame();

  // Counters for frames and fields.
  mCurrentField = 0;

  // Set up durations for polling.
  mMicrosPerField = 1000000UL / getFieldsPerSecond();
  mLastRenderFieldMicros = nowMicros;

  // Reset statistics
  mStats.reset();
}

void Renderer::writePatternAt(uint8_t digit, uint8_t pattern) {
  if (digit >= mNumDigits) return;
  DimmablePattern& dimmablePattern = mDimmablePatterns[digit];
  dimmablePattern.pattern = pattern;
  dimmablePattern.brightness = mBrightness;
}

void Renderer::writePatternAt(
    uint8_t digit, uint8_t pattern, uint8_t brightness) {
  if (digit >= mNumDigits) return;
  DimmablePattern& dimmablePattern = mDimmablePatterns[digit];
  dimmablePattern.pattern = pattern;
  dimmablePattern.brightness = brightness;
}

void Renderer::writeBrightnessAt(uint8_t digit, uint8_t brightness) {
  if (digit >= mNumDigits) return;
  DimmablePattern& dimmablePattern = mDimmablePatterns[digit];
  dimmablePattern.brightness = brightness;
}

void Renderer::writeDecimalPointAt(uint8_t digit, bool state) {
  if (digit >= mNumDigits) return;
  DimmablePattern& dimmablePattern = mDimmablePatterns[digit];
  if (state) {
    dimmablePattern.setDecimalPoint();
  } else {
    dimmablePattern.clearDecimalPoint();
  }
}
void Renderer::clear() {
  for (uint8_t i = 0; i < mNumDigits; i++) {
    mDimmablePatterns[i].pattern = 0;
    mDimmablePatterns[i].brightness = 0;
  }
}

bool Renderer::renderFieldWhenReady() {
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

void Renderer::renderField() {
  uint16_t now = mHardware->micros();
  if (mCurrentField == 0) {
    updateFrame();
  }
  mDriver->displayCurrentField();
  Util::incrementMod(mCurrentField, mFieldsPerFrame);

  uint16_t duration = mHardware->micros() - now;
  mStats.update(duration);
}

void Renderer::updateFrame() {
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    DimmablePattern& dimmablePattern = mDimmablePatterns[digit];
    mDriver->setPattern(
      digit, dimmablePattern.pattern, dimmablePattern.brightness);
  }

  if (mStatsResetInterval > 0 &&
      mStats.getCount() >= mStatsResetInterval) {
    mStats.reset();
  }
}

TimingStats Renderer::getTimingStats() {
  noInterrupts();
  TimingStats stats = mStats;
  interrupts();
  return stats;
}

}
