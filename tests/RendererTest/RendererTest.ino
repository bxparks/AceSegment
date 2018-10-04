#line 2 "RendererTest.ino"

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
#include <ace_segment/testing/TestableHardware.h>
#include <ace_segment/testing/FakeDriver.h>

using namespace aunit;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t NUM_DIGITS = 4;
const uint16_t FRAMES_PER_SECOND = 60;
const int8_t NUM_SUB_FIELDS = 3;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmablePattern dimmablePatterns[NUM_DIGITS + 1];
StyledPattern styledPatterns[NUM_DIGITS + 1];

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): start"));

  //TestRunner::exclude("*");
  //TestRunner::include("RendererTest", "activeStyles");

  Serial.println(F("setup(): end"));
}

void loop() {
  TestRunner::run();
}

// ----------------------------------------------------------------------
// Tests for various Stylers.
// ----------------------------------------------------------------------

class BlinkStylerTest: public TestOnce {
  protected:
    void setup() override {
      TestOnce::setup();
      blinkStyler = new BlinkStyler(30 /* fps */, 1000 /* durationMillis */);
    }

    void teardown() override {
      delete blinkStyler;
      TestOnce::teardown();
    }

    BlinkStyler* blinkStyler;
};

testF(BlinkStylerTest, test) {
  uint8_t pattern = 0x5C;
  uint8_t brightness = 128;

  for (uint8_t i = 0; i < 15; i++) {
    blinkStyler->calcForFrame();
    blinkStyler->apply(&pattern, &brightness);
    assertEqual(brightness, 128);
  }
  for (uint8_t i = 0; i < 15; i++) {
    blinkStyler->calcForFrame();
    blinkStyler->apply(&pattern, &brightness);
    assertEqual(brightness, 0);
  }
}

class PulseStylerTest: public TestOnce {
  protected:
    void setup() override {
      TestOnce::setup();
      pulseStyler = new PulseStyler(30 /* fps */, 1000 /* durationMillis */);
    }

    void teardown() override {
      delete pulseStyler;
      TestOnce::teardown();
    }

    void assertBrightness(uint8_t pattern, uint8_t brightness, uint8_t n, ...) {
      va_list args;
      va_start(args, n);
      for (uint8_t i = 0; i < n; i++) {
        uint8_t newPattern = pattern;
        uint8_t newBrightness = brightness;
        pulseStyler->calcForFrame();
        pulseStyler->apply(&newPattern, &newBrightness);

        uint8_t level = va_arg(args, int);
        assertEqual(level, newBrightness);
      }
      va_end(args);
    }

    PulseStyler* pulseStyler;
};

testF(PulseStylerTest, test) {
  uint8_t pattern = 0x5C;
  uint8_t brightness = 128;

  // pulse up
  assertBrightness(pattern, brightness, 15,
      0, 8, 17, 25, 34, 42, 51, 59, 68, 76, 85, 93, 102, 110, 119);

  // pulse down
  assertBrightness(pattern, brightness, 15,
      119, 110, 102, 93, 85, 76, 68, 59, 51, 42, 34, 25, 17, 8, 0);
}

// ----------------------------------------------------------------------
// Tests for Renderer.
// ----------------------------------------------------------------------

class RendererTest: public TestOnce {
  protected:
    static const uint16_t kBlinkSlowDurationMillis = 800;
    static const uint16_t kBlinkFastDurationMillis = 400;
    static const uint16_t kPulseSlowDurationMillis = 3000;
    static const uint16_t kPulseFastDurationMillis = 1000;
    static const uint8_t kStyleBlinkSlow = 1;
    static const uint8_t kStyleBlinkFast = 2;
    static const uint8_t kStylePulseSlow = 3;
    static const uint8_t kStylePulseFast = 4;
    static const uint16_t kStatsResetInterval = 1200;

