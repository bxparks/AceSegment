#include <Arduino.h>
#include "Hardware.h"
#include "SerialToParallelDriver.h"
#include "Util.h"

namespace ace_segment {

void SerialToParallelDriver::displayCurrentField() {
  if (mCurrentDigit != mPrevDigit) {
    writeDigitPin(mPrevDigit, mDigitOff);
    mIsCurrentDigitOn = false;
  }

  DimmingDigit& dimmingDigit = mDimmingDigits[mCurrentDigit];
  if (dimmingDigit.brightness == 0) {
    if (mIsCurrentDigitOn) {
      writeDigitPin(mCurrentDigit, mDigitOff);
      mIsCurrentDigitOn = false;
    }
  } else {
    if (!mIsCurrentDigitOn) {
      SegmentPatternType segmentPattern = dimmingDigit.pattern;
      if (segmentPattern != mSegmentPattern) {
        drawSegments(segmentPattern);
        mSegmentPattern = segmentPattern;
      }
      writeDigitPin(mCurrentDigit, mDigitOn);
      mIsCurrentDigitOn = true;
    }
  }

  mPrevDigit = mCurrentDigit;
  Util::incrementMod(mCurrentDigit, mNumDigits);
}

void SerialToParallelDriver::drawSegments(SegmentPatternType segmentPattern) {
  mHardware->writePin(mLatchPin, LOW);
  mHardware->shiftOut(mDataPin, mClockPin, MSBFIRST, segmentPattern);
  mHardware->writePin(mLatchPin, HIGH);
}

void SerialToParallelDriver::writeDigitPin(uint8_t digit, uint8_t output) {
  uint8_t digitPin = mDigitPins[digit];
  mHardware->writePin(digitPin, output);
}

}
