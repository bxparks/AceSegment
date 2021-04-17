#line 2 "LedMatrixTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/TestableClockInterface.h>
#include <ace_segment/testing/TestableGpioInterface.h>
#include <ace_segment/testing/TestableSpiInterface.h>

using aunit::TestRunner;
using aunit::TestOnce;
using namespace ace_segment;
using namespace ace_segment::testing;

const int8_t NUM_DIGITS = 4;
const int8_t NUM_SEGMENTS = 8;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {0, 1, 2, 3};
const uint8_t SEGMENT_PINS[8] = {4, 5, 6, 7, 8, 9, 10, 11};

// Common Cathode, with transistors on Group pins
LedMatrixDirect<TestableGpioInterface> ledMatrixDirect(
    LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
    LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS,
    NUM_SEGMENTS,
    SEGMENT_PINS);

// Common Cathode, with transistors on Group pins
TestableSpiInterface spiInterface;
LedMatrixSingleShiftRegister<TestableSpiInterface, TestableGpioInterface>
  ledMatrixSingleShiftRegister(
    spiInterface,
    LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
    LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS);

// Common Cathode, with transistors on Group pins
LedMatrixDualShiftRegister<TestableSpiInterface> ledMatrixDualShiftRegister(
    spiInterface,
    LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
    LedMatrixBase::kActiveHighPattern /*elementOnPattern*/);

// ----------------------------------------------------------------------
// Tests for LedMatrixSplitDirect.
// ----------------------------------------------------------------------

class LedMatrixDirectTest : public TestOnce {
  protected:
    void setup() override {
      ledMatrixDirect.begin();
      TestableGpioInterface::sEventLog.clear();
    }
};

testF(LedMatrixDirectTest, begin) {
  ledMatrixDirect.begin();
  assertEqual(24, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(24,
      (int) EventType::kPinMode, 0, OUTPUT,
      (int) EventType::kDigitalWrite, 0, LOW,
      (int) EventType::kPinMode, 1, OUTPUT,
      (int) EventType::kDigitalWrite, 1, LOW,
      (int) EventType::kPinMode, 2, OUTPUT,
      (int) EventType::kDigitalWrite, 2, LOW,
      (int) EventType::kPinMode, 3, OUTPUT,
      (int) EventType::kDigitalWrite, 3, LOW,

      (int) EventType::kPinMode, 4, OUTPUT,
      (int) EventType::kDigitalWrite, 4, LOW,
      (int) EventType::kPinMode, 5, OUTPUT,
      (int) EventType::kDigitalWrite, 5, LOW,
      (int) EventType::kPinMode, 6, OUTPUT,
      (int) EventType::kDigitalWrite, 6, LOW,
      (int) EventType::kPinMode, 7, OUTPUT,
      (int) EventType::kDigitalWrite, 7, LOW,
      (int) EventType::kPinMode, 8, OUTPUT,
      (int) EventType::kDigitalWrite, 8, LOW,
      (int) EventType::kPinMode, 9, OUTPUT,
      (int) EventType::kDigitalWrite, 9, LOW,
      (int) EventType::kPinMode, 10, OUTPUT,
      (int) EventType::kDigitalWrite, 10, LOW,
      (int) EventType::kPinMode, 11, OUTPUT,
      (int) EventType::kDigitalWrite, 11, LOW
  ));
}

testF(LedMatrixDirectTest, end) {
  ledMatrixDirect.end();
  assertEqual(12, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(12,
      (int) EventType::kPinMode, 0, INPUT,
      (int) EventType::kPinMode, 1, INPUT,
      (int) EventType::kPinMode, 2, INPUT,
      (int) EventType::kPinMode, 3, INPUT,
      (int) EventType::kPinMode, 4, INPUT,
      (int) EventType::kPinMode, 5, INPUT,
      (int) EventType::kPinMode, 6, INPUT,
      (int) EventType::kPinMode, 7, INPUT,
      (int) EventType::kPinMode, 8, INPUT,
      (int) EventType::kPinMode, 9, INPUT,
      (int) EventType::kPinMode, 10, INPUT,
      (int) EventType::kPinMode, 11, INPUT
  ));
}

testF(LedMatrixDirectTest, enableGroup) {
  ledMatrixDirect.enableGroup(1);
  assertEqual(1, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(1,
      (int) EventType::kDigitalWrite, 1, HIGH));
}

testF(LedMatrixDirectTest, disableGroup) {
  ledMatrixDirect.disableGroup(1);
  assertEqual(1, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(1,
      (int) EventType::kDigitalWrite, 1, LOW));
}

