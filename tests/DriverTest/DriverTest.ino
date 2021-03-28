#line 2 "DriverTest.ino"

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

#include <stdarg.h>
#include <Arduino.h>
#include <AUnit.h>
#include <AceSegment.h>
#include <ace_segment/testing/BaseHardwareTest.h>

using namespace aunit;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t DIGIT_ON = LOW;
const int8_t DIGIT_OFF = HIGH;
const int8_t SEGMENT_ON = HIGH;
const int8_t SEGMENT_OFF = LOW;

const int8_t NUM_DIGITS = 4;
const int8_t NUM_SEGMENTS = 8;
const uint16_t FRAMES_PER_SECOND = 60;
const int8_t NUM_SUB_FIELDS = 3;
const uint8_t digitPins[NUM_DIGITS] = {0, 1, 2, 3};
const uint8_t segmentPins[NUM_SEGMENTS] = {4, 5, 6, 7, 8, 9, 10, 11};
const uint8_t latchPin = 12;
const uint8_t dataPin = 13;
const uint8_t clockPin = 14;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmablePattern dimmablePatterns[NUM_DIGITS + 1];

// ----------------------------------------------------------------------
// Tests for SplitDigitDriver w/ LedMatrixDirect
// ----------------------------------------------------------------------

class SplitDirectDigitDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new SplitDirectDigitDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          1 /* numSubFields */,
          digitPins,
          segmentPins);
      mDriver->configure();
      mHardware->clear();
    }

    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(SplitDirectDigitDriverTest, configure) {
  mDriver->configure();
  assertEvents(24,
      Event::kTypePinMode, 0, OUTPUT,
      Event::kTypeDigitalWrite, 0, HIGH,
      Event::kTypePinMode, 1, OUTPUT,
      Event::kTypeDigitalWrite, 1, HIGH,
      Event::kTypePinMode, 2, OUTPUT,
      Event::kTypeDigitalWrite, 2, HIGH,
      Event::kTypePinMode, 3, OUTPUT,
      Event::kTypeDigitalWrite, 3, HIGH,
      Event::kTypePinMode, 4, OUTPUT,
      Event::kTypeDigitalWrite, 4, LOW,
      Event::kTypePinMode, 5, OUTPUT,
      Event::kTypeDigitalWrite, 5, LOW,
      Event::kTypePinMode, 6, OUTPUT,
      Event::kTypeDigitalWrite, 6, LOW,
      Event::kTypePinMode, 7, OUTPUT,
      Event::kTypeDigitalWrite, 7, LOW,
      Event::kTypePinMode, 8, OUTPUT,
      Event::kTypeDigitalWrite, 8, LOW,
      Event::kTypePinMode, 9, OUTPUT,
      Event::kTypeDigitalWrite, 9, LOW,
      Event::kTypePinMode, 10, OUTPUT,
      Event::kTypeDigitalWrite, 10, LOW,
      Event::kTypePinMode, 11, OUTPUT,
      Event::kTypeDigitalWrite, 11, LOW);
  assertEqual((uint16_t)4, mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(12,
      Event::kTypePinMode, 0, INPUT,
      Event::kTypePinMode, 1, INPUT,
      Event::kTypePinMode, 2, INPUT,
      Event::kTypePinMode, 3, INPUT,
      Event::kTypePinMode, 4, INPUT,
      Event::kTypePinMode, 5, INPUT,
      Event::kTypePinMode, 6, INPUT,
      Event::kTypePinMode, 7, INPUT,
      Event::kTypePinMode, 8, INPUT,
      Event::kTypePinMode, 9, INPUT,
      Event::kTypePinMode, 10, INPUT,
      Event::kTypePinMode, 11, INPUT);
}

testF(SplitDirectDigitDriverTest, displayCurrentField_one_dark) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);

  // display field 2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_ON,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_ON,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON);

  // display field 3
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);
}

