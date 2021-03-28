#line 2 "WriterTest.ino"

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
#include <ace_segment/testing/FakeRenderer.h>

using namespace aunit;
using namespace ace_segment;
using namespace ace_segment::testing;

const uint8_t CUSTOM_BRIGHTNESS = 64;
const uint8_t DIFFERENT_BRIGHTNESS = 32;

// ----------------------------------------------------------------------
// Tests for BaseWriterTest.
// ----------------------------------------------------------------------

class BaseWriterTest: public TestOnce {
  protected:
    static const uint8_t NUM_DIGITS = 4;

    void setup() override {
      mRenderer = new FakeRenderer(mDimmablePatterns, NUM_DIGITS);
      mRenderer->configure();
    }

    void teardown() override {
      delete mRenderer;
    }

  protected:
    FakeRenderer *mRenderer;

    // create NUM_DIGITS+1 elements for doing array bound checking
    DimmablePattern mDimmablePatterns[NUM_DIGITS + 1];
};

// ----------------------------------------------------------------------
// Tests for CharWriter.
// ----------------------------------------------------------------------

class CharWriterTest: public BaseWriterTest {
  protected:
    void setup() override {
      BaseWriterTest::setup();
      mCharWriter = new CharWriter(mRenderer);
    }

    void teardown() override {
      delete mCharWriter;
      BaseWriterTest::teardown();
    }

  protected:
    CharWriter *mCharWriter;
};

testF(CharWriterTest, getNumDigits) {
  assertEqual(NUM_DIGITS, mCharWriter->getNumDigits());
}

testF(CharWriterTest, writeAt) {
  mCharWriter->writeCharAt(0, '0');
  assertEqual(0b00111111, mDimmablePatterns[0].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[0].brightness);

  mCharWriter->writeCharAt(1, '0', CUSTOM_BRIGHTNESS);
  assertEqual(0b00111111, mDimmablePatterns[1].pattern);
  assertEqual(CUSTOM_BRIGHTNESS, mDimmablePatterns[1].brightness);

  mCharWriter->writeBrightnessAt(1, DIFFERENT_BRIGHTNESS);
  assertEqual(DIFFERENT_BRIGHTNESS, mDimmablePatterns[1].brightness);

  mCharWriter->writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, mDimmablePatterns[1].pattern);

  mCharWriter->writeDecimalPointAt(1, false);
  assertEqual(0b00111111, mDimmablePatterns[1].pattern);
}

testF(CharWriterTest, writeAt_outOfBounds) {
  mDimmablePatterns[4].pattern = 0;
  mDimmablePatterns[4].brightness = CUSTOM_BRIGHTNESS;

  mCharWriter->writeCharAt(4, 'a', DIFFERENT_BRIGHTNESS);
  mCharWriter->writeDecimalPointAt(4);
  assertEqual(0, mDimmablePatterns[4].pattern);
  assertEqual(CUSTOM_BRIGHTNESS, mDimmablePatterns[4].brightness);
}

// ----------------------------------------------------------------------
// Tests for StringWriter.
// ----------------------------------------------------------------------

class StringWriterTest: public BaseWriterTest {
  protected:
    void setup() override {
      BaseWriterTest::setup();
      mCharWriter = new CharWriter(mRenderer);
      mStringWriter = new StringWriter(mCharWriter);
    }

    void teardown() override {
      delete mStringWriter;
      delete mCharWriter;
      BaseWriterTest::teardown();
    }

    void assertDimmablePatternsEqual(int n, ...) {
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t pattern = va_arg(args, int);
        uint8_t brightness = va_arg(args, int);
        const DimmablePattern& dimmablePattern = mDimmablePatterns[i];
        assertEqual(pattern, dimmablePattern.pattern);
        assertEqual(brightness, dimmablePattern.brightness);
      }
      va_end(args);
    }

    CharWriter *mCharWriter;
    StringWriter *mStringWriter;
};

testF(StringWriterTest, writeStringAt) {
  mStringWriter->writeStringAt(0, ".1.2.3");

  // Should be (".", "1.", "2.", "3") as the 4 digits
  assertDimmablePatternsEqual(
    4,
    0b10000000, Renderer::kDefaultBrightness,
    0b00000110 | 0x80, Renderer::kDefaultBrightness,
    0b01011011 | 0x80, Renderer::kDefaultBrightness,
    0b01001111, Renderer::kDefaultBrightness
  );
}

// ----------------------------------------------------------------------
// Tests for HexWriter.
// ----------------------------------------------------------------------

class HexWriterTest: public BaseWriterTest {
  protected:
    void setup() override {
      BaseWriterTest::setup();
      mHexWriter = new HexWriter(mRenderer);
    }

    void teardown() override {
      delete mHexWriter;
      BaseWriterTest::teardown();
    }

    HexWriter *mHexWriter;
};

testF(HexWriterTest, getNumDigits) {
  assertEqual(NUM_DIGITS, mHexWriter->getNumDigits());
}

