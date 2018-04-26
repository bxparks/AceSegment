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
#include "StyledPattern.h"
#include "Hardware.h"
#include "Styler.h"
#include "Renderer.h"

// Set to 1 to use the reciprocal of pulse frames to avoid a long division
// calculation every frame.
#define ACE_SEGMENT_USE_INVERSE_PULSE_FRAMES 1

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

  // Reset the active styles.
  memset(mActiveStyles, 0, kNumStyles * sizeof(uint8_t));
  mActiveStyles[0] = mNumDigits;
}

void Renderer::writePatternAt(uint8_t digit, uint8_t pattern, uint8_t style) {
  if (digit >= mNumDigits) return;
  if (style >= kNumStyles) return;

  StyledPattern& styledPattern = mStyledPatterns[digit];
  styledPattern.pattern = pattern;

  mActiveStyles[styledPattern.style]--;
  mActiveStyles[style]++;
  styledPattern.style = style;
}

void Renderer::writePatternAt(uint8_t digit, uint8_t pattern) {
  if (digit >= mNumDigits) return;
  StyledPattern& styledPattern = mStyledPatterns[digit];
  styledPattern.pattern = pattern;
}

void Renderer::writeStyleAt(uint8_t digit, uint8_t style) {
  if (digit >= mNumDigits) return;
  if (style >= kNumStyles) return;

  StyledPattern& styledPattern = mStyledPatterns[digit];

  mActiveStyles[styledPattern.style]--;
  mActiveStyles[style]++;
  styledPattern.style = style;
}

void Renderer::writeDecimalPointAt(uint8_t digit, bool state) {
  if (digit >= mNumDigits) return;
  StyledPattern& styledPattern = mStyledPatterns[digit];
  if (state) {
    styledPattern.setDecimalPoint();
  } else {
    styledPattern.clearDecimalPoint();
  }
}
void Renderer::clear() {
  for (uint8_t i = 0; i < mNumDigits; i++) {
    mStyledPatterns[i].pattern = 0;
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
  updateStylers();
  renderStyledPatterns();
  if (mStatsResetInterval > 0 &&
      mStats.getCount() >= mStatsResetInterval) {
    mStats.reset();
  }
}

void Renderer::updateStylers() {
  // Update the active Stylers.
  for (uint8_t style = 1; style < kNumStyles; style++) {
    if (mActiveStyles[style] > 0) {
      Styler* styler = mStylers[style];
      if (isStylerSupported(styler)) {
        styler->calcForFrame();
      }
    }
  }
}

bool Renderer::isStylerSupported(Styler* styler) {
  if (styler == nullptr) return false;
  if (!styler->requiresBrightness()) return true;
  return mIsBrightnessEnabled;
}

TimingStats Renderer::getTimingStats() {
  noInterrupts();
  TimingStats stats = mStats;
  interrupts();
  return stats;
}

void Renderer::renderStyledPatterns() {
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    StyledPattern& styledPattern = mStyledPatterns[digit];

    uint8_t pattern = styledPattern.pattern;
    uint8_t brightness = mBrightness;

    uint8_t style = styledPattern.style;
    if (0 < style && style < kNumStyles) {
      Styler* styler = mStylers[style];
      if (isStylerSupported(styler)) {
        styler->apply(&pattern, &brightness);
      }
    }
    mDriver->setPattern(digit, pattern, brightness);
  }
}

}
