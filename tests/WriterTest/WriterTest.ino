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
#include <AUnit.h>
#include <AceSegment.h>
#include <ace_segment/testing/FakeRenderer.h>
#include <ace_segment/testing/StubStyler.h>

using namespace aunit;
using namespace ace_segment;
using namespace ace_segment::testing;

const uint8_t NUM_DIGITS = 4;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmablePattern dimmablePatterns[NUM_DIGITS + 1];
StyledPattern styledPatterns[NUM_DIGITS + 1];

const uint8_t STYLE_BLINK = 1;
const uint8_t STYLE_PULSE = 2;

Styler* blinkStyler = new StubStyler();
Styler* pulseStyler = new StubStyler();

Styler* stylers[Renderer::kNumStyles];

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): start"));

  memset(stylers, 0, Renderer::kNumStyles * sizeof(Styler*));
  stylers[STYLE_BLINK] = blinkStyler;
  stylers[STYLE_PULSE] = pulseStyler;

  Serial.println(F("setup(): end"));
}

void loop() {
  TestRunner::run();
}

void clearStyledPatterns() {
  for (int i = 0; i < NUM_DIGITS; i++) {
    styledPatterns[i].pattern = 0;
    styledPatterns[i].style = 0;
  }
}

// ----------------------------------------------------------------------
// Tests for CharWriter.
// ----------------------------------------------------------------------

class CharWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();

      mRenderer = new FakeRenderer(styledPatterns, NUM_DIGITS, stylers);
      mRenderer->configure();
      mCharWriter = new CharWriter(mRenderer);
      clearStyledPatterns();
    }

    virtual void teardown() override {
      delete mCharWriter;
      delete mRenderer;

      TestOnce::teardown();
    }

    FakeRenderer *mRenderer;
    CharWriter *mCharWriter;
};

testF(CharWriterTest, getNumDigits) {
  assertEqual(NUM_DIGITS, mCharWriter->getNumDigits());
}

testF(CharWriterTest, writeAt) {
  mCharWriter->writeCharAt(0, '0');
  assertEqual(0b00111111, styledPatterns[0].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[0].style);

  mCharWriter->writeCharAt(1, '0', STYLE_BLINK);
  assertEqual(0b00111111, styledPatterns[1].pattern);
  assertEqual(STYLE_BLINK, styledPatterns[1].style);

  mCharWriter->writeStyleAt(1, STYLE_PULSE);
  assertEqual(STYLE_PULSE, styledPatterns[1].style);

  mCharWriter->writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, styledPatterns[1].pattern);

  mCharWriter->writeDecimalPointAt(1, false);
  assertEqual(0b00111111, styledPatterns[1].pattern);
}

testF(CharWriterTest, writeAt_outOfBounds) {
  styledPatterns[4].pattern = 0;
  styledPatterns[4].style = StyledPattern::kStyleNormal;
  mCharWriter->writeCharAt(4, 'a', STYLE_BLINK);
  mCharWriter->writeDecimalPointAt(4);
  assertEqual(0, styledPatterns[4].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[4].style);
}

// ----------------------------------------------------------------------
// Tests for StringWriter.
// ----------------------------------------------------------------------

class StringWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();

      mRenderer = new FakeRenderer(styledPatterns, NUM_DIGITS, stylers);
      mRenderer->configure();
      mCharWriter = new CharWriter(mRenderer);
      mStringWriter = new StringWriter(mCharWriter);
      clearStyledPatterns();
    }

    virtual void teardown() override {
      delete mStringWriter;
      delete mCharWriter;
      delete mRenderer;

      TestOnce::teardown();
    }

    void assertStyledPatternsEqual(int n, ...) {
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t pattern = va_arg(args, int);
        uint8_t style = va_arg(args, int);
        const StyledPattern& styledPattern = styledPatterns[i];
        assertEqual(pattern, styledPattern.pattern);
        assertEqual(style, styledPattern.style);
      }
      va_end(args);
    }

    FakeRenderer *mRenderer;
    CharWriter *mCharWriter;
    StringWriter *mStringWriter;
};

testF(StringWriterTest, writeStringAt) {
  mStringWriter->writeStringAt(0, ".1.2.3");

  // Should ge (".", "1.", "2.", "3") as the 4 digits
  assertStyledPatternsEqual(4,
    0b10000000, StyledPattern::kStyleNormal,
    0b00000110 | 0x80, StyledPattern::kStyleNormal,
    0b01011011 | 0x80, StyledPattern::kStyleNormal,
    0b01001111, StyledPattern::kStyleNormal);
}

