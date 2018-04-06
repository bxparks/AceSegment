#line 2 "AceSegmentTest.ino"

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
#include <ace_segment/SegmentDriver.h>
#include <ace_segment/DigitDriver.h>
#include <ace_segment/ModulatingDigitDriver.h>
#include <ace_segment/LedMatrixDirect.h>
#include <ace_segment/LedMatrixSerial.h>
#include <ace_segment/testing/TestableHardware.h>
#include <ace_segment/testing/FakeDriver.h>

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

// fake hardware methods to allow testing
TestableHardware hardware;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmingDigit dimmingDigits[NUM_DIGITS + 1];
StyledDigit styledDigits[NUM_DIGITS + 1];

// the various kinds of drivers that we will test
Driver* digitDriver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();

Driver* modulatingDigitDriver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .setNumSubFields(NUM_SUB_FIELDS)
    .useModulatingDriver()
    .build();

Driver* edgeCaseModulatingDigitDriver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .setNumSubFields(1) // edge case of 1 subfield/field
    .useModulatingDriver()
    .build();

Driver* segmentDriver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnDigits()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();

Driver* serialToParallelDriver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentSerialPins(latchPin, dataPin, clockPin)
    .setDimmingDigits(dimmingDigits)
    .build();

FakeDriver fakeDriver(dimmingDigits, NUM_DIGITS);
Renderer renderer(&hardware, &fakeDriver, styledDigits, NUM_DIGITS);
CharWriter charWriter(&renderer);
StringWriter stringWriter(&charWriter);

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  //TestRunner::exclude("*");
  //TestRunner::include("SerialToParallelDriverTest*");

  Serial.println(F("setup(): start"));
  Serial.print(F("sizeof(Hardware): "));
  Serial.println(sizeof(Hardware));
  Serial.print(F("sizeof(LedMatrixDirect): "));
  Serial.println(sizeof(LedMatrixDirect));
  Serial.print(F("sizeof(LedMatrixSerial): "));
  Serial.println(sizeof(LedMatrixSerial));
  Serial.print(F("sizeof(Driver): "));
  Serial.println(sizeof(Driver));
  Serial.print(F("sizeof(SegmentDriver): "));
  Serial.println(sizeof(SegmentDriver));
  Serial.print(F("sizeof(DigitDriver): "));
  Serial.println(sizeof(DigitDriver));
  Serial.print(F("sizeof(ModulatingDigitDriver): "));
  Serial.println(sizeof(ModulatingDigitDriver));
  Serial.print(F("sizeof(Renderer): "));
  Serial.println(sizeof(Renderer));
  Serial.print(F("sizeof(CharWriter): "));
  Serial.println(sizeof(CharWriter));
  Serial.print(F("sizeof(StringWriter): "));
  Serial.println(sizeof(StringWriter));
}

void loop() {
  aunit::TestRunner::run();
}

void clearStyledDigits() {
  for (int i = 0; i < NUM_DIGITS; i++) {
    styledDigits[i].pattern = 0;
    styledDigits[i].style = 0;
  }
}

// ----------------------------------------------------------------------
// Tests for Util.
// ----------------------------------------------------------------------

test(incrementMod) {
  uint8_t i = 0;
  uint8_t n = 3;

  Util::incrementMod(i, n);
  assertEqual(i, 1);
  Util::incrementMod(i, n);
  assertEqual(i, 2);
  Util::incrementMod(i, n);
  assertEqual(i, 0);
}

// ----------------------------------------------------------------------
// Base class to assert various hardware events on TestableHardware.
// ----------------------------------------------------------------------

class BaseHardwareTest: public TestOnce {
  protected:

