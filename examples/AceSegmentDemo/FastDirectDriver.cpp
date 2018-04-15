// This file was generated by the following script:
//   ../../tools/fast_driver.py --digit_pins 12 14 15 16 --segment_direct_pins 4 5 6 7 8 9 10 11 --class_name FastDirectDriver --output_files
//
// DO NOT EDIT

#include <stdint.h>
#include <Arduino.h>
#include <digitalWriteFast.h>
#include "FastDirectDriver.h"

const uint8_t FastDirectDriver::kSegmentPins[] = {
  4,
  5,
  6,
  7,
  8,
  9,
  10,
  11,
};

const uint8_t FastDirectDriver::kDigitPins[] = {
  12,
  14,
  15,
  16,
};

const FastDirectDriver::DigitalWriter FastDirectDriver::kSegmentWriters[] = {
  digitalWriteFastSegment00Low,
  digitalWriteFastSegment00High,
  digitalWriteFastSegment01Low,
  digitalWriteFastSegment01High,
  digitalWriteFastSegment02Low,
  digitalWriteFastSegment02High,
  digitalWriteFastSegment03Low,
  digitalWriteFastSegment03High,
  digitalWriteFastSegment04Low,
  digitalWriteFastSegment04High,
  digitalWriteFastSegment05Low,
  digitalWriteFastSegment05High,
  digitalWriteFastSegment06Low,
  digitalWriteFastSegment06High,
  digitalWriteFastSegment07Low,
  digitalWriteFastSegment07High,
};

const FastDirectDriver::DigitalWriter FastDirectDriver::kDigitWriters[] = {
  digitalWriteFastDigit00Low,
  digitalWriteFastDigit00High,
  digitalWriteFastDigit01Low,
  digitalWriteFastDigit01High,
  digitalWriteFastDigit02Low,
  digitalWriteFastDigit02High,
  digitalWriteFastDigit03Low,
  digitalWriteFastDigit03High,
};

void FastDirectDriver::configure() {
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, OUTPUT);
    disableDigit(digit);
  }

  for (uint8_t segment = 0; segment < kNumSegments; segment++) {
    uint8_t elementPin = kSegmentPins[segment];
    pinMode(elementPin, OUTPUT);
    writeSegment(segment, kSegmentOff);
  }

  ace_segment::ModulatingDigitDriver::configure();
}

void FastDirectDriver::displayCurrentField() {
  if (mPreparedToSleep) return;

  bool isCurrentDigitOn;
  ace_segment::DimmingDigit& dimmingDigit = mDimmingDigits[mCurrentDigit];
  uint8_t brightness = dimmingDigit.brightness;
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
      SegmentPatternType segmentPattern = dimmingDigit.pattern;
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

void FastDirectDriver::drawSegments(uint8_t pattern) {
  uint8_t elementMask = 0x1;
  for (uint8_t segment = 0; segment < kNumSegments; segment++) {
    uint8_t output = (pattern & elementMask) ? kSegmentOn : kSegmentOff;
    writeSegment(segment, output);
    elementMask <<= 1;
  }
}

void FastDirectDriver::prepareToSleep() {
  Driver::prepareToSleep();
  disableDigit(mPrevDigit);
}
