#line 2 "ScanningModuleTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/EventLog.h>
#include <ace_segment/testing/TestableClockInterface.h>
#include <ace_segment/testing/TestableLedMatrix.h>

using aunit::TestRunner;
using aunit::TestOnce;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t NUM_DIGITS = 4;
const int8_t NUM_SEGMENTS = 8;
const uint16_t FRAMES_PER_SECOND = 60;
const int8_t NUM_SUB_FIELDS = 1;

TestableLedMatrix ledMatrix;

ScanningModule<
    TestableLedMatrix,
    NUM_DIGITS,
    NUM_SUB_FIELDS,
    TestableClockInterface
> scanningModule(ledMatrix, FRAMES_PER_SECOND);

// ----------------------------------------------------------------------
// Tests for SplitDigitDriver w/ LedMatrixDirect
// ----------------------------------------------------------------------

class ScanningModuleTest: public TestOnce {
  protected:
    void setup() override {
      scanningModule.begin();
      TestableClockInterface::sEventLog.clear();
      ledMatrix.mEventLog.clear();
    }
};

testF(ScanningModuleTest, renderFieldNow) {
  scanningModule.setPatternAt(0, 0x00);
  scanningModule.setPatternAt(1, 0x11);
  scanningModule.setPatternAt(2, 0x22);
  scanningModule.setPatternAt(3, 0x33);

  // display field 0
  ledMatrix.mEventLog.clear();
  scanningModule.renderFieldNow();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  // Cast to (int) required on 8-bit AVR processors (not sure why), something to
  // do with the size of EventType, which is a uint8_t, which does not get
  // automatically promoted?
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, (int) EventType::kLedMatrixDraw, 0, 0x00));

  // display field 1
  ledMatrix.mEventLog.clear();
  scanningModule.renderFieldNow();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, (int) EventType::kLedMatrixDraw, 1, 0x11));

  // display field 2
  ledMatrix.mEventLog.clear();
  scanningModule.renderFieldNow();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, (int) EventType::kLedMatrixDraw, 2, 0x22));

  // display field 3
  ledMatrix.mEventLog.clear();
  scanningModule.renderFieldNow();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, (int) EventType::kLedMatrixDraw, 3, 0x33));

  // cycle back to field 0
  ledMatrix.mEventLog.clear();
  scanningModule.renderFieldNow();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  assertTrue(ledMatrix.mEventLog.assertEvents(
      1, (int) EventType::kLedMatrixDraw, 0, 0x00));
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
