// This file was generated by the following script:
//   ../../tools/fast_driver.py --digit_pins 4 5 6 7 --segment_serial_pins 10 11 13 --class_name FastSerialDriver --output_files
//
// DO NOT EDIT

#ifdef __AVR__

#include <stdint.h>
#include <Arduino.h>
#include <digitalWriteFast.h>
#include "FastSerialDriver.h"

const uint8_t FastSerialDriver::kDigitPins[] = {
  4,
  5,
  6,
  7,
};

const FastSerialDriver::DigitalWriter FastSerialDriver::kDigitWriters[] = {
  digitalWriteFastDigit00Low,
  digitalWriteFastDigit00High,
  digitalWriteFastDigit01Low,
  digitalWriteFastDigit01High,
  digitalWriteFastDigit02Low,
  digitalWriteFastDigit02High,
  digitalWriteFastDigit03Low,
  digitalWriteFastDigit03High,
};

void FastSerialDriver::configure() {
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, OUTPUT);
    disableDigit(digit);
  }

  pinMode(kLatchPin, OUTPUT);
  pinMode(kDataPin, OUTPUT);
  pinMode(kClockPin, OUTPUT);

  ace_segment::SplitDigitDriver::configure();
}

void FastSerialDriver::finish() {
  ace_segment::SplitDigitDriver::finish();

  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, INPUT);
  }

  pinMode(kLatchPin, INPUT);
  pinMode(kDataPin, INPUT);
  pinMode(kClockPin, INPUT);
}

void FastSerialDriver::displayCurrentField() {
  if (mPreparedToSleep) return;

  bool isCurrentDigitOn;
  ace_segment::DimmablePattern& dimmablePattern =
      mDimmablePatterns[mCurrentDigit];
  uint8_t brightness = dimmablePattern.brightness;
  if (mCurrentDigit != mPrevDigit) {
    disableDigit(mPrevDigit);
    isCurrentDigitOn = false;
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  } else {
    isCurrentDigitOn = mIsPrevDigitOn;
  }

  if (brightness < 255 && mCurrentSubField >= mCurrentSubFieldMax) {
    if (isCurrentDigitOn) {
      disableDigit(mCurrentDigit);
      isCurrentDigitOn = false;
    }
  } else {
    if (!isCurrentDigitOn) {
      SegmentPatternType segmentPattern = dimmablePattern.pattern;
      if (segmentPattern != mSegmentPattern) {
        drawSegments(segmentPattern);
        mSegmentPattern = segmentPattern;
      }
      enableDigit(mCurrentDigit);
      isCurrentDigitOn = true;
    }
  }

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  mIsPrevDigitOn = isCurrentDigitOn;
  if (mCurrentSubField >= mNumSubFields) {
    ace_segment::Util::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }
}

void FastSerialDriver::shiftOutFast(uint8_t pattern) {
  uint8_t mask = 0x80;
  for (uint8_t i = 0; i < 8; i++)  {
    digitalWriteFast(kClockPin, LOW);
    if (pattern & mask) {
      digitalWriteFast(kDataPin, HIGH);
    } else {
      digitalWriteFast(kDataPin, LOW);
    }
    digitalWriteFast(kClockPin, HIGH);
    mask >>= 1;
  }
}

void FastSerialDriver::prepareToSleep() {
  Driver::prepareToSleep();
  disableDigit(mPrevDigit);
}

#endif
