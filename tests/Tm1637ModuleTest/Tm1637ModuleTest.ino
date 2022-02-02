#line 2 "Tm1637ModuleTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/EventLog.h>
#include <ace_segment/testing/TestableTmi1637Interface.h>

using aunit::TestRunner;
using ace_segment::testing::TestableTmi1637Interface;
using ace_segment::testing::EventType;
using ace_segment::testing::gEventLog;
using ace_segment::Tm1637Module;

//----------------------------------------------------------------------------

const uint8_t NUM_DIGITS = 4;
TestableTmi1637Interface tmiInterface;
using TmModule = Tm1637Module<TestableTmi1637Interface, NUM_DIGITS>;
TmModule tm1637Module(tmiInterface);

test(Tm1637ModuleTest, flushIncremental) {
  tmiInterface.begin();
  tm1637Module.begin();
  gEventLog.clear();

  // Verify dirty bits initial start dirty.
  assertTrue(tm1637Module.isAnyDigitDirty());
  assertTrue(tm1637Module.isBrightnessDirty());
  tm1637Module.clearDigitsDirty();
  tm1637Module.clearBrightnessDirty();

  // Verify mFlushStage
  assertEqual(0, tm1637Module.mFlushStage);

  // Set digit 0.
  tm1637Module.setPatternAt(1, 0x11);
  assertTrue(tm1637Module.isDigitDirty(1));

  // Set brightness.
  tm1637Module.setBrightness(2);
  assertTrue(tm1637Module.isBrightnessDirty());

  // Iteration 0 sends digit 0, which did not change, so send nothing.
  assertEqual(0, tm1637Module.mFlushStage);
  gEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(0, gEventLog.getNumRecords());
  assertFalse(tm1637Module.isDigitDirty(0));
  assertEqual(1, tm1637Module.mFlushStage);

  // Iteration 1 sends digit 1, which changed, so send digit 1;
  assertEqual(1, tm1637Module.mFlushStage);
  gEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(7, gEventLog.getNumRecords());
  assertTrue(gEventLog.assertEvents(
    7,
    (int) EventType::kTmi1637StartCondition,
    (int) EventType::kTmi1637SendByte, TmModule::kDataCmdFixedAddress,
    (int) EventType::kTmi1637StopCondition,
    (int) EventType::kTmi1637StartCondition,
    (int) EventType::kTmi1637SendByte, TmModule::kAddressCmd | 0x1,
    (int) EventType::kTmi1637SendByte, 0x11,
    (int) EventType::kTmi1637StopCondition
  ));
  assertFalse(tm1637Module.isDigitDirty(1));
  assertEqual(2, tm1637Module.mFlushStage);

  // Iteration 2 sends digit 3, which sends nothing.
  assertEqual(2, tm1637Module.mFlushStage);
  gEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(0, gEventLog.getNumRecords());
  assertFalse(tm1637Module.isDigitDirty(2));
  assertEqual(3, tm1637Module.mFlushStage);

  // Iteration 3 sends digit 3, which sends nothing.
  assertEqual(3, tm1637Module.mFlushStage);
  gEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(0, gEventLog.getNumRecords());
  assertFalse(tm1637Module.isDigitDirty(3));
  assertEqual(4, tm1637Module.mFlushStage);

  // Iteration 4 sends brightness, which changed, so sends package.
  assertEqual(4, tm1637Module.mFlushStage);
  gEventLog.clear();
  tm1637Module.flushIncremental();
  assertEqual(3, gEventLog.getNumRecords());
  assertTrue(gEventLog.assertEvents(
    3,
    (int) EventType::kTmi1637StartCondition,
    (int) EventType::kTmi1637SendByte,
        TmModule::kBrightnessCmd | TmModule::kBrightnessLevelOn | 2,
    (int) EventType::kTmi1637StopCondition
  ));
  assertFalse(tm1637Module.isDigitDirty(4));
  assertEqual(0, tm1637Module.mFlushStage);

  assertFalse(tm1637Module.isFlushRequired());
}

test(Tm1637ModuleTest, flush) {
  tmiInterface.begin();
  tm1637Module.begin();
  gEventLog.clear();

  // Verify dirty bits initial start dirty, 4 digit bits + 1 brightness bit
  assertTrue(tm1637Module.isAnyDigitDirty());
  assertTrue(tm1637Module.isBrightnessDirty());
  tm1637Module.clearDigitsDirty();
  tm1637Module.clearBrightnessDirty();

  // Set digit 1.
  tm1637Module.setPatternAt(1, 0x11);
  assertTrue(tm1637Module.isDigitDirty(1));

  // Set brightness.
  tm1637Module.setBrightness(2);
  assertTrue(tm1637Module.isBrightnessDirty());

  // Calling flush() sends everything.
  gEventLog.clear();
  tm1637Module.flush();
  assertEqual(13, gEventLog.getNumRecords());
  assertTrue(gEventLog.assertEvents(
    13,

    // auto increment mode (3 records)
    (int) EventType::kTmi1637StartCondition,
    (int) EventType::kTmi1637SendByte, TmModule::kDataCmdAutoAddress,
    (int) EventType::kTmi1637StopCondition,

    // send 4 digits (7 records)
    (int) EventType::kTmi1637StartCondition,
    (int) EventType::kTmi1637SendByte, TmModule::kAddressCmd,
    (int) EventType::kTmi1637SendByte, 0x00,
    (int) EventType::kTmi1637SendByte, 0x11,
    (int) EventType::kTmi1637SendByte, 0x00,
    (int) EventType::kTmi1637SendByte, 0x00,
    (int) EventType::kTmi1637StopCondition,

    // brightness (3 records)
    (int) EventType::kTmi1637StartCondition,
    (int) EventType::kTmi1637SendByte,
        TmModule::kBrightnessCmd | TmModule::kBrightnessLevelOn | 2,
    (int) EventType::kTmi1637StopCondition
  ));

  assertFalse(tm1637Module.isBrightnessDirty());
  assertFalse(tm1637Module.isAnyDigitDirty());
  assertFalse(tm1637Module.isFlushRequired());

  tm1637Module.end();
}

test(Tm1637ModuleTest, isFlushRequired) {
  tm1637Module.begin();
  assertTrue(tm1637Module.isFlushRequired());

  tm1637Module.flush();
  assertFalse(tm1637Module.isFlushRequired());

  tm1637Module.setPatternAt(0, 0);
  assertTrue(tm1637Module.isFlushRequired());

  tm1637Module.flush();
  assertFalse(tm1637Module.isFlushRequired());

  tm1637Module.setBrightness(1);
  assertTrue(tm1637Module.isFlushRequired());

  tm1637Module.end();
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