testF(HexWriterTest, writeAt) {
  mHexWriter->writeHexAt(0, 0);
  assertEqual(0b00111111, mDimmablePatterns[0].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[0].brightness);

  mHexWriter->writeHexAt(1, 0, CUSTOM_BRIGHTNESS);
  assertEqual(0b00111111, mDimmablePatterns[1].pattern);
  assertEqual(CUSTOM_BRIGHTNESS, mDimmablePatterns[1].brightness);

  mHexWriter->writeBrightnessAt(1, DIFFERENT_BRIGHTNESS);
  assertEqual(DIFFERENT_BRIGHTNESS, mDimmablePatterns[1].brightness);

  mHexWriter->writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, mDimmablePatterns[1].pattern);

  mHexWriter->writeDecimalPointAt(1, false);
  assertEqual(0b00111111, mDimmablePatterns[1].pattern);
}

testF(HexWriterTest, writeAt_outOfBounds) {
  mDimmablePatterns[4].pattern = 0;
  mDimmablePatterns[4].brightness = CUSTOM_BRIGHTNESS;

  mHexWriter->writeHexAt(4, HexWriter::kMinus, DIFFERENT_BRIGHTNESS);
  mHexWriter->writeDecimalPointAt(4);
  assertEqual(0, mDimmablePatterns[4].pattern);
  assertEqual(CUSTOM_BRIGHTNESS, mDimmablePatterns[4].brightness);
}

// ----------------------------------------------------------------------
// Tests for ClockWriter.
// ----------------------------------------------------------------------

class ClockWriterTest: public BaseWriterTest {
  protected:
    void setup() override {
      BaseWriterTest::setup();
      mClockWriter = new ClockWriter(mRenderer);
    }

    void teardown() override {
      delete mClockWriter;
      BaseWriterTest::teardown();
    }

    ClockWriter *mClockWriter;
};

testF(ClockWriterTest, toBcd) {
  assertEqual(0x00, ClockWriter::toBcd(00));
  assertEqual(0x42, ClockWriter::toBcd(42));
  assertEqual(0x99, ClockWriter::toBcd(99));
  assertEqual(0xFF, ClockWriter::toBcd(100));
}

testF(ClockWriterTest, writeCharAt) {
  mClockWriter->writeCharAt(0, ClockWriter::kP);
  assertEqual(0b01110011, mDimmablePatterns[0].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[0].brightness);

  mClockWriter->writeCharAt(1, ClockWriter::kP, CUSTOM_BRIGHTNESS);
  assertEqual(0b01110011, mDimmablePatterns[1].pattern);
  assertEqual(CUSTOM_BRIGHTNESS, mDimmablePatterns[1].brightness);

  mClockWriter->writeBrightnessAt(1, DIFFERENT_BRIGHTNESS);
  assertEqual(DIFFERENT_BRIGHTNESS, mDimmablePatterns[1].brightness);
}

testF(ClockWriterTest, writeBcdAt) {
  mClockWriter->writeBcdAt(0, 0x12);
  assertEqual(0b00000110, mDimmablePatterns[0].pattern);
  assertEqual(0b01011011, mDimmablePatterns[1].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[0].brightness);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[1].brightness);

  mClockWriter->writeBcdAt(2, 0xAF);
  assertEqual(0x0, mDimmablePatterns[2].pattern);
  assertEqual(0x0, mDimmablePatterns[3].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[2].brightness);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[3].brightness);
}

testF(ClockWriterTest, writeDecimalAt) {
  mClockWriter->writeDecimalAt(0, 12);
  assertEqual(0b00000110, mDimmablePatterns[0].pattern);
  assertEqual(0b01011011, mDimmablePatterns[1].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[0].brightness);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[1].brightness);

  mClockWriter->writeDecimalAt(2, 100);
  assertEqual(0x0, mDimmablePatterns[2].pattern);
  assertEqual(0x0, mDimmablePatterns[3].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[2].brightness);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[3].brightness);
}

testF(ClockWriterTest, writeBcdClock) {
  mClockWriter->writeBcdClock(0x12, 0x30);
  assertEqual(0b00000110, mDimmablePatterns[0].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[0].brightness);
  assertEqual(0b01011011 | 0x80, mDimmablePatterns[1].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[1].brightness);
  assertEqual(0b01001111, mDimmablePatterns[2].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[2].brightness);
  assertEqual(0b00111111, mDimmablePatterns[3].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[3].brightness);
}

testF(ClockWriterTest, writeClock) {
  mClockWriter->writeClock(12, 30);
  assertEqual(0b00000110, mDimmablePatterns[0].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[0].brightness);
  assertEqual(0b01011011 | 0x80, mDimmablePatterns[1].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[1].brightness);
  assertEqual(0b01001111, mDimmablePatterns[2].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[2].brightness);
  assertEqual(0b00111111, mDimmablePatterns[3].pattern);
  assertEqual(Renderer::kDefaultBrightness, mDimmablePatterns[3].brightness);
}

testF(ClockWriterTest, writeColon) {
  mClockWriter->writeClock(12, 30); // colon on by default
  assertEqual(0b01011011 | 0x80, mDimmablePatterns[1].pattern);

  mClockWriter->writeColon(false); // turns it off
  assertEqual(0b01011011, mDimmablePatterns[1].pattern);
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
}

void loop() {
  TestRunner::run();
}
