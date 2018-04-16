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
#include "StyledPattern.h"
#include "Hardware.h"
#include "Renderer.h"
#include "Util.h"

// Set to 1 to use the reciprocal of pulse frames to avoid a long division
// calculation every frame.
#define ACE_SEGMENT_USE_INVERSE_PULSE_FRAMES 1

namespace ace_segment {

void Renderer::configure() {
  uint16_t nowMicros = mHardware->micros();

  // Extract driver specific info.
  mIsPulseEnabled = mDriver->isBrightnessSupported();
  mFieldsPerFrame = mDriver->getFieldsPerFrame();

  // Counters for frames and fields.
  mCurrentField = 0;

  // Set up durations for polling.
  mMicrosPerField = 1000000UL / getFieldsPerSecond();
  mLastRenderFieldMicros = nowMicros;

  // Reset statistics
  mStats.reset();

  // Set up for blinking slow.
  mFramesPerBlinkSlow = (uint32_t) mFramesPerSecond
      * mBlinkSlowDurationMillis / 1000;
  mCurrentBlinkSlowFrame = 0;

  // Set up for blinking fast.
  mFramesPerBlinkFast = (uint32_t) mFramesPerSecond
      * mBlinkFastDurationMillis / 1000;
  mCurrentBlinkFastFrame = 0;

  // Set up for pulsing slow.
  mFramesPerPulseSlow = (uint32_t) mFramesPerSecond
      * mPulseSlowDurationMillis / 1000;
  mFramesPerPulseSlowInverse = (uint32_t) 65536 * 1000
      / mFramesPerSecond / mPulseSlowDurationMillis;
  mCurrentPulseSlowFrame = 0;

  // Set up for pulsing fast.
  mFramesPerPulseFast = (uint32_t) mFramesPerSecond
      * mPulseFastDurationMillis / 1000;
  mFramesPerPulseFastInverse = (uint32_t) 65536 * 1000
      / mFramesPerSecond / mPulseFastDurationMillis;
  mCurrentPulseFastFrame = 0;
}

void Renderer::writePatternAt(uint8_t digit, uint8_t pattern, uint8_t style) {
  if (digit >= mNumDigits) return;
  StyledPattern& styledPattern = mStyledPatterns[digit];
  styledPattern.pattern = pattern;
  styledPattern.style = style;
}

void Renderer::writePatternAt(uint8_t digit, uint8_t pattern) {
  if (digit >= mNumDigits) return;
  StyledPattern& styledPattern = mStyledPatterns[digit];
  styledPattern.pattern = pattern;
}

void Renderer::writeStyleAt(uint8_t digit, uint8_t style) {
  if (digit >= mNumDigits) return;
  StyledPattern& styledPattern = mStyledPatterns[digit];
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

void Renderer::renderFieldWhenReady() {
  uint16_t now = mHardware->micros();
  uint16_t elapsedMicros = now - mLastRenderFieldMicros;
  if (elapsedMicros >= mMicrosPerField) {
    renderField();
    mLastRenderFieldMicros = now;
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
  calcBlinkAndPulseForFrame();
  renderStyledPatterns();
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

void Renderer::renderStyledPatterns() {
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    StyledPattern& styledPattern = mStyledPatterns[digit];
    StyledPattern::StyleType style = styledPattern.style;
    uint8_t brightness = calcBrightness(style, mBrightness,
        mBlinkSlowState, mBlinkFastState, mIsPulseEnabled,
        mPulseSlowFraction, mPulseFastFraction);
    mDriver->setPattern(digit, styledPattern.pattern, brightness);
  }
}

uint8_t Renderer::calcBrightness(uint8_t style, uint8_t brightness,
    uint8_t blinkSlowState, uint8_t blinkFastState, bool isPulseEnabled,
    uint8_t pulseSlowFraction, uint8_t pulseFastFraction) {

  switch (style) {
    case StyledPattern::kStyleNormal:
      return brightness;
    case StyledPattern::kStyleBlinkSlow:
      return (blinkSlowState == kBlinkStateOff) ? 0 : brightness;
    case StyledPattern::kStyleBlinkFast:
      return (blinkFastState == kBlinkStateOff) ? 0 : brightness;
    case StyledPattern::kStylePulseSlow:
      if (isPulseEnabled) {
        return ((uint16_t) pulseSlowFraction * brightness) / 256;
      } else {
        return brightness;
      }
    case StyledPattern::kStylePulseFast:
      if (isPulseEnabled) {
        return ((uint16_t) pulseFastFraction * brightness) / 256;
      } else {
        return brightness;
      }
    default:
      return brightness;
  }
}

void Renderer::calcBlinkAndPulseForFrame() {
  calcBlinkStateForFrame(mFramesPerBlinkSlow, mCurrentBlinkSlowFrame,
      mBlinkSlowState);
  calcBlinkStateForFrame(mFramesPerBlinkFast, mCurrentBlinkFastFrame,
      mBlinkFastState);

  if (mIsPulseEnabled) {
#if ACE_SEGMENT_USE_INVERSE_PULSE_FRAMES == 1
    calcPulseFractionForFrameUsingInverse(mFramesPerPulseSlowInverse,
        mFramesPerPulseSlow, mCurrentPulseSlowFrame, mPulseSlowFraction);
    calcPulseFractionForFrameUsingInverse(mFramesPerPulseFastInverse,
        mFramesPerPulseFast, mCurrentPulseFastFrame, mPulseFastFraction);
#else
    calcPulseFractionForFrame(mFramesPerPulseSlow, mCurrentPulseSlowFrame,
        mPulseSlowFraction);
    calcPulseFractionForFrame(mFramesPerPulseFast, mCurrentPulseFastFrame,
        mPulseFastFraction);
#endif
  }
}

void Renderer::calcBlinkStateForFrame(uint16_t framesPerBlink,
    uint16_t& currentFrame, uint8_t& blinkState) {
  uint16_t middleOfBlink = framesPerBlink / 2;
  if (currentFrame < middleOfBlink) {
    blinkState = kBlinkStateOn;
  } else {
    blinkState = kBlinkStateOff;
  }
  Util::incrementMod(currentFrame, framesPerBlink);
}

void Renderer::calcPulseFractionForFrame(uint16_t framesPerPulse,
    uint16_t& currentFrame, uint8_t& pulseFraction) {
  uint16_t middleOfPulse = framesPerPulse / 2;
  uint16_t fraction;
  if (currentFrame < middleOfPulse) {
    // TODO: rewrite to avoid expensive division operation
    fraction = 256 * (uint32_t) currentFrame / middleOfPulse;
  } else if (currentFrame < framesPerPulse) {
    uint16_t reverse = (framesPerPulse - currentFrame - 1);
    // TODO: rewrite to avoid expensive division operation
    fraction = 256 * (uint32_t) reverse / middleOfPulse;
  } else {
    fraction = 0;
  }
  if (fraction > 255) fraction = 255;
  pulseFraction = fraction;
  Util::incrementMod(currentFrame, framesPerPulse);
}

void Renderer::calcPulseFractionForFrameUsingInverse(
    uint16_t framesPerPulseInverse, uint16_t framesPerPulse,
    uint16_t& currentFrame, uint8_t& pulseFraction) {
  uint16_t middleOfPulse = framesPerPulse / 2;
  uint16_t fraction;
  if (currentFrame < middleOfPulse) {
    fraction = (uint32_t) framesPerPulseInverse * currentFrame / (256/2);
  } else if (currentFrame < framesPerPulse) {
    uint16_t reverse = (framesPerPulse - currentFrame - 1);
    fraction = (uint32_t) framesPerPulseInverse * reverse / (256/2);
  } else {
    fraction = 0;
  }
  if (fraction > 255) fraction = 255;
  pulseFraction = fraction;
  Util::incrementMod(currentFrame, framesPerPulse);
}

}