testF(LedMatrixDirectTest, drawElements) {
  ledMatrixDirect.drawElements(0x55);
  assertEqual(8, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(8,
      (int) EventType::kDigitalWrite, 4, HIGH,
      (int) EventType::kDigitalWrite, 5, LOW,
      (int) EventType::kDigitalWrite, 6, HIGH,
      (int) EventType::kDigitalWrite, 7, LOW,
      (int) EventType::kDigitalWrite, 8, HIGH,
      (int) EventType::kDigitalWrite, 9, LOW,
      (int) EventType::kDigitalWrite, 10, HIGH,
      (int) EventType::kDigitalWrite, 11, LOW
  ));
}

// ----------------------------------------------------------------------
// Tests for LedMatrixSingleShiftRegister.
// ----------------------------------------------------------------------

class LedMatrixSingleShiftRegisterTest : public TestOnce {
  protected:
    void setup() override {
      ledMatrixSingleShiftRegister.begin();
      TestableGpioInterface::sEventLog.clear();
    }
};

testF(LedMatrixSingleShiftRegisterTest, begin) {
  ledMatrixSingleShiftRegister.begin();
  assertEqual(8, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(8,
      (int) EventType::kPinMode, 0, OUTPUT,
      (int) EventType::kDigitalWrite, 0, LOW,
      (int) EventType::kPinMode, 1, OUTPUT,
      (int) EventType::kDigitalWrite, 1, LOW,
      (int) EventType::kPinMode, 2, OUTPUT,
      (int) EventType::kDigitalWrite, 2, LOW,
      (int) EventType::kPinMode, 3, OUTPUT,
      (int) EventType::kDigitalWrite, 3, LOW));
}

testF(LedMatrixSingleShiftRegisterTest, end) {
  ledMatrixSingleShiftRegister.end();
  assertEqual(4, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(4,
      (int) EventType::kPinMode, 0, INPUT,
      (int) EventType::kPinMode, 1, INPUT,
      (int) EventType::kPinMode, 2, INPUT,
      (int) EventType::kPinMode, 3, INPUT));

}

testF(LedMatrixSingleShiftRegisterTest, enableGroup) {
  ledMatrixSingleShiftRegister.enableGroup(1);
  assertEqual(1, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(1,
      (int) EventType::kDigitalWrite, 1, HIGH));
}

testF(LedMatrixSingleShiftRegisterTest, disableGroup) {
  ledMatrixSingleShiftRegister.disableGroup(1);
  assertEqual(1, TestableGpioInterface::sEventLog.getNumRecords());
  assertTrue(TestableGpioInterface::sEventLog.assertEvents(1,
      (int) EventType::kDigitalWrite, 1, LOW));
}

testF(LedMatrixSingleShiftRegisterTest, drawElements) {
  ledMatrixSingleShiftRegister.drawElements(0x55);
  assertEqual(1, spiInterface.mEventLog.getNumRecords());
  assertTrue(spiInterface.mEventLog.assertEvents(1,
      (int) EventType::kSpiTransfer, 0x55
  ));
}

// ----------------------------------------------------------------------
// Tests for LedMatrixSplitSpi.
// ----------------------------------------------------------------------

class LedMatrixDualShiftRegisterTest : public TestOnce {
  protected:
    void setup() override {
      ledMatrixDualShiftRegister.begin();
      spiInterface.mEventLog.clear();
    }
};

testF(LedMatrixDualShiftRegisterTest, begin) {
  ledMatrixDualShiftRegister.begin();
  assertEqual(0, spiInterface.mEventLog.getNumRecords());
}

testF(LedMatrixDualShiftRegisterTest, end) {
  ledMatrixDualShiftRegister.end();
  assertEqual(0, spiInterface.mEventLog.getNumRecords());
}

testF(LedMatrixDualShiftRegisterTest, enableGroup) {
  ledMatrixDualShiftRegister.mPrevElementPattern = 0x42;
  ledMatrixDualShiftRegister.enableGroup(1);

  assertEqual(1, spiInterface.mEventLog.getNumRecords());
  uint16_t expectedOutput = ((0x1 << 1) << 8) | 0x42;
  assertTrue(spiInterface.mEventLog.assertEvents(1,
      (int) EventType::kSpiTransfer16, expectedOutput));
}

testF(LedMatrixDualShiftRegisterTest, disableGroup) {
  ledMatrixDualShiftRegister.disableGroup(2);

  assertEqual(1, spiInterface.mEventLog.getNumRecords());
  uint16_t expectedOutput = 0x0000;
  assertTrue(spiInterface.mEventLog.assertEvents(1,
      (int) EventType::kSpiTransfer16, expectedOutput));
}

testF(LedMatrixDualShiftRegisterTest, draw) {
  ledMatrixDualShiftRegister.draw(3, 0x55);

  uint16_t expectedOutput = ((0x1 << 3) << 8) | 0x55;
  assertEqual(1, spiInterface.mEventLog.getNumRecords());
  assertTrue(spiInterface.mEventLog.assertEvents(1,
    (int) EventType::kSpiTransfer16, expectedOutput
  ));
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