    /**
     * assertEvents(numEvents,
     *    type, arg1, arg2[, ...],
     *    ...
     *    type, arg1, arg2[, ...]);
     */
    void assertEvents(int8_t n, ...) {
      assertEqual(n, hardware.getNumRecords());
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t type = va_arg(args, int);
        switch (type) {
          case Event::kTypeDigitalWrite: {
              uint8_t pin = va_arg(args, int);
              uint8_t value = va_arg(args, int);
              Event& event = hardware.getEvent(i);
              assertEqual(pin, event.arg1);
              assertEqual(value, event.arg2);
            }
            break;
          case Event::kTypePinMode: {
              uint8_t pin = va_arg(args, int);
              uint8_t mode = va_arg(args, int);
              Event& event = hardware.getEvent(i);
              assertEqual(pin, event.arg1);
              assertEqual(mode, event.arg2);
            }
            break;
          case Event::kTypeShiftOut: {
              uint8_t dataPin = va_arg(args, int);
              uint8_t clockPin = va_arg(args, int);
              uint8_t bitOrder = va_arg(args, int);
              uint8_t value = va_arg(args, int);
              Event& event = hardware.getEvent(i);
              assertEqual(dataPin, event.arg1);
              assertEqual(clockPin, event.arg2);
              assertEqual(bitOrder, event.arg3);
              assertEqual(value, event.arg4);
            }
            break;
          default:
            // Unknown event type
            fail();
        }
      }
      va_end(args);
    }
};

// ----------------------------------------------------------------------
// Test some isolated methods of Driver using FakeDriver.
// ----------------------------------------------------------------------

class FakeDriverTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      hardware.clear();
    }
};

testF(FakeDriverTest, setPattern) {
  const DimmingDigit& dimmingDigit = dimmingDigits[0];

  fakeDriver.setPattern(0, 0x11);
  assertEqual(0x11, dimmingDigit.pattern);
  assertEqual(255, dimmingDigit.brightness);

  fakeDriver.setPattern(0, 0x11, 10);
  assertEqual(0x11, dimmingDigit.pattern);
  assertEqual(10, dimmingDigit.brightness);
}

// Setting a digit that's out of bounds (>= NUM_DIGITS) does nothing
testF(FakeDriverTest, setPattern_outOfBounds) {
  DimmingDigit& digitOutOfBounds = dimmingDigits[4];
  digitOutOfBounds.pattern = 1;
  digitOutOfBounds.brightness = 2;
  fakeDriver.setPattern(5, 0x11, 10);
  assertEqual(1, digitOutOfBounds.pattern);
  assertEqual(2, digitOutOfBounds.brightness);
}

testF(FakeDriverTest, setBrightness) {
  const DimmingDigit& dimmingDigit = dimmingDigits[0];

  fakeDriver.setBrightness(0, 20);
  assertEqual(0x11, dimmingDigit.pattern);
  assertEqual(20, dimmingDigit.brightness);
}

// Setting a digit that's out of bounds (>= NUM_DIGITS) does nothing
testF(FakeDriverTest, setBrightness_outOfBounds) {
  DimmingDigit& digitOutOfBounds = dimmingDigits[4];
  digitOutOfBounds.pattern = 1;
  digitOutOfBounds.brightness = 2;
  fakeDriver.setBrightness(5, 10);
  assertEqual(1, digitOutOfBounds.pattern);
  assertEqual(2, digitOutOfBounds.brightness);
}

// ----------------------------------------------------------------------
// Tests for DigitDriver.
// ----------------------------------------------------------------------

class DigitDriverTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      driver->configure();
      hardware.clear();
    }

    Driver* driver = digitDriver;
};

testF(DigitDriverTest, configure) {
  driver->configure();
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
  assertEqual((uint16_t)4, driver->getFieldsPerFrame());
}

testF(DigitDriverTest, displayCurrentField_one_dark) {
  driver->setPattern(0, 0x11, 255);
  driver->setPattern(1, 0x22, 128);
  driver->setPattern(2, 0x55, 64);
  driver->setPattern(3, 0x11, 0);

  // display field 0
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // display field 0
  hardware.clear();
  driver->displayCurrentField();
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
  driver->setPattern(0, 0x11, 255);
  driver->setPattern(1, 0x22, 128);
  driver->setPattern(2, 0x55, 64);
  // same segment pattern should optimize the next iteration
  driver->setPattern(3, 0x11, 64);

  // display field 0
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // display field 1
  hardware.clear();
  driver->displayCurrentField();
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
      driver->configure();
      hardware.clear();
    }

    Driver* driver = segmentDriver;
};

testF(SegmentDriverTest, configure) {
  driver->configure();
  assertEqual((uint16_t)(8), driver->getFieldsPerFrame());
}