testF(SplitDirectDigitDriverTest, displayCurrentField_repeated_segment_pattern) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  // same segment pattern should optimize the next iteration
  mDriver->setPattern(3, 0x11, 64);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);

  // display field 2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_ON,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_ON,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON);

  // display field 3
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_ON);

  // display field 0
  // Segment pattern is repeated from the previous cycle, so shouldn't see the
  // pins being set.
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);
}

testF(SplitDirectDigitDriverTest, prepareToSleep) {
  //enableVerbosity(Verbosity::kAssertionPassed);

  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  mHardware->clear();
  mDriver->prepareToSleep();
  assertEvents(1,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF);

  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  mHardware->clear();
  mDriver->wakeFromSleep();
  assertEvents(0);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);
}

// ----------------------------------------------------------------------
// Tests for SplitSegmentDriver w/ LedMatrixDirect.
// ----------------------------------------------------------------------

class SplitDirectSegmentDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new SplitDirectSegmentDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          digitPins,
          segmentPins);
      mDriver->configure();
      mHardware->clear();
    }

    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(SplitDirectSegmentDriverTest, configure) {
  mDriver->configure();
  assertEvents(24,
      Event::kTypePinMode, 4, OUTPUT,
      Event::kTypeDigitalWrite, 4, LOW,
      Event::kTypePinMode, 5, OUTPUT,
      Event::kTypeDigitalWrite, 5, LOW,
      Event::kTypePinMode, 6, OUTPUT,
      Event::kTypeDigitalWrite, 6, LOW,
      Event::kTypePinMode, 7, OUTPUT,
      Event::kTypeDigitalWrite, 7, LOW,
      Event::kTypePinMode, 8, OUTPUT,
      Event::kTypeDigitalWrite, 8, LOW,
      Event::kTypePinMode, 9, OUTPUT,
      Event::kTypeDigitalWrite, 9, LOW,
      Event::kTypePinMode, 10, OUTPUT,
      Event::kTypeDigitalWrite, 10, LOW,
      Event::kTypePinMode, 11, OUTPUT,
      Event::kTypeDigitalWrite, 11, LOW,
      Event::kTypePinMode, 0, OUTPUT,
      Event::kTypeDigitalWrite, 0, HIGH,
      Event::kTypePinMode, 1, OUTPUT,
      Event::kTypeDigitalWrite, 1, HIGH,
      Event::kTypePinMode, 2, OUTPUT,
      Event::kTypeDigitalWrite, 2, HIGH,
      Event::kTypePinMode, 3, OUTPUT,
      Event::kTypeDigitalWrite, 3, HIGH);
  assertEqual((uint16_t)(8), mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(12,
      Event::kTypePinMode, 4, INPUT,
      Event::kTypePinMode, 5, INPUT,
      Event::kTypePinMode, 6, INPUT,
      Event::kTypePinMode, 7, INPUT,
      Event::kTypePinMode, 8, INPUT,
      Event::kTypePinMode, 9, INPUT,
      Event::kTypePinMode, 10, INPUT,
      Event::kTypePinMode, 11, INPUT,
      Event::kTypePinMode, 0, INPUT,
      Event::kTypePinMode, 1, INPUT,
      Event::kTypePinMode, 2, INPUT,
      Event::kTypePinMode, 3, INPUT);
}

testF(SplitDirectSegmentDriverTest, displayCurrentField_one_dark) {
  // The following segment patterns were crafted so that digitPattern(7) bits
  // have the same digitPattern as digitPattern(0), which should cause the digit
  // bits to be reused between the two iterations.
  mDriver->setPattern(0, 0x91, 255);  // 1001 0001
  mDriver->setPattern(1, 0x22, 128);  // 0010 0010
  mDriver->setPattern(2, 0xD5, 64);   // 1101 0101
  mDriver->setPattern(3, 0x91, 0);    // 0000 0000 <- 1001 0001 (brightness = 0)

  // field 0 (segment 0)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON);

  // field 1 (segment 1)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON);

  // field 2 (segment 2)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_ON);

  // field 3 (segment 3)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_ON);

  // field 4 (segment 4)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON);

  // field 5 (segment 5)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON);

  // field 6 (segment 6)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_ON);

  // field 7 (segment 7)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_ON);

  // field 0 (segment 0), reuse the digit pattern from the prev iteration
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON);
}