    void setup() override {
      TestOnce::setup();

      memset(dimmablePatterns, 0, (NUM_DIGITS+1) * sizeof(DimmablePattern));
      memset(styledPatterns, 0, (NUM_DIGITS+1) * sizeof(StyledPattern));

      hardware = new TestableHardware();

      driver = new FakeDriver(dimmablePatterns, NUM_DIGITS);
      driver->setNumSubFields(NUM_SUB_FIELDS);
      driver->configure();

      blinkSlow = new BlinkStyler(FRAMES_PER_SECOND, kBlinkSlowDurationMillis);
      blinkFast = new BlinkStyler(FRAMES_PER_SECOND, kBlinkFastDurationMillis);
      pulseSlow = new PulseStyler(FRAMES_PER_SECOND, kPulseSlowDurationMillis);
      pulseFast = new PulseStyler(FRAMES_PER_SECOND, kPulseFastDurationMillis);

      styleTable = new StyleTable();
      styleTable->setStyler(kStyleBlinkSlow, blinkSlow);
      styleTable->setStyler(kStyleBlinkFast, blinkFast);
      styleTable->setStyler(kStylePulseSlow, pulseSlow);
      styleTable->setStyler(kStylePulseFast, pulseFast);

      renderer = new Renderer(hardware, driver, styledPatterns, styleTable,
          NUM_DIGITS, FRAMES_PER_SECOND, kStatsResetInterval);
      renderer->writeBrightness(255);
      renderer->configure();

      hardware->clear();
    }

    void teardown() override {
      delete renderer;
      delete styleTable;
      delete pulseFast;
      delete pulseSlow;
      delete blinkFast;
      delete blinkSlow;
      delete driver;
      delete hardware;
      TestOnce::teardown();
    }

    void assertDimmablePatternsEqual(int n, ...) {
      assertEqual(NUM_DIGITS, n);
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t pattern = va_arg(args, int);
        uint8_t brightness = va_arg(args, int);
        const DimmablePattern& dimmablePattern = dimmablePatterns[i];
        assertEqual(pattern, dimmablePattern.pattern);
        assertEqual(brightness, dimmablePattern.brightness);
      }
      va_end(args);
    }

    void fastForwardFields(int n) {
      while (n--) {
        renderer->renderField();
      }
    }

    void assertActiveStyles(int n, ...) {
      va_list args;
      va_start(args, n);
      uint8_t* activeStyles = renderer->getActiveStyles();
      for (int style = 0; style < n; style++) {
        uint8_t styleCount = va_arg(args, int);
        assertEqual(styleCount, activeStyles[style]);
      }
      va_end(args);
    }

    TestableHardware* hardware;
    FakeDriver* driver;
    BlinkStyler* blinkFast;
    BlinkStyler* blinkSlow;
    PulseStyler* pulseFast;
    PulseStyler* pulseSlow;
    StyleTable* styleTable;
    Renderer* renderer;
};

testF(RendererTest, frames_and_fields) {
  assertEqual(NUM_DIGITS, renderer->getNumDigits());
  assertEqual(FRAMES_PER_SECOND, renderer->getFramesPerSecond());
  assertEqual(NUM_DIGITS * NUM_SUB_FIELDS * FRAMES_PER_SECOND,
      renderer->getFieldsPerSecond());
}

testF(RendererTest, writePatternAt) {
  StyledPattern& styledPattern = styledPatterns[0];

  renderer->writePatternAt(0, 0x11, StyledPattern::kStyleNormal);
  assertEqual(0x11, styledPattern.pattern);
  assertEqual(0, styledPattern.style);

  renderer->writePatternAt(0, 0x22);
  assertEqual(0x22, styledPattern.pattern);
  assertEqual(0, styledPattern.style);

  renderer->writeStyleAt(0, kStyleBlinkSlow);
  assertEqual(0x22, styledPattern.pattern);
  assertEqual(kStyleBlinkSlow, styledPattern.style);
}

testF(RendererTest, writePatternAt_outOfBounds) {
  StyledPattern& styledPattern = styledPatterns[4];
  styledPattern.pattern = 1;
  styledPattern.style = StyledPattern::kStyleNormal;

  renderer->writePatternAt(4, 0x11, kStyleBlinkSlow);
  assertEqual(1, styledPattern.pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPattern.style);

  renderer->writePatternAt(4, 0x11);
  assertEqual(1, styledPattern.pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPattern.style);

  renderer->writeStyleAt(4, kStyleBlinkFast);
  assertEqual(1, styledPattern.pattern);
  assertEqual(StyledPattern::kStyleNormal, styledPattern.style);
}

testF(RendererTest, writeDecimalPointAt) {
  StyledPattern& styledPattern = styledPatterns[0];

  renderer->writePatternAt(0, 0x11, StyledPattern::kStyleNormal);
  assertEqual(0x11, styledPattern.pattern);
  assertEqual(0, styledPattern.style);

  renderer->writeDecimalPointAt(0);
  assertEqual(0x11 | 0x80, styledPattern.pattern);
  assertEqual(0, styledPattern.style);

  renderer->writeDecimalPointAt(0, false);
  assertEqual(0x11, styledPattern.pattern);
  assertEqual(0, styledPattern.style);
}