testF(SegmentDriverTest, displayCurrentField_one_dark) {
  // The following segment patterns were crafted so that digitPattern(7) bits
  // have the same digitPattern as digitPattern(0), which should cause the digit
  // bits to be reused between the two iterations.
  driver->setPattern(0, 0x91, 255);  // 1001 0001
  driver->setPattern(1, 0x22, 128);  // 0010 0010
  driver->setPattern(2, 0xD5, 64);   // 1101 0101
  driver->setPattern(3, 0x91, 0);    // 0000 0000 <- 1001 0001 (brightness = 0)

  // field 0 (segment 0)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON);

  // field 1 (segment 1)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON);

  // field 2 (segment 2)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_ON);

  // field 3 (segment 3)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_ON);

  // field 4 (segment 4)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON);

  // field 5 (segment 5)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON);

  // field 6 (segment 6)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_ON);

  // field 7 (segment 7)
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_ON);

  // field 0 (segment 0), reuse the digit pattern from the prev iteration
  hardware.clear();
  driver->displayCurrentField();
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
      driver->configure();
      hardware.clear();
    }

    Driver* driver = modulatingDigitDriver;
};

testF(ModulatingDigitDriverTest, configure) {
  driver->configure();
  assertEqual((uint16_t)(3 * 4), driver->getFieldsPerFrame());
}

testF(ModulatingDigitDriverTest, displayCurrentField_one_dark) {
  driver->setPattern(0, 0x11, 255);
  driver->setPattern(1, 0x22, 128);
  driver->setPattern(2, 0x55, 64);
  driver->setPattern(3, 0x11, 0);

  // field 0.0, stays on for 3 sub fields
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 0.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0); // 255 is special, always on regardless of integer roundoff

  // field 1.0, stays on for 1/3 subfields
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 1.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 2.0, stays on for 0/3 subfields (because 64/256*3 is 0).
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 2.1
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 2.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 3.0, is off for 3/3 subfields
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 3.1
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 3.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 4.0
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 4.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0); // 255 is special, always on regardless of integer roundoff
}

testF(ModulatingDigitDriverTest,
    displayCurrentField_repeated_segment_pattern) {
  driver->setPattern(0, 0x11, 255);
  driver->setPattern(1, 0x22, 128);
  // same segment pattern should optimize the next iteration
  driver->setPattern(2, 0x22, 128);
  driver->setPattern(3, 0x11, 64);

  // field 0.0
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 0.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 1.0
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 1.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 2.0, stays on for 1/3 subfields reusing the previous pattern
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON);

  // field 2.1
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 2.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 3.0, off for 3/3 subfields
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 3.1
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 3.2
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 4.0, same segment pattern, so don't need to write the pins again
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(0);

  // field 4.2
  hardware.clear();
  driver->displayCurrentField();
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
      driver->configure();
      hardware.clear();
    }

    Driver* driver = edgeCaseModulatingDigitDriver;
};

testF(EdgeCaseModulatingDigitDriverTest, displayCurrentField) {
  driver->setPattern(0, 0x11, 255);
  driver->setPattern(1, 0x22, 128);
  driver->setPattern(2, 0x55, 64);
  driver->setPattern(3, 0x11, 0);

  // field 0.0, should be "on" even though integer round off with a
  // numSubFields of 1 would normally cause a brightness of 0
  hardware.clear();
  driver->displayCurrentField();
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
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF);

  // field 2.0, the next iteration goes to the next frame
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF);

  // field 3.0, the next iteration goes to the next frame
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(1,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF);

  // field 4.0, should be "on" again, and we reuse the bit patterns from
  // the previous iteration.
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);
}

// ----------------------------------------------------------------------
// Tests for SerialToParallelDriver.
// ----------------------------------------------------------------------

class SerialToParallelDriverTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      driver->configure();
      hardware.clear();
    }

    Driver* driver = serialToParallelDriver;
};

testF(SerialToParallelDriverTest, configure) {
  driver->configure();
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
  assertEqual((uint16_t)(4), driver->getFieldsPerFrame());
}

testF(SerialToParallelDriverTest, displayCurrentField) {

  driver->setPattern(0, 0x11, 255);
  driver->setPattern(1, 0x22, 128);
  driver->setPattern(2, 0x55, 64);
  driver->setPattern(3, 0x11, 0);

  // display field 0
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(5,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, 0x11,
      Event::kTypeDigitalWrite, latchPin, HIGH,
      Event::kTypeDigitalWrite, 0, DIGIT_ON);

  // display field 1
  hardware.clear();
  driver->displayCurrentField();
  assertEvents(5,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, 0x22,
      Event::kTypeDigitalWrite, latchPin, HIGH,
      Event::kTypeDigitalWrite, 1, DIGIT_ON);
}

