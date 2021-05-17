#line 2 "Tm1637ModuleTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/TestableWireInterface.h>
#include <ace_segment/testing/EventLog.h>

using aunit::TestRunner;
using ace_segment::testing::TestableWireInterface;
using ace_segment::testing::EventType;
using ace_segment::internal::initialDirtyBits;
using ace_segment::Tm1637Module;

//----------------------------------------------------------------------------

test(Tm1637ModuleTest, initialDirtyBits) {
  assertEqual(0b0, initialDirtyBits(0));
  assertEqual(0b1, initialDirtyBits(1));
  assertEqual(0b11, initialDirtyBits(2));
  assertEqual(0b111, initialDirtyBits(3));
  assertEqual(0b1111111, initialDirtyBits(7));
  assertEqual(0b11111111, initialDirtyBits(8));
}

//----------------------------------------------------------------------------

const uint8_t NUM_DIGITS = 4;
TestableWireInterface wireInterface;
Tm1637Module<TestableWireInterface, NUM_DIGITS> tm1637Module(wireInterface);

test(Tm1637ModuleTest, flushIncremental) {
  wireInterface.begin();
  tm1637Module.begin();
  wireInterface.mEventLog.clear();

  // Verify dirty bits initial start dirty, 4 digit bits + 1 brightness bit
  assertEqual(0x1F, tm1637Module.mIsDirty);
  tm1637Module.mIsDirty = 0x0;

  // Verify mFlushStage
  assertEqual(0, tm1637Module.mFlushStage);

  // Set digit 0.
  tm1637Module.setPatternAt(1, 0x11);
  assertEqual(0x1 << 1, tm1637Module.mIsDirty);

  // Set brightness.
  tm1637Module.setBrightness(2);
  assertEqual((0x1 << NUM_DIGITS) | (0x1 << 1), tm1637Module.mIsDirty);

  // Iteration 0 sends digit 0, which did not change, so send nothing.
  assertEqual(0, tm1637Module.mFlushStage);
  wireInterface.mEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(0, wireInterface.mEventLog.getNumRecords());
  assertFalse(tm1637Module.isDirtyBit(0));
  assertEqual(1, tm1637Module.mFlushStage);

  // Iteration 1 sends digit 1, which changed, so send digit 1;
  assertEqual(1, tm1637Module.mFlushStage);
  wireInterface.mEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(7, wireInterface.mEventLog.getNumRecords());
  assertTrue(wireInterface.mEventLog.assertEvents(
    7,
    (int) EventType::kWireStartCondition,
    (int) EventType::kWireSendByte,
        Tm1637Module<TestableWireInterface, NUM_DIGITS>::kDataCmdFixedAddress,
    (int) EventType::kWireStopCondition,
    (int) EventType::kWireStartCondition,
    (int) EventType::kWireSendByte,
        Tm1637Module<TestableWireInterface, NUM_DIGITS>::kAddressCmd | 0x1,
    (int) EventType::kWireSendByte, 0x11,
    (int) EventType::kWireStopCondition
  ));
  assertFalse(tm1637Module.isDirtyBit(1));
  assertEqual(2, tm1637Module.mFlushStage);

  // Iteration 2 sends digit 3, which sends nothing.
  assertEqual(2, tm1637Module.mFlushStage);
  wireInterface.mEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(0, wireInterface.mEventLog.getNumRecords());
  assertFalse(tm1637Module.isDirtyBit(2));
  assertEqual(3, tm1637Module.mFlushStage);

  // Iteration 3 sends digit 3, which sends nothing.
  assertEqual(3, tm1637Module.mFlushStage);
  wireInterface.mEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(0, wireInterface.mEventLog.getNumRecords());
  assertFalse(tm1637Module.isDirtyBit(3));
  assertEqual(4, tm1637Module.mFlushStage);

  // Iteration 4 sends brightness, which changed, so sends package.
  assertEqual(4, tm1637Module.mFlushStage);
  wireInterface.mEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(3, wireInterface.mEventLog.getNumRecords());
  assertTrue(wireInterface.mEventLog.assertEvents(
    3,
    (int) EventType::kWireStartCondition,
    (int) EventType::kWireSendByte,
        Tm1637Module<TestableWireInterface, NUM_DIGITS>::kBrightnessCmd
          | Tm1637Module<TestableWireInterface, NUM_DIGITS>::kBrightnessLevelOn
          | 2,
    (int) EventType::kWireStopCondition
  ));
  assertFalse(tm1637Module.isDirtyBit(4));
  assertEqual(0x0, tm1637Module.mIsDirty);
  assertEqual(0, tm1637Module.mFlushStage);
}

test(Tm1637ModuleTest, flush) {
  wireInterface.begin();
  tm1637Module.begin();
  wireInterface.mEventLog.clear();

  // Verify dirty bits initial start dirty, 4 digit bits + 1 brightness bit
  assertEqual(0x1F, tm1637Module.mIsDirty);
  tm1637Module.mIsDirty = 0x0;

  // Calling flush() when nothing is dirty returns immediately.
  wireInterface.mEventLog.clear();
  tm1637Module.flush();
  assertEqual(0, wireInterface.mEventLog.getNumRecords());

  // Set digit 0.
  tm1637Module.setPatternAt(1, 0x11);
  assertEqual(0x1 << 1, tm1637Module.mIsDirty);

  // Set brightness.
  tm1637Module.setBrightness(2);
  assertEqual((0x1 << NUM_DIGITS) | (0x1 << 1), tm1637Module.mIsDirty);

  // Calling flush() sends everything.
  wireInterface.mEventLog.clear();
  tm1637Module.flush();
  assertEqual(13, wireInterface.mEventLog.getNumRecords());
  assertTrue(wireInterface.mEventLog.assertEvents(
    13,
    // brightness
    (int) EventType::kWireStartCondition,
    (int) EventType::kWireSendByte,
        Tm1637Module<TestableWireInterface, NUM_DIGITS>::kBrightnessCmd
          | Tm1637Module<TestableWireInterface, NUM_DIGITS>::kBrightnessLevelOn
          | 2,
    (int) EventType::kWireStopCondition,

    // auto increment mode
    (int) EventType::kWireStartCondition,
    (int) EventType::kWireSendByte,
        Tm1637Module<TestableWireInterface, NUM_DIGITS>::kDataCmdAutoAddress,
    (int) EventType::kWireStopCondition,

    // send 4 digits
    (int) EventType::kWireStartCondition,
    (int) EventType::kWireSendByte,
        Tm1637Module<TestableWireInterface, NUM_DIGITS>::kAddressCmd,
    (int) EventType::kWireSendByte, 0x00,
    (int) EventType::kWireSendByte, 0x11,
    (int) EventType::kWireSendByte, 0x00,
    (int) EventType::kWireSendByte, 0x00,
    (int) EventType::kWireStopCondition
  ));
  assertEqual(0x0, tm1637Module.mIsDirty);
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