testF(RendererTest, clear) {
  StyledPattern& styledPattern = styledPatterns[0];

  renderer->writePatternAt(0, 0x11, kStyleBlinkSlow);
  assertEqual(0x11, styledPattern.pattern);
  assertEqual(kStyleBlinkSlow, styledPattern.style);

  renderer->clear();
  assertEqual(0, styledPattern.pattern);
  assertEqual(kStyleBlinkSlow, styledPattern.style);
}

testF(RendererTest, writeDecimalPointAt_outOfBounds) {
  StyledPattern& styledPattern = styledPatterns[4];
  styledPattern.pattern = 1;
  styledPattern.style = StyledPattern::kStyleNormal;

  renderer->writeDecimalPointAt(4);
  assertEqual(1, styledPattern.pattern);
}

testF(RendererTest, activeStyles) {
  assertActiveStyles(5, 4, 0, 0, 0, 0);

  renderer->writePatternAt(0, 0x11, StyledPattern::kStyleNormal);
  assertActiveStyles(5, 4, 0, 0, 0, 0);

  renderer->writePatternAt(0, 0x11, kStyleBlinkSlow);
  assertActiveStyles(5, 3, 1, 0, 0, 0);

  renderer->writePatternAt(1, 0x11, kStyleBlinkFast);
  assertActiveStyles(5, 2, 1, 1, 0, 0);

  renderer->writePatternAt(2, 0x11, kStylePulseSlow);
  assertActiveStyles(5, 1, 1, 1, 1, 0);

  renderer->writePatternAt(3, 0x11, kStylePulseFast);
  assertActiveStyles(5, 0, 1, 1, 1, 1);

  renderer->writeStyleAt(3, StyledPattern::kStyleNormal);
  assertActiveStyles(5, 1, 1, 1, 1, 0);

  // verify that write pattern to out of bounds digit does nothing
  renderer->writePatternAt(21, 0x55, kStyleBlinkSlow);
  assertActiveStyles(5, 1, 1, 1, 1, 0);

  // verify that write style to out of bounds digit does nothing
  renderer->writeStyleAt(20, kStyleBlinkSlow);
  assertActiveStyles(5, 1, 1, 1, 1, 0);

  // verify that writing an invalid style does nothing
  renderer->writeStyleAt(0, StyleTable::kNumStyles);
  assertActiveStyles(5, 1, 1, 1, 1, 0);

  // verify that write pattern with an unregistered style does nothing
  renderer->writePatternAt(0, 0x55, 5);
  assertActiveStyles(6, 1, 1, 1, 1, 0, 0);

  // verify that writing an unregsitered style does nothing
  renderer->writeStyleAt(0, 5);
  assertActiveStyles(6, 1, 1, 1, 1, 0, 0);
}

testF(RendererTest, isStylerSupported_driverWithoutBrightness) {
  driver->setNumSubFields(1);
  assertFalse(driver->isBrightnessSupported());
  renderer->configure();

  assertTrue(renderer->isStylerSupported(blinkSlow));
  assertTrue(renderer->isStylerSupported(blinkFast));
  assertFalse(renderer->isStylerSupported(pulseSlow));
  assertFalse(renderer->isStylerSupported(pulseFast));
}

testF(RendererTest, isStylerSupported_driverWithBrightness) {
  driver->setNumSubFields(3);
  assertTrue(driver->isBrightnessSupported());
  renderer->configure();

  assertTrue(renderer->isStylerSupported(blinkSlow));
  assertTrue(renderer->isStylerSupported(blinkFast));
  assertTrue(renderer->isStylerSupported(pulseSlow));
  assertTrue(renderer->isStylerSupported(pulseFast));
}