testF(SplitDirectSegmentDriverTest, prepareToSleep) {
  mDriver->setPattern(0, 0x91, 255);  // 1001 0001
  mDriver->setPattern(1, 0x22, 128);  // 0010 0010
  mDriver->setPattern(2, 0xD5, 64);   // 1101 0101
  mDriver->setPattern(3, 0x91, 0);    // 0000 0000 <- 1001 0001 (brightness = 0)

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON);

  mHardware->clear();
  mDriver->prepareToSleep();
  assertEvents(1,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF);

  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  mHardware->clear();
  mDriver->wakeFromSleep();
  assertEvents(0);

  // field 1 (segment 1)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON);
}

// ----------------------------------------------------------------------
// Tests for SplitDirectDigitDriver w/ Modulation at numSubFieldsPerField = 3
// ----------------------------------------------------------------------

class ModulatingSplitDirectDigitDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new SplitDirectDigitDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          NUM_SUB_FIELDS,
          digitPins,
          segmentPins);
      mDriver->configure();
      mHardware->clear();
    }

    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(ModulatingSplitDirectDigitDriverTest, configure) {
  mDriver->configure();
  assertEvents(24,
      Event::kTypePinMode, 0, OUTPUT,
      Event::kTypeDigitalWrite, 0, HIGH,
      Event::kTypePinMode, 1, OUTPUT,
      Event::kTypeDigitalWrite, 1, HIGH,
      Event::kTypePinMode, 2, OUTPUT,
      Event::kTypeDigitalWrite, 2, HIGH,
      Event::kTypePinMode, 3, OUTPUT,
      Event::kTypeDigitalWrite, 3, HIGH,
      Event::kTypePinMode, 4, OUTPUT,
      Event::kTypeDigitalWrite, 4, LOW,
      Event::kTypePinMode, 5, OUTPUT,
      Event::kTypeDigitalWrite, 5, LOW,
      Event::kTypePinMode, 6, OUTPUT,
      Event::kTypeDigitalWrite, 6, LOW,
      Event::kTypePinMode, 7, OUTPUT,
      Event::kTypeDigitalWrite, 7, LOW,
      Event::kTypePinMode, 8, OUTPUT,
      Event::kTypeDigitalWrite, 8, LOW,
      Event::kTypePinMode, 9, OUTPUT,
      Event::kTypeDigitalWrite, 9, LOW,
      Event::kTypePinMode, 10, OUTPUT,
      Event::kTypeDigitalWrite, 10, LOW,
      Event::kTypePinMode, 11, OUTPUT,
      Event::kTypeDigitalWrite, 11, LOW);
  assertEqual((uint16_t)(3 * 4), mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(12,
      Event::kTypePinMode, 0, INPUT,
      Event::kTypePinMode, 1, INPUT,
      Event::kTypePinMode, 2, INPUT,
      Event::kTypePinMode, 3, INPUT,
      Event::kTypePinMode, 4, INPUT,
      Event::kTypePinMode, 5, INPUT,
      Event::kTypePinMode, 6, INPUT,
      Event::kTypePinMode, 7, INPUT,
      Event::kTypePinMode, 8, INPUT,
      Event::kTypePinMode, 9, INPUT,
      Event::kTypePinMode, 10, INPUT,
      Event::kTypePinMode, 11, INPUT);
}

testF(ModulatingSplitDirectDigitDriverTest, displayCurrentField_one_dark) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // field 0.0, stays on for 3 sub fields
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // field 0.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 0.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0); // 255 is special, always on regardless of integer roundoff

  // field 1.0, stays on for 1/3 subfields
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);

  // field 1.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 1.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 2.0, stays on for 0/3 subfields (because 64/256*3 is 0).
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 2.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 2.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 3.0, is off for 3/3 subfields
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 3.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 3.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 0.0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // field 0.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 0.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0); // 255 is special, always on regardless of integer roundoff
}

