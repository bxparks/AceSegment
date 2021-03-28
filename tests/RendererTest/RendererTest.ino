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
#include <Arduino.h>
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
const uint8_t CUSTOM_BRIGHTNESS = 64;
const uint8_t DIFFERENT_BRIGHTNESS = 32;

// create NUM_DIGITS+1 elements for doing array bound checking
DimmablePattern dimmablePatterns[NUM_DIGITS + 1];

// ----------------------------------------------------------------------
// Tests for Renderer.
// ----------------------------------------------------------------------

class RendererTest: public TestOnce {
  protected:
    static const uint16_t kStatsResetInterval = 1200;

    void setup() override {
      TestOnce::setup();

      memset(dimmablePatterns, 0, (NUM_DIGITS+1) * sizeof(DimmablePattern));

      hardware = new TestableHardware();

      driver = new FakeDriver(dimmablePatterns, NUM_DIGITS);
      driver->setNumSubFields(NUM_SUB_FIELDS);
      driver->configure();

      renderer = new Renderer(hardware, driver, dimmablePatterns,
          NUM_DIGITS, FRAMES_PER_SECOND, kStatsResetInterval);
      //renderer->writeBrightness(255);
      renderer->configure();

      hardware->clear();
    }

    void teardown() override {
      delete renderer;
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

    TestableHardware* hardware;
    FakeDriver* driver;
    Renderer* renderer;
};

testF(RendererTest, frames_and_fields) {
  assertEqual(NUM_DIGITS, renderer->getNumDigits());
  assertEqual(FRAMES_PER_SECOND, renderer->getFramesPerSecond());
  assertEqual(NUM_DIGITS * NUM_SUB_FIELDS * FRAMES_PER_SECOND,
      renderer->getFieldsPerSecond());
}

testF(RendererTest, writePatternAt) {
  DimmablePattern& dimmablePattern = dimmablePatterns[0];

  renderer->writePatternAt(0, 0x11, CUSTOM_BRIGHTNESS);
  assertEqual(0x11, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);

  renderer->writePatternAt(0, 0x22);
  assertEqual(0x22, dimmablePattern.pattern);
  assertEqual(Renderer::kDefaultBrightness, dimmablePattern.brightness);

  renderer->writeBrightnessAt(0, CUSTOM_BRIGHTNESS);
  assertEqual(0x22, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);
}

testF(RendererTest, writePatternAt_outOfBounds) {
  DimmablePattern& dimmablePattern = dimmablePatterns[4];
  dimmablePattern.pattern = 1;
  dimmablePattern.brightness = CUSTOM_BRIGHTNESS;

  renderer->writePatternAt(4, 0x11, CUSTOM_BRIGHTNESS);
  assertEqual(1, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);

  renderer->writePatternAt(4, 0x11);
  assertEqual(1, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);

  renderer->writeBrightnessAt(4, DIFFERENT_BRIGHTNESS);
  assertEqual(1, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);
}

testF(RendererTest, writeDecimalPointAt) {
  DimmablePattern& dimmablePattern = dimmablePatterns[0];

  renderer->writePatternAt(0, 0x11, CUSTOM_BRIGHTNESS);
  assertEqual(0x11, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);

  renderer->writeDecimalPointAt(0);
  assertEqual(0x11 | 0x80, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);

  renderer->writeDecimalPointAt(0, false);
  assertEqual(0x11, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);
}

testF(RendererTest, clear) {
  DimmablePattern& dimmablePattern = dimmablePatterns[0];

  renderer->writePatternAt(0, 0x11, CUSTOM_BRIGHTNESS);
  assertEqual(0x11, dimmablePattern.pattern);
  assertEqual(CUSTOM_BRIGHTNESS, dimmablePattern.brightness);

  renderer->clear();
  assertEqual(0, dimmablePattern.pattern);
  assertEqual(0, dimmablePattern.brightness);
}

testF(RendererTest, writeDecimalPointAt_outOfBounds) {
  DimmablePattern& dimmablePattern = dimmablePatterns[4];
  dimmablePattern.pattern = 1;
  dimmablePattern.brightness = CUSTOM_BRIGHTNESS;

  renderer->writeDecimalPointAt(4);
  assertEqual(1, dimmablePattern.pattern);
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