// ----------------------------------------------------------------------
// Tests for Renderer.
// ----------------------------------------------------------------------

test(calcBlinkStateForFrame) {
  uint16_t framesPerBlink = 60;
  uint16_t currentFrame;
  uint8_t blinkState;

  currentFrame = 0;
  Renderer::calcBlinkStateForFrame(framesPerBlink, currentFrame, blinkState);
  assertEqual(blinkState, (uint8_t) 1);
  assertEqual(currentFrame, (uint16_t) 1);

  currentFrame = 29;
  Renderer::calcBlinkStateForFrame(framesPerBlink, currentFrame, blinkState);
  assertEqual(blinkState, (uint8_t) 1);
  assertEqual(currentFrame, (uint16_t) 30);

  currentFrame = 30;
  Renderer::calcBlinkStateForFrame(framesPerBlink, currentFrame, blinkState);
  assertEqual(blinkState, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 31);

  currentFrame = 59;
  Renderer::calcBlinkStateForFrame(framesPerBlink, currentFrame, blinkState);
  assertEqual(blinkState, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 0);

  currentFrame = 60;
  Renderer::calcBlinkStateForFrame(framesPerBlink, currentFrame, blinkState);
  assertEqual(blinkState, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 0);
}

test(calcPulseFractionForFrame) {
  uint16_t framesPerPulse = 31;
  uint16_t currentFrame;
  uint8_t pulseFraction;

  currentFrame = 0;
  Renderer::calcPulseFractionForFrame(framesPerPulse, currentFrame,
      pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 1);

  currentFrame = 1;
  Renderer::calcPulseFractionForFrame(framesPerPulse, currentFrame,
      pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 17);
  assertEqual(currentFrame, (uint16_t) 2);

  currentFrame = 14;
  Renderer::calcPulseFractionForFrame(framesPerPulse, currentFrame,
      pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 238);
  assertEqual(currentFrame, (uint16_t) 15);

  currentFrame = 15;
  Renderer::calcPulseFractionForFrame(framesPerPulse, currentFrame,
      pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 255);
  assertEqual(currentFrame, (uint16_t) 16);

  currentFrame = 16;
  Renderer::calcPulseFractionForFrame(framesPerPulse, currentFrame,
      pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 238);
  assertEqual(currentFrame, (uint16_t) 17);

  currentFrame = 30;
  Renderer::calcPulseFractionForFrame(framesPerPulse, currentFrame,
      pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 0);

  currentFrame = 31;
  Renderer::calcPulseFractionForFrame(framesPerPulse, currentFrame,
      pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 0);
}

test(calcPulseFractionForFrameUsingInverse) {
  uint16_t framesPerPulse = 31;
  uint16_t framesPerPulseInverse = (uint32_t) 65536 / framesPerPulse ;
  uint16_t currentFrame;
  uint8_t pulseFraction;

  currentFrame = 0;
  Renderer::calcPulseFractionForFrameUsingInverse(framesPerPulseInverse,
      framesPerPulse, currentFrame, pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 1);

  currentFrame = 1;
  Renderer::calcPulseFractionForFrameUsingInverse(framesPerPulseInverse,
      framesPerPulse, currentFrame, pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 16);
  assertEqual(currentFrame, (uint16_t) 2);

  currentFrame = 14;
  Renderer::calcPulseFractionForFrameUsingInverse(framesPerPulseInverse,
      framesPerPulse, currentFrame, pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 231);
  assertEqual(currentFrame, (uint16_t) 15);

  currentFrame = 15;
  Renderer::calcPulseFractionForFrameUsingInverse(framesPerPulseInverse,
      framesPerPulse, currentFrame, pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 247);
  assertEqual(currentFrame, (uint16_t) 16);

  // peak of the pulse occurs at 15.5, so the UsingInverse version
  // is actually more accurate than the original

  currentFrame = 16;
  Renderer::calcPulseFractionForFrameUsingInverse(framesPerPulseInverse,
      framesPerPulse, currentFrame, pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 231);
  assertEqual(currentFrame, (uint16_t) 17);

  currentFrame = 30;
  Renderer::calcPulseFractionForFrameUsingInverse(framesPerPulseInverse,
      framesPerPulse, currentFrame, pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 0);

  currentFrame = 31;
  Renderer::calcPulseFractionForFrameUsingInverse(framesPerPulseInverse,
      framesPerPulse, currentFrame, pulseFraction);
  assertEqual(pulseFraction, (uint8_t) 0);
  assertEqual(currentFrame, (uint16_t) 0);
}