testF(ModulatingSplitDirectDigitDriverTest,
    displayCurrentField_repeated_segment_pattern) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  // same segment pattern should optimize the next iteration
  mDriver->setPattern(2, 0x22, 128);
  mDriver->setPattern(3, 0x11, 64);

  // field 0.0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // field 0.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 0.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 1.0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);

  // field 1.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 1.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 2.0, stays on for 1/3 subfields reusing the previous pattern
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON);

  // field 2.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 2.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 3.0, off for 3/3 subfields
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 3.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 3.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 4.0, same segment pattern, so don't need to write the pins again
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // field 4.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 4.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);
}

testF(ModulatingSplitDirectDigitDriverTest, prepareToSleep) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // field 0.0, stays on for 3 sub fields
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // field 0.1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  // field 0.2
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0); // 255 is special, always on regardless of integer roundoff

  // prepareToSleep() on digit-to-digit boundary must turn off the previous
  // digit, not the "current" digit (this was an early bug).
  mHardware->clear();
  mDriver->prepareToSleep();
  assertEvents(1,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF);

  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  mHardware->clear();
  mDriver->wakeFromSleep();
  assertEvents(0);

  // field 1.0, stays on for 1/3 subfields
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(10,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);
}

// ----------------------------------------------------------------------
// Tests for SplitDigitDriver w/ LedMatrixSplitSerial.
// ----------------------------------------------------------------------

class SplitSerialDigitDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new SplitSerialDigitDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          1 /* numSubFields */,
          digitPins,
          latchPin, dataPin, clockPin);
      mDriver->configure();
      mHardware->clear();
    }
    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(SplitSerialDigitDriverTest, configure) {
  mDriver->configure();
  assertEvents(11,
      Event::kTypePinMode, latchPin, OUTPUT,
      Event::kTypePinMode, dataPin, OUTPUT,
      Event::kTypePinMode, clockPin, OUTPUT,
      Event::kTypePinMode, 0, OUTPUT,
      Event::kTypeDigitalWrite, 0, HIGH,
      Event::kTypePinMode, 1, OUTPUT,
      Event::kTypeDigitalWrite, 1, HIGH,
      Event::kTypePinMode, 2, OUTPUT,
      Event::kTypeDigitalWrite, 2, HIGH,
      Event::kTypePinMode, 3, OUTPUT,
      Event::kTypeDigitalWrite, 3, HIGH);
  assertEqual((uint16_t)(4), mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(7,
      Event::kTypePinMode, latchPin, INPUT,
      Event::kTypePinMode, dataPin, INPUT,
      Event::kTypePinMode, clockPin, INPUT,
      Event::kTypePinMode, 0, INPUT,
      Event::kTypePinMode, 1, INPUT,
      Event::kTypePinMode, 2, INPUT,
      Event::kTypePinMode, 3, INPUT);
}

testF(SplitSerialDigitDriverTest, displayCurrentField) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(5,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, 0x11,
      Event::kTypeDigitalWrite, latchPin, HIGH,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(5,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, 0x22,
      Event::kTypeDigitalWrite, latchPin, HIGH,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);
}

// ----------------------------------------------------------------------
// Tests for SplitDigitDriver w/ LedMatrixSplitSpi.
// ----------------------------------------------------------------------

class SplitSpiDigitDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new SplitSpiDigitDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          1 /* numSubFields */,
          digitPins,
          latchPin, dataPin, clockPin);
      mDriver->configure();
      mHardware->clear();
    }
    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(SplitSpiDigitDriverTest, configure) {
  mDriver->configure();
  assertEvents(12,
      Event::kTypePinMode, latchPin, OUTPUT,
      Event::kTypePinMode, dataPin, OUTPUT,
      Event::kTypePinMode, clockPin, OUTPUT,
      Event::kTypePinMode, 0, OUTPUT,
      Event::kTypeDigitalWrite, 0, HIGH,
      Event::kTypePinMode, 1, OUTPUT,
      Event::kTypeDigitalWrite, 1, HIGH,
      Event::kTypePinMode, 2, OUTPUT,
      Event::kTypeDigitalWrite, 2, HIGH,
      Event::kTypePinMode, 3, OUTPUT,
      Event::kTypeDigitalWrite, 3, HIGH,
      Event::kTypeSpiBegin);
  assertEqual((uint16_t)(4), mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(8,
      Event::kTypeSpiEnd,
      Event::kTypePinMode, latchPin, INPUT,
      Event::kTypePinMode, dataPin, INPUT,
      Event::kTypePinMode, clockPin, INPUT,
      Event::kTypePinMode, 0, INPUT,
      Event::kTypePinMode, 1, INPUT,
      Event::kTypePinMode, 2, INPUT,
      Event::kTypePinMode, 3, INPUT);
}

testF(SplitSpiDigitDriverTest, displayCurrentField) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(5,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeSpiTransfer, 0x11,
      Event::kTypeDigitalWrite, latchPin, HIGH,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(5,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeSpiTransfer, 0x22,
      Event::kTypeDigitalWrite, latchPin, HIGH,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);
}

// ----------------------------------------------------------------------
// Tests for MergedDigitDriver w/ LedMatrixMergedSerial.
// ----------------------------------------------------------------------

class MergedSerialDigitDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new MergedSerialDigitDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          1 /* numSubFields */,
          latchPin, dataPin, clockPin);
      mDriver->configure();
      mHardware->clear();
    }
    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(MergedSerialDigitDriverTest, configure) {
  mDriver->configure();
  assertEvents(3,
      Event::kTypePinMode, latchPin, OUTPUT,
      Event::kTypePinMode, dataPin, OUTPUT,
      Event::kTypePinMode, clockPin, OUTPUT);
  assertEqual((uint16_t)(4), mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(3,
      Event::kTypePinMode, latchPin, INPUT,
      Event::kTypePinMode, dataPin, INPUT,
      Event::kTypePinMode, clockPin, INPUT);
}

testF(MergedSerialDigitDriverTest, displayCurrentField) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(4,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, ~0x01,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, 0x11,
      Event::kTypeDigitalWrite, latchPin, HIGH);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(4,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, ~0x02,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, 0x22,
      Event::kTypeDigitalWrite, latchPin, HIGH);
}

