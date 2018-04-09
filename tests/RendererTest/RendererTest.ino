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
DimmingDigit dimmingDigits[NUM_DIGITS + 1];
StyledDigit styledDigits[NUM_DIGITS + 1];

TestableHardware *hardware;
FakeDriver *driver;
Renderer *renderer;

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): start"));

  hardware = new TestableHardware();
  driver = new FakeDriver(dimmingDigits, NUM_DIGITS);
  renderer = new Renderer(hardware, driver, styledDigits, NUM_DIGITS);

  Serial.println(F("setup(): end"));
}

void loop() {
  TestRunner::run();
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
      driver->setNumSubFields(NUM_SUB_FIELDS);
      (*renderer)
          .setFramesPerSecond(FRAMES_PER_SECOND)
          .configure();

      renderer->writeBrightness(255);
      hardware->clear();
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
        renderer->renderField();
      }
    }
};

testF(RendererTest, frames_and_fields) {
  assertEqual(NUM_DIGITS, renderer->getNumDigits());
  assertEqual(FRAMES_PER_SECOND, renderer->getFramesPerSecond());
  assertEqual(NUM_DIGITS * NUM_SUB_FIELDS * FRAMES_PER_SECOND,
      renderer->getFieldsPerSecond());
}

testF(RendererTest, writePatternAt) {
  StyledDigit& styledDigit = styledDigits[0];

  renderer->writePatternAt(0, 0x11, StyledDigit::kStyleNormal);
  assertEqual(0x11, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer->writePatternAt(0, 0x22);
  assertEqual(0x22, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer->writeStyleAt(0, StyledDigit::kStyleBlinkFast);
  assertEqual(0x22, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleBlinkFast, styledDigit.style);
}

testF(RendererTest, writePatternAt_outOfBounds) {
  StyledDigit& styledDigit = styledDigits[4];
  styledDigit.pattern = 1;
  styledDigit.style = StyledDigit::kStyleNormal;

  renderer->writePatternAt(4, 0x11, StyledDigit::kStyleBlinkSlow);
  assertEqual(1, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigit.style);

  renderer->writePatternAt(4, 0x11);
  assertEqual(1, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigit.style);

  renderer->writeStyleAt(4, StyledDigit::kStyleBlinkFast);
  assertEqual(1, styledDigit.pattern);
  assertEqual(StyledDigit::kStyleNormal, styledDigit.style);
}

testF(RendererTest, writeDecimalPointAt) {
  StyledDigit& styledDigit = styledDigits[0];

  renderer->writePatternAt(0, 0x11, StyledDigit::kStyleNormal);
  assertEqual(0x11, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer->writeDecimalPointAt(0);
  assertEqual(0x11 | 0x80, styledDigit.pattern);
  assertEqual(0, styledDigit.style);

  renderer->writeDecimalPointAt(0, false);
  assertEqual(0x11, styledDigit.pattern);
  assertEqual(0, styledDigit.style);
}

testF(RendererTest, writeDecimalPointAt_outOfBounds) {
  StyledDigit& styledDigit = styledDigits[4];
  styledDigit.pattern = 1;
  styledDigit.style = StyledDigit::kStyleNormal;

  renderer->writeDecimalPointAt(4);
  assertEqual(1, styledDigit.pattern);
}

// Frame rates for various blinks and pulses at 60 fps, (4 * 3) fields/frame:
//  - blink slow: 48 frames/cycle = 4*3*48 = 576 fields/cycle
//  - blink fast: 24 frames/cycle = 4*3*24 = 288 fields/cycle
//  - pulse slow: 180 frames/cycle = 4*3*180 = 2160 fields/cycle
//  - pulse fast: 60 frames/cycle - 4*3*60 = 720 fields/cycle
testF(RendererTest, displayCurrentField) {
  renderer->writePatternAt(0, 0x11, StyledDigit::kStyleBlinkSlow);
  renderer->writePatternAt(1, 0x22, StyledDigit::kStyleBlinkFast);
  renderer->writePatternAt(2, 0x33, StyledDigit::kStylePulseSlow);
  renderer->writePatternAt(3, 0x44, StyledDigit::kStylePulseFast);

  assertEqual((uint16_t) (60 * 4 * 3), renderer->getFieldsPerSecond());

  renderer->renderField();
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
  driver->setNumSubFields(1);
  assertEqual(false, driver->isBrightnessSupported());
  renderer->configure();

  renderer->writePatternAt(0, 0x11, StyledDigit::kStyleBlinkSlow);
  renderer->writePatternAt(1, 0x22, StyledDigit::kStyleBlinkFast);
  renderer->writePatternAt(2, 0x33, StyledDigit::kStylePulseSlow);
  renderer->writePatternAt(3, 0x44, StyledDigit::kStylePulseFast);

  assertEqual((uint16_t) (60 * 4 * 1), renderer->getFieldsPerSecond());

  renderer->renderField(); // #0
  assertDimmingDigitsEqual(4, 0x11, 255, 0x22, 255, 0x33, 255, 0x44, 255);

  fastForwardFields(52); // +13 frames = #13
  assertDimmingDigitsEqual(4, 0x11, 255, 0x22, 0, 0x33, 255, 0x44, 255);

  fastForwardFields(52); // +13 frames = #26
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 255, 0x33, 255, 0x44, 255);

  fastForwardFields(236); // +59 frames = #85
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 0, 0x33, 255, 0x44, 255);
}

testF(RendererTest, displayCurrentField_dimmedBrightness) {
  driver->setNumSubFields(3);
  assertEqual(true, driver->isBrightnessSupported());
  renderer->configure();

  renderer->writeBrightness(127);
  renderer->writePatternAt(0, 0x11, StyledDigit::kStyleBlinkSlow);
  renderer->writePatternAt(1, 0x22, StyledDigit::kStyleBlinkFast);
  renderer->writePatternAt(2, 0x33, StyledDigit::kStylePulseSlow);
  renderer->writePatternAt(3, 0x44, StyledDigit::kStylePulseFast);

  assertEqual((uint16_t) (60 * 4 * 3), renderer->getFieldsPerSecond());

  // Verify that the digit brightness is about 1/2 the value of
  // the testF(RendererTest, displayCurrentField) test case.

  renderer->renderField();
  assertDimmingDigitsEqual(4, 0x11, 127, 0x22, 127, 0x33, 0, 0x44, 0);

  fastForwardFields(156); // +13 frames = #13
  assertDimmingDigitsEqual(4, 0x11, 127, 0x22, 0, 0x33, 17, 0x44, 54);

  fastForwardFields(156); // +13 frames = #26
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 127, 0x33, 36, 0x44, 109);

  fastForwardFields(708); // +59 frames = #85
  assertDimmingDigitsEqual(4, 0x11, 0, 0x22, 0, 0x33, 119, 0x44, 105);
}


