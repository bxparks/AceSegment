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
const uint16_t FRAMES_PER_SECOND = 60;
const int8_t NUM_SUB_FIELDS = 3;
const uint8_t digitPins[NUM_DIGITS] = {0, 1, 2, 3};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
const uint8_t latchPin = 12;
const uint8_t dataPin = 13;
const uint8_t clockPin = 14;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmingDigit dimmingDigits[NUM_DIGITS + 1];
StyledDigit styledDigits[NUM_DIGITS + 1];

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): start"));

  //Serial.println(F("setup(): setting excludes and includes"));
  //TestRunner::exclude("*");
  //TestRunner::include("DigitDriverTest*");
  //TestRunner::include("SegmentDriverTest*");
  //TestRunner::include("ModulatingDigitDriverTest*");
  //TestRunner::include("EdgeCaseModulatingDigitDriverTest*");
  //TestRunner::include("EdgeCaseModulatingDigitDriverTest*");
  //TestRunner::include("DigitDriverSerialLedMatrixTest*");

  Serial.println(F("setup(): end"));
}

void loop() {
  TestRunner::run();
}

// ----------------------------------------------------------------------
// Tests for DigitDriver.
// ----------------------------------------------------------------------

class DigitDriverTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mDriver = DriverBuilder(mHardware)
          .setNumDigits(NUM_DIGITS)
          .setCommonCathode()
          .setResistorsOnSegments()
          .setDigitPins(digitPins)
          .setSegmentDirectPins(segmentPins)
          .setDimmingDigits(dimmingDigits)
          .build();

      mDriver->configure();
      mHardware->clear();
    }

    virtual void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(DigitDriverTest, configure) {
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
}

testF(DigitDriverTest, displayCurrentField_one_dark) {
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
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

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

testF(DigitDriverTest, displayCurrentField_repeated_segment_pattern) {
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

// ----------------------------------------------------------------------
// Tests for SegmentDriver.
// ----------------------------------------------------------------------

class SegmentDriverTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mDriver = DriverBuilder(mHardware)
          .setNumDigits(NUM_DIGITS)
          .setCommonCathode()
          .setResistorsOnDigits()
          .setDigitPins(digitPins)
          .setSegmentDirectPins(segmentPins)
          .setDimmingDigits(dimmingDigits)
          .build();
      mDriver->configure();
      mHardware->clear();
    }

    virtual void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(SegmentDriverTest, configure) {
  mDriver->configure();
  assertEqual((uint16_t)(8), mDriver->getFieldsPerFrame());
}

testF(SegmentDriverTest, displayCurrentField_one_dark) {
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

// ----------------------------------------------------------------------
// Tests for ModulatingDigitDriver. numSubFieldsPerField = 3
// ----------------------------------------------------------------------

class ModulatingDigitDriverTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mDriver = DriverBuilder(mHardware)
          .setNumDigits(NUM_DIGITS)
          .setCommonCathode()
          .setResistorsOnSegments()
          .setDigitPins(digitPins)
          .setSegmentDirectPins(segmentPins)
          .setDimmingDigits(dimmingDigits)
          .useModulatingDriver(NUM_SUB_FIELDS)
          .build();
      mDriver->configure();
      mHardware->clear();
    }

    virtual void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(ModulatingDigitDriverTest, configure) {
  mDriver->configure();
  assertEqual((uint16_t)(3 * 4), mDriver->getFieldsPerFrame());
}

testF(ModulatingDigitDriverTest, displayCurrentField_one_dark) {
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

  // field 4.0
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
  assertEvents(0); // 255 is special, always on regardless of integer roundoff
}

testF(ModulatingDigitDriverTest,
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

// ----------------------------------------------------------------------
// Tests for ModulatingDigitDriver with an edge case of
// numSubFieldsPerField = 1. This should not normally happen, but let's
// make sure that the code doesn't blow up.
// ----------------------------------------------------------------------

class EdgeCaseModulatingDigitDriverTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mDriver = DriverBuilder(mHardware)
          .setNumDigits(NUM_DIGITS)
          .setCommonCathode()
          .setResistorsOnSegments()
          .setDigitPins(digitPins)
          .setSegmentDirectPins(segmentPins)
          .setDimmingDigits(dimmingDigits)
          .useModulatingDriver(1) // edge case of 1 subfield/field
          .build();

      mDriver->configure();
      mHardware->clear();
    }

    virtual void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(EdgeCaseModulatingDigitDriverTest, displayCurrentField) {
  mDriver->setPattern(0, 0x11, 255);
  mDriver->setPattern(1, 0x22, 128);
  mDriver->setPattern(2, 0x55, 64);
  mDriver->setPattern(3, 0x11, 0);

  // field 0.0, should be "on" even though integer round off with a
  // numSubFields of 1 would normally cause a brightness of 0
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

  // field 1.0, the next iteration goes to the next frame
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF);

  // field 2.0, the next iteration goes to the next frame
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 3.0, the next iteration goes to the next frame
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 4.0, should be "on" again, and we reuse the bit patterns from
  // the previous iteration.
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);
}

// ----------------------------------------------------------------------
// Tests for DigitDriver w/ LedMatrixSerial.
// ----------------------------------------------------------------------

class DigitDriverSerialLedMatrixTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mDriver = DriverBuilder(mHardware)
          .setNumDigits(NUM_DIGITS)
          .setCommonCathode()
          .setResistorsOnSegments()
          .setDigitPins(digitPins)
          .setSegmentSerialPins(latchPin, dataPin, clockPin)
          .setDimmingDigits(dimmingDigits)
          .build();

      mDriver->configure();
      mHardware->clear();
    }
    virtual void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(DigitDriverSerialLedMatrixTest, configure) {
  mDriver->configure();
  assertEvents(14,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeDigitalWrite, dataPin, LOW,
      Event::kTypeDigitalWrite, clockPin, LOW,
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
}

testF(DigitDriverSerialLedMatrixTest, displayCurrentField) {

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