// ----------------------------------------------------------------------
// Tests for MergedDigitDriver w/ LedMatrixMergedSpi.
// ----------------------------------------------------------------------

class MergedSpiDigitDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new MergedSpiDigitDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          1 /* numSubFields */,
          latchPin, dataPin, clockPin);
      mDriver->configure();
      mHardware->clear();
    }
    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(MergedSpiDigitDriverTest, configure) {
  mDriver->configure();
  assertEvents(4,
      Event::kTypePinMode, latchPin, OUTPUT,
      Event::kTypePinMode, dataPin, OUTPUT,
      Event::kTypePinMode, clockPin, OUTPUT,
      Event::kTypeSpiBegin);
  assertEqual((uint16_t)(4), mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(4,
      Event::kTypeSpiEnd,
      Event::kTypePinMode, latchPin, INPUT,
      Event::kTypePinMode, dataPin, INPUT,
      Event::kTypePinMode, clockPin, INPUT);
}

testF(MergedSpiDigitDriverTest, displayCurrentField) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(3,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeSpiTransfer16, (~0x01 << 8) | 0x11,
      Event::kTypeDigitalWrite, latchPin, HIGH);

  // display field 1
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(3,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeSpiTransfer16, (~0x02 << 8) | 0x22,
      Event::kTypeDigitalWrite, latchPin, HIGH);
}

//----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  //Serial.println(F("setup(): setting excludes and includes"));
  //TestRunner::exclude("*");
  //TestRunner::include("SplitDirectDigitDriverTest*");
  //TestRunner::include("ModulatingSplitDigitDriverTest*");
  //TestRunner::include("SplitDirectSegmentDriverTest*");
  //TestRunner::include("SplitSerialDigitDriverTest*");
  //TestRunner::include("MergedSerialDigitDriverTest*");
}

void loop() {
  TestRunner::run();
}
