#line 2 "ScanningModuleTest.ino"

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
// Tests for ScanningModule w/ a TestableLedMatrix
// ----------------------------------------------------------------------

test(ScanningModuleTest, renderFieldNow) {
  scanningModule.begin();
  ledMatrix.mEventLog.clear();

  scanningModule.setPatternAt(0, 0x00);
  scanningModule.setPatternAt(1, 0x11);
  scanningModule.setPatternAt(2, 0x22);
  scanningModule.setPatternAt(3, 0x33);

  // display field 0
  ledMatrix.mEventLog.clear();
  scanningModule.renderFieldNow();
  assertEqual(1, ledMatrix.mEventLog.getNumRecords());
  // The following cast to (int) is required on 8-bit AVR processors, but not on
  // the 32-bit processors. I am not sure why because my understanding of C++
  // says that the promotion should be done automatically. Maybe it's something
  // to do with the size of EventType, which is a uint8_t, which does not get
  // automatically promoted on 8-bit compilers.
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

  scanningModule.end();
}

// A subclass of ScanningModule must render the digits and segments
// continuously. So renderFieldNow() will never reset the digit dirty bits.
test(ScanningModuleTest, isAnyDigitDirty) {
  scanningModule.begin();
  assertTrue(scanningModule.isAnyDigitDirty());

  scanningModule.renderFieldNow();
  assertTrue(scanningModule.isAnyDigitDirty());

  scanningModule.end();
}


// A subclass of ScanningModule keeps track of brightness on a per-digit basis.
// The global brightness is transferred into the per-digit brightness array just
// upon rendering. The global isBrightnessDirty() is used to keep track of
// whether or not the global-to-per-digit must be done. So renderFieldNow() will
// clear the isBrightnessDirty() bit.
test(ScanningModuleTest, isBrightnessDirty) {
  scanningModule.begin();
  assertTrue(scanningModule.isBrightnessDirty());

  scanningModule.renderFieldNow();
  assertFalse(scanningModule.isBrightnessDirty());

  scanningModule.end();
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
