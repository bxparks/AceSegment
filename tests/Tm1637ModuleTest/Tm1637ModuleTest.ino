#line 2 "Tm1637ModuleTest.ino"

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
using ace_segment::internal::initialDirtyBits;

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
