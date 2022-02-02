#line 2 "Tm1638ModuleTest.ino"

/*
 * MIT License
 * Copyright (c) 2022 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/EventLog.h>
#include <ace_segment/testing/TestableTmi1638Interface.h>

using aunit::TestRunner;
using ace_segment::testing::TestableTmi1638Interface;
using ace_segment::testing::EventType;
using ace_segment::testing::gEventLog;
using ace_segment::Tm1638Module;

//----------------------------------------------------------------------------

const uint8_t NUM_DIGITS = 8;
TestableTmi1638Interface tmiInterface;
using TmModule = Tm1638Module<TestableTmi1638Interface, NUM_DIGITS>;
TmModule tm1638Module(tmiInterface);

test(Tm1638ModuleTest, flush) {
  tmiInterface.begin();
  tm1638Module.begin();
  gEventLog.clear();

  // Verify dirty bits initial start dirty, 8 digit bits + 1 brightness bit
  assertTrue(tm1638Module.isAnyDigitDirty());
  assertTrue(tm1638Module.isBrightnessDirty());
  tm1638Module.clearDigitsDirty();
  tm1638Module.clearBrightnessDirty();

  // Set digit 1, 3, 7
  tm1638Module.setPatternAt(1, 0x11);
  assertTrue(tm1638Module.isDigitDirty(1));
  tm1638Module.setPatternAt(3, 0x33);
  assertTrue(tm1638Module.isDigitDirty(3));
  tm1638Module.setPatternAt(7, 0x77);
  assertTrue(tm1638Module.isDigitDirty(7));

  // Set brightness.
  tm1638Module.setBrightness(2);
  assertTrue(tm1638Module.isBrightnessDirty());

  // Calling flush() sends everything.
  gEventLog.clear();
  tm1638Module.flush();
  assertEqual(25, gEventLog.getNumRecords());
  assertTrue(gEventLog.assertEvents(
    25,

    // auto increment mode (3 records)
    (int) EventType::kTmi1638BeginTransaction,
    (int) EventType::kTmi1638Write, TmModule::kDataCmdAutoAddress,
    (int) EventType::kTmi1638EndTransaction,

    // send 8 digits (19 records)
    (int) EventType::kTmi1638BeginTransaction,
    (int) EventType::kTmi1638Write, TmModule::kAddressCmd,
    (int) EventType::kTmi1638Write, 0x00, // seg0
    (int) EventType::kTmi1638Write, 0x00, // seg0
    (int) EventType::kTmi1638Write, 0x11, // seg1
    (int) EventType::kTmi1638Write, 0x00, // seg1
    (int) EventType::kTmi1638Write, 0x00, // seg2
    (int) EventType::kTmi1638Write, 0x00, // seg2
    (int) EventType::kTmi1638Write, 0x33, // seg3
    (int) EventType::kTmi1638Write, 0x00, // seg3
    (int) EventType::kTmi1638Write, 0x00, // seg4
    (int) EventType::kTmi1638Write, 0x00, // seg4
    (int) EventType::kTmi1638Write, 0x00, // seg5
    (int) EventType::kTmi1638Write, 0x00, // seg5
    (int) EventType::kTmi1638Write, 0x00, // seg6
    (int) EventType::kTmi1638Write, 0x00, // seg6
    (int) EventType::kTmi1638Write, 0x77, // seg7
    (int) EventType::kTmi1638Write, 0x00, // seg7
    (int) EventType::kTmi1638EndTransaction,

    // brightness (3 records)
    (int) EventType::kTmi1638BeginTransaction,
    (int) EventType::kTmi1638Write, (
        TmModule::kBrightnessCmd | TmModule::kBrightnessLevelOn | 2
    ),
    (int) EventType::kTmi1638EndTransaction
  ));

  assertFalse(tm1638Module.isBrightnessDirty());
  assertFalse(tm1638Module.isAnyDigitDirty());
  assertFalse(tm1638Module.isFlushRequired());

  tm1638Module.end();
}

test(Tm1638ModuleTest, isFlushRequired) {
  tm1638Module.begin();
  assertTrue(tm1638Module.isFlushRequired());

  tm1638Module.flush();
  assertFalse(tm1638Module.isFlushRequired());

  tm1638Module.setPatternAt(0, 0);
  assertTrue(tm1638Module.isFlushRequired());

  tm1638Module.flush();
  assertFalse(tm1638Module.isFlushRequired());

  tm1638Module.setBrightness(0);
  assertTrue(tm1638Module.isFlushRequired());

  tm1638Module.end();
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
