#line 2 "LedMatrixTest.ino"

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
#include <ace_segment/LedMatrixDirect.h>
#include <ace_segment/LedMatrixSerial.h>
#include <ace_segment/LedMatrixSpi.h>
#include <ace_segment/testing/TestableHardware.h>
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
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
const uint8_t latchPin = 12;
const uint8_t dataPin = 13;
const uint8_t clockPin = 14;

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
}

void loop() {
  TestRunner::run();
}

// ----------------------------------------------------------------------
// Tests for LedMatrixDirect.
// ----------------------------------------------------------------------

class LedMatrixDirectTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mLedMatrix = new LedMatrixDirect(mHardware, true /* cathodeOnGroup */,
          false /* useTransistorsOnGroups */,
          false /* useTransistorsOnElements */,
          NUM_DIGITS, NUM_SEGMENTS, digitPins, segmentPins);

      mLedMatrix->configure();
      mHardware->clear();
    }

    virtual void teardown() override {
      delete mLedMatrix;
      BaseHardwareTest::teardown();
    }

    LedMatrixDirect* mLedMatrix;
};

testF(LedMatrixDirectTest, configure) {
  mLedMatrix->configure();
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

  mHardware->clear();
  mLedMatrix->finish();
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

testF(LedMatrixDirectTest, enableGroup) {
  mLedMatrix->enableGroup(1);
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, LOW);
}

testF(LedMatrixDirectTest, disableGroup) {
  mLedMatrix->disableGroup(1);
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, HIGH);
}

testF(LedMatrixDirectTest, drawElements) {
  mLedMatrix->drawElements(0x55);
  assertEvents(8,
      Event::kTypeDigitalWrite, 4, HIGH,
      Event::kTypeDigitalWrite, 5, LOW,
      Event::kTypeDigitalWrite, 6, HIGH,
      Event::kTypeDigitalWrite, 7, LOW,
      Event::kTypeDigitalWrite, 8, HIGH,
      Event::kTypeDigitalWrite, 9, LOW,
      Event::kTypeDigitalWrite, 10, HIGH,
      Event::kTypeDigitalWrite, 11, LOW
  );
}

// ----------------------------------------------------------------------
// Tests for LedMatrixSerial.
// ----------------------------------------------------------------------

class LedMatrixSerialTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mLedMatrix = new LedMatrixSerial(mHardware, true /* cathodeOnGroup */,
          false /* useTransistorsOnGroups */,
          false /* useTransistorsOnElements */,
          NUM_DIGITS, NUM_SEGMENTS, digitPins, latchPin, dataPin, clockPin);

      mLedMatrix->configure();
      mHardware->clear();
    }

    virtual void teardown() override {
      delete mLedMatrix;
      BaseHardwareTest::teardown();
    }

    LedMatrixSerial* mLedMatrix;
};

testF(LedMatrixSerialTest, configure) {
  mLedMatrix->configure();
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

  mHardware->clear();
  mLedMatrix->finish();
  assertEvents(7,
      Event::kTypePinMode, latchPin, INPUT,
      Event::kTypePinMode, dataPin, INPUT,
      Event::kTypePinMode, clockPin, INPUT,
      Event::kTypePinMode, 0, INPUT,
      Event::kTypePinMode, 1, INPUT,
      Event::kTypePinMode, 2, INPUT,
      Event::kTypePinMode, 3, INPUT);

}

testF(LedMatrixSerialTest, enableGroup) {
  mLedMatrix->enableGroup(1);
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, LOW);
}

testF(LedMatrixSerialTest, disableGroup) {
  mLedMatrix->disableGroup(1);
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, HIGH);
}

testF(LedMatrixSerialTest, drawElements) {
  mLedMatrix->drawElements(0x55);
  assertEvents(3,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeShiftOut, dataPin, clockPin, MSBFIRST, 0x55,
      Event::kTypeDigitalWrite, latchPin, HIGH
  );
}

// ----------------------------------------------------------------------
// Tests for LedMatrixSpi.
// ----------------------------------------------------------------------

class LedMatrixSpiTest: public BaseHardwareTest {
  protected:
    virtual void setup() override {
      BaseHardwareTest::setup();
      mLedMatrix = new LedMatrixSpi(mHardware,
          true /* cathodeOnGroup */, false /* useTransistorsOnGroups */,
          false /* useTransistorsOnElements */,
          NUM_DIGITS, NUM_SEGMENTS, digitPins, latchPin, dataPin, clockPin);

      mLedMatrix->configure();
      mHardware->clear();
    }

    virtual void teardown() override {
      delete mLedMatrix;
      BaseHardwareTest::teardown();
    }

    LedMatrixSpi* mLedMatrix;
};

testF(LedMatrixSpiTest, configure) {
  mLedMatrix->configure();
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

  mHardware->clear();
  mLedMatrix->finish();
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

testF(LedMatrixSpiTest, enableGroup) {
  mLedMatrix->enableGroup(1);
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, LOW);
}

testF(LedMatrixSpiTest, disableGroup) {
  mLedMatrix->disableGroup(1);
  assertEvents(1,
      Event::kTypeDigitalWrite, 1, HIGH);
}

testF(LedMatrixSpiTest, drawElements) {
  mLedMatrix->drawElements(0x55);
  assertEvents(3,
      Event::kTypeDigitalWrite, latchPin, LOW,
      Event::kTypeSpiTransfer, 0x55,
      Event::kTypeDigitalWrite, latchPin, HIGH
  );
}

