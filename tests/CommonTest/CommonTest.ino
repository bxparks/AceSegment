#line 2 "CommonTest.ino"

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
#include <ace_segment/DigitDriver.h>
#include <ace_segment/ModulatingDigitDriver.h>
#include <ace_segment/SegmentDriver.h>
#include <ace_segment/TimingStats.h>
#include <ace_segment/testing/FakeDriver.h>

using namespace aunit;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t NUM_DIGITS = 4;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmingDigit dimmingDigits[NUM_DIGITS + 1];

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): start"));

  Serial.print(F("sizeof(Hardware): "));
  Serial.println(sizeof(Hardware));
  Serial.print(F("sizeof(LedMatrixDirect): "));
  Serial.println(sizeof(LedMatrixDirect));
  Serial.print(F("sizeof(LedMatrixSerial): "));
  Serial.println(sizeof(LedMatrixSerial));
  Serial.print(F("sizeof(LedMatrixSpi): "));
  Serial.println(sizeof(LedMatrixSpi));
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

  Serial.println(F("setup(): end"));
}

void loop() {
  TestRunner::run();
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
// Tests for TimingStats.
// ----------------------------------------------------------------------

class TimingStatsTest: public TestOnce {
  protected:
    virtual void setup() override {
      mStats = new TimingStats();
    }

    virtual void teardown() override {
      delete mStats;
    }

    TimingStats* mStats;
};

testF(TimingStatsTest, reset) {
  assertEqual(0U, mStats->getCount());
  assertEqual(0U, mStats->getCounter());
  assertEqual(0xffffU, mStats->getMin());
  assertEqual(0U, mStats->getMax());
  assertEqual(0U, mStats->getAvg());
  assertEqual(0U, mStats->getExpDecayAvg());
}

testF(TimingStatsTest, update) {
  mStats->update(1);
  mStats->update(3);
  mStats->update(10);

  assertEqual(3U, mStats->getCount());
  assertEqual(3U, mStats->getCounter());
  assertEqual(1U, mStats->getMin());
  assertEqual(10U, mStats->getMax());
  assertEqual(4U, mStats->getAvg());
  assertEqual(5U, mStats->getExpDecayAvg());

  mStats->reset();
  assertEqual(0U, mStats->getCount());
  assertEqual(3U, mStats->getCounter());
}

// ----------------------------------------------------------------------
// Test FakeDriver to test some common methods of Driver and to be sure that
// the fake version works.
// ----------------------------------------------------------------------

class FakeDriverTest: public TestOnce {
  protected:
    virtual void setup() override {
      mDriver = new FakeDriver(dimmingDigits, NUM_DIGITS);
    }

    virtual void teardown() override {
      delete mDriver;
    }

    FakeDriver* mDriver;
};

testF(FakeDriverTest, setPattern) {
  const DimmingDigit& dimmingDigit = dimmingDigits[0];

  mDriver->setPattern(0, 0x11);
  assertEqual(0x11, dimmingDigit.pattern);
  assertEqual(255, dimmingDigit.brightness);

  mDriver->setPattern(0, 0x11, 10);
  assertEqual(0x11, dimmingDigit.pattern);
  assertEqual(10, dimmingDigit.brightness);
}

// Setting a digit that's out of bounds (>= NUM_DIGITS) does nothing
testF(FakeDriverTest, setPattern_outOfBounds) {
  DimmingDigit& digitOutOfBounds = dimmingDigits[4];
  digitOutOfBounds.pattern = 1;
  digitOutOfBounds.brightness = 2;
  mDriver->setPattern(5, 0x11, 10);
  assertEqual(1, digitOutOfBounds.pattern);
  assertEqual(2, digitOutOfBounds.brightness);
}

testF(FakeDriverTest, setBrightness) {
  const DimmingDigit& dimmingDigit = dimmingDigits[0];

  mDriver->setPattern(0, 0x11);
  assertEqual(0x11, dimmingDigit.pattern);
  assertEqual(255, dimmingDigit.brightness);

  mDriver->setBrightness(0, 20);
  assertEqual(0x11, dimmingDigit.pattern);
  assertEqual(20, dimmingDigit.brightness);
}

// Setting a digit that's out of bounds (>= NUM_DIGITS) does nothing
testF(FakeDriverTest, setBrightness_outOfBounds) {
  DimmingDigit& digitOutOfBounds = dimmingDigits[4];
  digitOutOfBounds.pattern = 1;
  digitOutOfBounds.brightness = 2;
  mDriver->setBrightness(4, 96);
  assertEqual(1, digitOutOfBounds.pattern);
  assertEqual(2, digitOutOfBounds.brightness);
}