// Frame rates for various blinks and pulses at 60 fps, (4 * 3) fields/frame:
//  - blink slow: 48 frames/cycle = 4*3*48 = 576 fields/cycle
//  - blink fast: 24 frames/cycle = 4*3*24 = 288 fields/cycle
//  - pulse slow: 180 frames/cycle = 4*3*180 = 2160 fields/cycle
//  - pulse fast: 60 frames/cycle - 4*3*60 = 720 fields/cycle
testF(RendererTest, renderField) {
  assertActiveStyles(5, 4, 0, 0, 0, 0);

  renderer->writePatternAt(0, 0x11, kStyleBlinkSlow);
  renderer->writePatternAt(1, 0x22, kStyleBlinkFast);
  renderer->writePatternAt(2, 0x33, kStylePulseSlow);
  renderer->writePatternAt(3, 0x44, kStylePulseFast);

  assertActiveStyles(5, 0, 1, 1, 1, 1);

  assertEqual((uint16_t) (FRAMES_PER_SECOND * 4 * 3),
      renderer->getFieldsPerSecond());

  renderer->renderField();
  assertDimmablePatternsEqual(4, 0x11, 255, 0x22, 255, 0x33, 0, 0x44, 0);

  fastForwardFields(156); // +13 frames = #13
  assertDimmablePatternsEqual(4, 0x11, 255, 0x22, 0, 0x33, 35, 0x44, 109);

  fastForwardFields(156); // +13 frames = #26
  assertDimmablePatternsEqual(4, 0x11, 0, 0x22, 255, 0x33, 72, 0x44, 220);

  fastForwardFields(708); // +59 frames = #85
  assertDimmablePatternsEqual(4, 0x11, 0, 0x22, 0, 0x33, 240, 0x44, 212);
}

// If the driver does not support subfields, then pulsing is disabled.
//  - blink slow: 48 frames/cycle = 4*48 = 192 fields/cycle
//  - blink fast: 24 frames/cycle = 4*24 = 96 fields/cycle
//  - pulse slow: 180 frames/cycle = 4*180 = 720 fields/cycle
//  - pulse fast: 60 frames/cycle - 4*60 = 240 fields/cycle
testF(RendererTest, renderField_noSubFieldDriver) {
  driver->setNumSubFields(1);
  assertFalse(driver->isBrightnessSupported());
  renderer->configure();

  renderer->writePatternAt(0, 0x11, kStyleBlinkSlow);
  renderer->writePatternAt(1, 0x22, kStyleBlinkFast);
  renderer->writePatternAt(2, 0x33, kStylePulseSlow);
  renderer->writePatternAt(3, 0x44, kStylePulseFast);

  assertEqual((uint16_t) (FRAMES_PER_SECOND * 4 * 1),
      renderer->getFieldsPerSecond());

  renderer->renderField(); // #0
  assertDimmablePatternsEqual(4, 0x11, 255, 0x22, 255, 0x33, 255, 0x44, 255);

  fastForwardFields(52); // +13 frames = #13
  assertDimmablePatternsEqual(4, 0x11, 255, 0x22, 0, 0x33, 255, 0x44, 255);

  fastForwardFields(52); // +13 frames = #26
  assertDimmablePatternsEqual(4, 0x11, 0, 0x22, 255, 0x33, 255, 0x44, 255);

  fastForwardFields(236); // +59 frames = #85
  assertDimmablePatternsEqual(4, 0x11, 0, 0x22, 0, 0x33, 255, 0x44, 255);
}

// If the driver supports subfields, then pulsing is enabled.
testF(RendererTest, renderField_dimmedBrightness) {
  driver->setNumSubFields(3);
  assertTrue(driver->isBrightnessSupported());
  renderer->configure();

  renderer->writeBrightness(127);
  renderer->writePatternAt(0, 0x11, kStyleBlinkSlow);
  renderer->writePatternAt(1, 0x22, kStyleBlinkFast);
  renderer->writePatternAt(2, 0x33, kStylePulseSlow);
  renderer->writePatternAt(3, 0x44, kStylePulseFast);

  assertEqual((uint16_t) (FRAMES_PER_SECOND * 4 * 3),
      renderer->getFieldsPerSecond());

  // Verify that the digit brightness is about 1/2 the value of
  // the testF(RendererTest, renderField) test case.

  renderer->renderField();
  assertDimmablePatternsEqual(4, 0x11, 127, 0x22, 127, 0x33, 0, 0x44, 0);

  fastForwardFields(156); // +13 frames = #13
  assertDimmablePatternsEqual(4, 0x11, 127, 0x22, 0, 0x33, 17, 0x44, 54);

  fastForwardFields(156); // +13 frames = #26
  assertDimmablePatternsEqual(4, 0x11, 0, 0x22, 127, 0x33, 36, 0x44, 109);

  fastForwardFields(708); // +59 frames = #85
  assertDimmablePatternsEqual(4, 0x11, 0, 0x22, 0, 0x33, 119, 0x44, 105);
}