test(calcBrightness) {
  const uint8_t overallBrightness = 199;
  const uint8_t pulseSlowFraction = 128; // fraction=128/256
  const uint8_t pulseFastFraction = 63; // fraction=63/256
  uint8_t brightness;

  brightness = Renderer::calcBrightness(StyledDigit::kStyleNormal,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOff,
      false, pulseSlowFraction, pulseFastFraction);
  assertEqual(199, brightness);

  brightness = Renderer::calcBrightness(StyledDigit::kStyleBlinkSlow,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOff,
      false, pulseSlowFraction, pulseFastFraction);
  assertEqual(0, brightness);

  brightness = Renderer::calcBrightness(StyledDigit::kStyleBlinkSlow,
      overallBrightness, Renderer::kBlinkStateOn, Renderer::kBlinkStateOff,
      false, pulseSlowFraction, pulseFastFraction);
  assertEqual(199, brightness);

  brightness = Renderer::calcBrightness(StyledDigit::kStyleBlinkFast,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOff,
      false, pulseSlowFraction, pulseFastFraction);
  assertEqual(0, brightness);

  brightness = Renderer::calcBrightness(StyledDigit::kStyleBlinkFast,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOn,
      false, pulseSlowFraction, pulseFastFraction);
  assertEqual(199, brightness);

  // enablePulse is false, so should return the overallBrightness
  brightness = Renderer::calcBrightness(StyledDigit::kStylePulseSlow,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOff,
      false, pulseSlowFraction, pulseFastFraction);
  assertEqual(199, brightness);

  // enablePulse is false, so should return the overallBrightness
  brightness = Renderer::calcBrightness(StyledDigit::kStylePulseFast,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOff,
      false, pulseSlowFraction, pulseFastFraction);
  assertEqual(199, brightness);

  // enablePulse is true, so should return a reduced brightness
  brightness = Renderer::calcBrightness(StyledDigit::kStylePulseSlow,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOff,
      true, pulseSlowFraction, pulseFastFraction);
  assertEqual(99, brightness);

  // enablePulse is false, so should return a reduced brightness
  brightness = Renderer::calcBrightness(StyledDigit::kStylePulseFast,
      overallBrightness, Renderer::kBlinkStateOff, Renderer::kBlinkStateOff,
      true, pulseSlowFraction, pulseFastFraction);
  assertEqual(48, brightness);
}

class RendererTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();
      fakeDriver.setNumSubFields(NUM_SUB_FIELDS);
      renderer
          .setFramesPerSecond(FRAMES_PER_SECOND)
          .setBrightness(255)
          .configure();

      hardware.clear();
    }

    void assertDimmingDigitsEqual(int n, ...) {
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t pattern = va_arg(args, int);
        uint8_t brightness = va_arg(args, int);
        const DimmingDigit& dimmingDigit = dimmingDigits[i];
        assertEqual(pattern, dimmingDigit.pattern);
        assertEqual(brightness, dimmingDigit.brightness);
      }
      va_end(args);
    }

    void fastForwardFields(int n) {
      while (n--) {
        renderer.renderField();
      }
    }

    Driver& driver = fakeDriver;
};

testF(RendererTest, frames_and_fields) {
  assertEqual(NUM_DIGITS, renderer.getNumDigits());
  assertEqual(FRAMES_PER_SECOND, renderer.getFramesPerSecond());
  assertEqual(NUM_DIGITS * NUM_SUB_FIELDS * FRAMES_PER_SECOND,
      renderer.getFieldsPerSecond());
}

