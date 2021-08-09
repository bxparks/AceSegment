#line 2 "Max7219ModuleTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>
#include <ace_segment/testing/TestableSpiInterface.h>

using aunit::TestRunner;
using ace_segment::testing::TestableSpiInterface;
using ace_segment::Max7219Module;
using ace_segment::internal::convertPatternMax7219;

//----------------------------------------------------------------------------

const uint8_t NUM_DIGITS = 8;
TestableSpiInterface spiInterface;
Max7219Module<TestableSpiInterface, NUM_DIGITS> max7219Module(
    spiInterface);

test(Max7219ModuleTest, convertPatternMax7219) {
  assertEqual(0b01010001, convertPatternMax7219(0b01000101));
  assertEqual(0b11010001, convertPatternMax7219(0b11000101));
}

test(Max7219ModuleTest, isFlushRequired) {
  max7219Module.begin();

  max7219Module.flush();
  assertFalse(max7219Module.isFlushRequired());

  max7219Module.setPatternAt(0, 0);
  assertTrue(max7219Module.isFlushRequired());

  max7219Module.flush();
  assertFalse(max7219Module.isFlushRequired());

  max7219Module.end();
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
