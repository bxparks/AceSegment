// This file was generated by the following script:
//   /home/brian/dev/AceSegment/tools/fast_driver.py --digit_pins 4 5 6 7 --segment_spi_pins 10 11 13 --class_name FastSpiDriver --output_files
//
// DO NOT EDIT

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include "FastSpiDriver.h"

const uint8_t FastSpiDriver::kDigitPins[] = {
  4,
  5,
  6,
  7,
};

const FastSpiDriver::DigitalWriter FastSpiDriver::kDigitWriters[] = {
  digitalWriteFastDigit00Low,
  digitalWriteFastDigit00High,
  digitalWriteFastDigit01Low,
  digitalWriteFastDigit01High,
  digitalWriteFastDigit02Low,
  digitalWriteFastDigit02High,
  digitalWriteFastDigit03Low,
  digitalWriteFastDigit03High,
};

void FastSpiDriver::configure() {
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, OUTPUT);
    disableDigit(digit);
  }

  pinMode(kLatchPin, OUTPUT);
  pinMode(kDataPin, OUTPUT);
  pinMode(kClockPin, OUTPUT);

  SPI.begin();

  ace_segment::ModulatingDigitDriver::configure();
}

void FastSpiDriver::displayCurrentField() {
  ace_segment::DimmingDigit& dimmingDigit = mDimmingDigits[mCurrentDigit];
  uint8_t brightness = dimmingDigit.brightness;
  if (mCurrentDigit != mPrevDigit) {
    disableDigit(mPrevDigit);
    mIsCurrentDigitOn = false;
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  }

  if (brightness < 255 && mCurrentSubField >= mCurrentSubFieldMax) {
    if (mIsCurrentDigitOn) {
      disableDigit(mCurrentDigit);
      mIsCurrentDigitOn = false;
    }
  } else {
    if (!mIsCurrentDigitOn) {
      SegmentPatternType segmentPattern = dimmingDigit.pattern;
      if (segmentPattern != mSegmentPattern) {
        drawSegments(segmentPattern);
        mSegmentPattern = segmentPattern;
      }
      enableDigit(mCurrentDigit);
      mIsCurrentDigitOn = true;
    }
  }

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  if (mCurrentSubField >= mNumSubFields) {
    ace_segment::Util::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }
}