testF(RendererTest, writePatternAt) {
  StyledDigit& styledDigit = styledDigits[0];

  renderer.writePatternAt(0, 0x11, StyledDigit::kStyleNormal);
  assertEqual(0x11, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer.writePatternAt(0, 0x22);
  assertEqual(0x22, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer.writeStyleAt(0, StyledDigit::kStyleBlinkFast);
  assertEqual(0x22, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleBlinkFast, styledDigit.style);
}

testF(RendererTest, writePatternAt_outOfBounds) {
  StyledDigit& styledDigit = styledDigits[4];
  styledDigit.pattern = 1;
  styledDigit.style = StyledDigit::kStyleNormal;

  renderer.writePatternAt(4, 0x11, StyledDigit::kStyleBlinkSlow);
  assertEqual(1, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigit.style);

  renderer.writePatternAt(4, 0x11);
  assertEqual(1, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigit.style);

  renderer.writeStyleAt(4, StyledDigit::kStyleBlinkFast);
  assertEqual(1, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigit.style);
}

testF(RendererTest, writeDecimalPointAt) {
  StyledDigit& styledDigit = styledDigits[0];

  renderer.writePatternAt(0, 0x11, StyledDigit::kStyleNormal);
  assertEqual(0x11, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer.writeDecimalPointAt(0);
  assertEqual(0x11 | 0x80, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer.writeDecimalPointAt(0, false);
  assertEqual(0x11, styledDigit.pattern);
  assertEqual(0, styledDigit.style);
}

testF(RendererTest, writeDecimalPointAt_outOfBounds) {
  StyledDigit& styledDigit = styledDigits[4];
  styledDigit.pattern = 1;
  styledDigit.style = StyledDigit::kStyleNormal;

  renderer.writeDecimalPointAt(4);
  assertEqual(1, styledDigit.pattern);
}

// Frame rates for various blinks and pulses at 60 fps, (4 * 3) fields/frame:
//  - blink slow: 48 frames/cycle = 4*3*48 = 576 fields/cycle
//  - blink fast: 24 frames/cycle = 4*3*24 = 288 fields/cycle
//  - pulse slow: 180 frames/cycle = 4*3*180 = 2160 fields/cycle
//  - pulse fast: 60 frames/cycle - 4*3*60 = 720 fields/cycle
testF(RendererTest, displayCurrentField) {
  renderer.writePatternAt(0, 0x11, StyledDigit::kStyleBlinkSlow);
  renderer.writePatternAt(1, 0x22, StyledDigit::kStyleBlinkFast);
  renderer.writePatternAt(2, 0x33, StyledDigit::kStylePulseSlow);
  renderer.writePatternAt(3, 0x44, StyledDigit::kStylePulseFast);

  assertEqual((uint16_t) (60 * 4 * 3), renderer.getFieldsPerSecond());

  renderer.renderField();
  assertDimmingDigitsEqual(4, 0x11, 255, 0x22, 255, 0x33, 0, 0x44, 0);

  fastForwardFields(156); // +13 frames = #13
  assertDimmingDigitsEqual(4, 0x11, 255, 0x22, 0, 0x33, 35, 0x44, 109);

  fastForwardFields(156); // +13 frames = #26
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 255, 0x33, 72, 0x44, 220);

  fastForwardFields(708); // +59 frames = #85
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 0, 0x33, 240, 0x44, 212);
}

// If the driver does not support subfields, then pulsing is disabled.
//  - blink slow: 48 frames/cycle = 4*48 = 192 fields/cycle
//  - blink fast: 24 frames/cycle = 4*24 = 96 fields/cycle
//  - pulse slow: 180 frames/cycle = 4*180 = 720 fields/cycle
//  - pulse fast: 60 frames/cycle - 4*60 = 240 fields/cycle
testF(RendererTest, displayCurrentField_noSubFieldDriver) {
  fakeDriver.setNumSubFields(1);
  assertEqual(false, fakeDriver.isBrightnessSupported());
  renderer.configure();

  renderer.writePatternAt(0, 0x11, StyledDigit::kStyleBlinkSlow);
  renderer.writePatternAt(1, 0x22, StyledDigit::kStyleBlinkFast);
  renderer.writePatternAt(2, 0x33, StyledDigit::kStylePulseSlow);
  renderer.writePatternAt(3, 0x44, StyledDigit::kStylePulseFast);

  assertEqual((uint16_t) (60 * 4 * 1), renderer.getFieldsPerSecond());

  renderer.renderField(); // #0
  assertDimmingDigitsEqual(4, 0x11, 255, 0x22, 255, 0x33, 255, 0x44, 255);

  fastForwardFields(52); // +13 frames = #13
  assertDimmingDigitsEqual(4, 0x11, 255, 0x22, 0, 0x33, 255, 0x44, 255);

  fastForwardFields(52); // +13 frames = #26
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 255, 0x33, 255, 0x44, 255);

  fastForwardFields(236); // +59 frames = #85
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 0, 0x33, 255, 0x44, 255);
}

