#line 2 "SegmentDisplayTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/EventLog.h>
#include <ace_segment/testing/TestableHardware.h>
#include <ace_segment/testing/TestableLedMatrix.h>

using aunit::TestRunner;
using aunit::TestOnce;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t NUM_DIGITS = 4;
const int8_t NUM_SEGMENTS = 8;
const uint16_t FRAMES_PER_SECOND = 60;
const int8_t NUM_SUB_FIELDS = 1;

TestableHardware hardware;
TestableLedMatrix ledMatrix;

SegmentDisplay<TestableHardware, TestableLedMatrix, NUM_DIGITS, NUM_SUB_FIELDS>
  segmentDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

// ----------------------------------------------------------------------
// Tests for SplitDigitDriver w/ LedMatrixDirect
// ----------------------------------------------------------------------

class SegmentDisplayTest: public TestOnce {
  protected:
    void setup() override {
      segmentDisplay.begin();
      hardware.mEventLog.clear();
      ledMatrix.mEventLog.clear();
    }
};

testF(SegmentDisplayTest, displayCurrentField) {
  segmentDisplay.writePatternAt(0, 0x00);
  segmentDisplay.writePatternAt(1, 0x11);
  segmentDisplay.writePatternAt(2, 0x22);
  segmentDisplay.writePatternAt(3, 0x33);

  // display field 0
  ledMatrix.mEventLog.clear();
  segmentDisplay.displayCurrentField();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, EventType::kLedMatrixDraw, 0, 0x00));

  // display field 1
  ledMatrix.mEventLog.clear();
  segmentDisplay.displayCurrentField();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, EventType::kLedMatrixDraw, 1, 0x11));

  // display field 2
  ledMatrix.mEventLog.clear();
  segmentDisplay.displayCurrentField();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, EventType::kLedMatrixDraw, 2, 0x22));

  // display field 3
  ledMatrix.mEventLog.clear();
  segmentDisplay.displayCurrentField();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, EventType::kLedMatrixDraw, 3, 0x33));

  // cycle back to field 0
  ledMatrix.mEventLog.clear();
  segmentDisplay.displayCurrentField();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, EventType::kLedMatrixDraw, 0, 0x00));
}

//----------------------------------------------------------------------------

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
