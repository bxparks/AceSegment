#line 2 "RemapTest.ino"

/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnitVerbose.h>
#include <AceSegment.h>

using aunit::TestRunner;
using ace_segment::internal::invertRemapArray;

//----------------------------------------------------------------------------

// If the logical and physical positions are related so that they are
// non-overlapping pair-wise swappped, then the remapArray is identical to its
// inverse.
test(invertRemapArray, self_inversion) {
  const uint8_t NUM_DIGITS = 4;
  const uint8_t remapArray[NUM_DIGITS] = {3, 2, 1, 0};
  uint8_t invertedArray[NUM_DIGITS];

  invertRemapArray(invertedArray, remapArray, NUM_DIGITS);
  assertEqual(3, invertedArray[0]);
  assertEqual(2, invertedArray[1]);
  assertEqual(1, invertedArray[2]);
  assertEqual(0, invertedArray[3]);
}

// If the logical and physical positions are NOT pair-wise swapped, then the two
// arrays are different.
test(invertRemapArray, permuted) {
  const uint8_t NUM_DIGITS = 4;
  const uint8_t remapArray[NUM_DIGITS] = {1, 2, 0, 3};
  uint8_t invertedArray[NUM_DIGITS];

  invertRemapArray(invertedArray, remapArray, NUM_DIGITS);
  assertEqual(2, invertedArray[0]);
  assertEqual(0, invertedArray[1]);
  assertEqual(1, invertedArray[2]);
  assertEqual(3, invertedArray[3]);
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