testF(RendererTest, displayCurrentField_dimmedBrightness) {
  fakeDriver.setNumSubFields(3);
  assertEqual(true, fakeDriver.isBrightnessSupported());
  renderer
      .setBrightness(127)
      .configure();

  renderer.writePatternAt(0, 0x11, StyledDigit::kStyleBlinkSlow);
  renderer.writePatternAt(1, 0x22, StyledDigit::kStyleBlinkFast);
  renderer.writePatternAt(2, 0x33, StyledDigit::kStylePulseSlow);
  renderer.writePatternAt(3, 0x44, StyledDigit::kStylePulseFast);

  assertEqual((uint16_t) (60 * 4 * 3), renderer.getFieldsPerSecond());

  // Verify that the digit brightness is about 1/2 the value of
  // the testF(RendererTest, displayCurrentField) test case.

  renderer.renderField();
  assertDimmingDigitsEqual(4, 0x11, 127, 0x22, 127, 0x33, 0, 0x44, 0);

  fastForwardFields(156); // +13 frames = #13
  assertDimmingDigitsEqual(4, 0x11, 127, 0x22, 0, 0x33, 17, 0x44, 54);

  fastForwardFields(156); // +13 frames = #26
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 127, 0x33, 36, 0x44, 109);

  fastForwardFields(708); // +59 frames = #85
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 0, 0x33, 119, 0x44, 105);
}

// ----------------------------------------------------------------------
// Tests for CharWriter.
// ----------------------------------------------------------------------

class CharWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();
      fakeDriver.setNumSubFields(NUM_SUB_FIELDS);
      renderer
          .setFramesPerSecond(FRAMES_PER_SECOND)
          .setBrightness(255)
          .configure();
      hardware.clear();
    }
};

testF(CharWriterTest, getNumDigits) {
  assertEqual(NUM_DIGITS, charWriter.getNumDigits());
}

testF(CharWriterTest, writeAt) {
  charWriter.writeCharAt(0, '0');
  assertEqual(0b00111111, styledDigits[0].pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigits[0].style);

  charWriter.writeCharAt(1, '0', StyledDigit::kStyleBlinkSlow);
  assertEqual(0b00111111, styledDigits[1].pattern);
  assertEqual(StyledDigit::kStyleBlinkSlow, styledDigits[1].style);

  charWriter.writeStyleAt(1, StyledDigit::kStyleBlinkFast);
  assertEqual(StyledDigit::kStyleBlinkFast, styledDigits[1].style);

  charWriter.writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, styledDigits[1].pattern);

  charWriter.writeDecimalPointAt(1, false);
  assertEqual(0b00111111, styledDigits[1].pattern);
}

testF(CharWriterTest, writeAt_outOfBounds) {
  styledDigits[4].pattern = 0;
  styledDigits[4].style = StyledDigit::kStyleNormal;
  charWriter.writeCharAt(4, 'a', StyledDigit::kStyleBlinkSlow);
  charWriter.writeDecimalPointAt(4);
  assertEqual(0, styledDigits[4].pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigits[4].style);
}

// ----------------------------------------------------------------------
// Tests for StringWriter.
// ----------------------------------------------------------------------

class StringWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();
      fakeDriver.setNumSubFields(NUM_SUB_FIELDS);
      renderer
          .setFramesPerSecond(FRAMES_PER_SECOND)
          .setBrightness(255)
          .configure();
      hardware.clear();
      clearStyledDigits();
    }

    void assertStyledDigitsEqual(int n, ...) {
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t pattern = va_arg(args, int);
        uint8_t style = va_arg(args, int);
        const StyledDigit& styledDigit = styledDigits[i];
        assertEqual(pattern, styledDigit.pattern);
        assertEqual(style, styledDigit.style);
      }
      va_end(args);
    }
};

testF(StringWriterTest, writeStringAt) {
  stringWriter.writeStringAt(0, ".1.2.3");

  // Should ge (".", "1.", "2.", "3") as the 4 digits
  assertStyledDigitsEqual(4,
    0b10000000, StyledDigit::kStyleNormal,
    0b00000110 | 0x80, StyledDigit::kStyleNormal,
    0b01011011 | 0x80, StyledDigit::kStyleNormal,
    0b01001111, StyledDigit::kStyleNormal);
}