// ----------------------------------------------------------------------
// Tests for HexWriter.
// ----------------------------------------------------------------------

class HexWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();

      mRenderer = new FakeRenderer(styledPatterns, NUM_DIGITS, stylers);
      mRenderer->configure();
      mHexWriter = new HexWriter(mRenderer);
      clearStyledPatterns();
    }

    virtual void teardown() override {
      delete mHexWriter;
      delete mRenderer;

      TestOnce::teardown();
    }

    FakeRenderer *mRenderer;
    HexWriter *mHexWriter;
};

testF(HexWriterTest, getNumDigits) {
  assertEqual(NUM_DIGITS, mHexWriter->getNumDigits());
}

testF(HexWriterTest, writeAt) {
  mHexWriter->writeHexAt(0, 0);
  assertEqual(0b00111111, styledPatterns[0].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[0].style);

  mHexWriter->writeHexAt(1, 0, STYLE_BLINK);
  assertEqual(0b00111111, styledPatterns[1].pattern);
  assertEqual(STYLE_BLINK, styledPatterns[1].style);

  mHexWriter->writeStyleAt(1, STYLE_PULSE);
  assertEqual(STYLE_PULSE, styledPatterns[1].style);

  mHexWriter->writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, styledPatterns[1].pattern);

  mHexWriter->writeDecimalPointAt(1, false);
  assertEqual(0b00111111, styledPatterns[1].pattern);
}

testF(HexWriterTest, writeAt_outOfBounds) {
  styledPatterns[4].pattern = 0;
  styledPatterns[4].style = StyledPattern::kStyleNormal;
  mHexWriter->writeHexAt(4, HexWriter::kMinus, STYLE_BLINK);
  mHexWriter->writeDecimalPointAt(4);
  assertEqual(0, styledPatterns[4].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[4].style);
}

// ----------------------------------------------------------------------
// Tests for ClockWriter.
// ----------------------------------------------------------------------

class ClockWriterTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();

      mRenderer = new FakeRenderer(styledPatterns, NUM_DIGITS, stylers);
      mRenderer->configure();
      mClockWriter = new ClockWriter(mRenderer);
      clearStyledPatterns();
    }

    virtual void teardown() override {
      delete mClockWriter;
      delete mRenderer;

      TestOnce::teardown();
    }

    FakeRenderer *mRenderer;
    ClockWriter *mClockWriter;
};

testF(ClockWriterTest, toBcd) {
  assertEqual(0x00, ClockWriter::toBcd(00));
  assertEqual(0x42, ClockWriter::toBcd(42));
  assertEqual(0x99, ClockWriter::toBcd(99));
  assertEqual(0x99, ClockWriter::toBcd(100));
}

testF(ClockWriterTest, writeBcdClock) {
  mClockWriter->writeBcdClock(0x12, 0x30);
  assertEqual(0b00000110, styledPatterns[0].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[0].style);
  assertEqual(0b01011011 | 0x80, styledPatterns[1].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[1].style);
  assertEqual(0b01001111, styledPatterns[2].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[2].style);
  assertEqual(0b00111111, styledPatterns[3].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[3].style);
}

testF(ClockWriterTest, writeClock) {
  mClockWriter->writeClock(12, 30);
  assertEqual(0b00000110, styledPatterns[0].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[0].style);
  assertEqual(0b01011011 | 0x80, styledPatterns[1].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[1].style);
  assertEqual(0b01001111, styledPatterns[2].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[2].style);
  assertEqual(0b00111111, styledPatterns[3].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[3].style);
}

testF(ClockWriterTest, writeCharAt) {
  mClockWriter->writeCharAt(1, ClockWriter::kP);
  assertEqual(0b01110011, styledPatterns[1].pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPatterns[1].style);

  mClockWriter->writeStyleAt(1, STYLE_BLINK);
  assertEqual(STYLE_BLINK, styledPatterns[1].style);
}

testF(ClockWriterTest, writeColon) {
  mClockWriter->writeClock(12, 30); // colon on by default
  assertEqual(0b01011011 | 0x80, styledPatterns[1].pattern);

  mClockWriter->writeColon(false); // turns it off
  assertEqual(0b01011011, styledPatterns[1].pattern);
}
