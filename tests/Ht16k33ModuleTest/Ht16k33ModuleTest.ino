#line 2 "Ht16k33ModuleTest.ino"

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
using ace_segment::Ht16k33Module;

//----------------------------------------------------------------------------

const uint8_t NUM_DIGITS = 4;
const uint8_t HT16K33_I2C_ADDRESS = 0x70;
TestableWireInterface wireInterface;
Ht16k33Module<TestableWireInterface, NUM_DIGITS> ht16k33Module(
    wireInterface, HT16K33_I2C_ADDRESS);

test(Ht16k33ModuleTest, patternForChipPos_colonDisabled) {
  uint8_t patterns[4] = {0x00, 0x01|0x80, 0x02, 0x03|0x80};
  bool enableColon = false;
  assertEqual(0x00, ht16k33Module.patternForChipPos(0, patterns, enableColon));
  assertEqual(0x01|0x80,
      ht16k33Module.patternForChipPos(1, patterns, enableColon));
  assertEqual(0x00, ht16k33Module.patternForChipPos(2, patterns, enableColon));
  assertEqual(0x02, ht16k33Module.patternForChipPos(3, patterns, enableColon));
  assertEqual(
      0x03|0x80,
      ht16k33Module.patternForChipPos(4, patterns, enableColon));
}

test(Ht16k33ModuleTest, patternForChipPos_colonEnabled) {
  uint8_t patterns[4] = {0x00, 0x01|0x80, 0x02, 0x03|0x80};
  bool enableColon = true;
  assertEqual(0x00, ht16k33Module.patternForChipPos(0, patterns, enableColon));
  assertEqual(0x01, ht16k33Module.patternForChipPos(1, patterns, enableColon));

  // bit 1 transfered from bit 7 of digit 1
  assertEqual(0x02, ht16k33Module.patternForChipPos(2, patterns, enableColon));

  assertEqual(0x02, ht16k33Module.patternForChipPos(3, patterns, enableColon));
  assertEqual(
      0x03|0x80,
      ht16k33Module.patternForChipPos(4, patterns, enableColon));
}

test(Ht16k33ModuleTest, isFlushRequired) {
  ht16k33Module.begin();
  assertTrue(ht16k33Module.isFlushRequired());

  ht16k33Module.flush();
  assertFalse(ht16k33Module.isFlushRequired());

  ht16k33Module.setPatternAt(0, 0);
  assertTrue(ht16k33Module.isFlushRequired());

  ht16k33Module.flush();
  assertFalse(ht16k33Module.isFlushRequired());

  ht16k33Module.end();
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
