#line 2 "WriterTest.ino"

/*
 * MIT License
 * Copyright (c) 2018 Brian T. Park
 */

#include <stdarg.h>
#include <Arduino.h>
#include <AUnit.h>
#include <AceSegment.h>
#include <ace_segment/testing/TestableLedDisplay.h>

using aunit::TestRunner;
using aunit::TestOnce;
using namespace ace_segment;
using namespace ace_segment::testing;

const uint8_t NUM_DIGITS = 4;
const uint8_t CUSTOM_BRIGHTNESS = 64;
const uint8_t DIFFERENT_BRIGHTNESS = 32;

TestableLedDisplay<NUM_DIGITS> ledDisplay;
NumberWriter numberWriter(ledDisplay);
ClockWriter clockWriter(ledDisplay);
CharWriter charWriter(ledDisplay);
StringWriter stringWriter(charWriter);

// ----------------------------------------------------------------------
// Tests for CharWriter.
// ----------------------------------------------------------------------

class CharWriterTest : public TestOnce {
  protected:
    void setup() override {
      ledDisplay.clear();
      mPatterns = ledDisplay.getPatterns();
    }

    uint8_t* mPatterns;
};

testF(CharWriterTest, writeAt) {
  charWriter.writeCharAt(0, '0');
  assertEqual(0b00111111, mPatterns[0]);
}

testF(CharWriterTest, writeAt_outOfBounds) {
  mPatterns[4] = 0;

  charWriter.writeCharAt(4, 'a');
  assertEqual(0, mPatterns[4]);
}

// ----------------------------------------------------------------------
// Tests for StringWriter.
// ----------------------------------------------------------------------

class StringWriterTest : public TestOnce {
  protected:
    void setup() override {
      ledDisplay.clear();
      mPatterns = ledDisplay.getPatterns();
    }

    void assertPatternsEqual(int n, ...) {
      va_list args;
      va_start(args, n);
      for (int i = 0; i < n; i++) {
        uint8_t pattern = va_arg(args, int);
        assertEqual(pattern, mPatterns[i]);
      }
      va_end(args);
    }

    uint8_t* mPatterns;
};

testF(StringWriterTest, writeStringAt) {
  stringWriter.writeStringAt(0, ".1.2.3");

  // Should be (".", "1.", "2.", "3") as the 4 digits
  assertPatternsEqual(
    4,
    0b10000000,
    0b00000110 | 0x80,
    0b01011011 | 0x80,
    0b01001111
  );
}

// ----------------------------------------------------------------------
// Tests for NumberWriter.
// ----------------------------------------------------------------------

class NumberWriterTest : public TestOnce {
  protected:
    void setup() override {
      ledDisplay.clear();
      mPatterns = ledDisplay.getPatterns();
    }

    uint8_t* mPatterns;
};

testF(NumberWriterTest, writeRawHexCharAt) {
  numberWriter.writeHexCharAt(0, 0);
  assertEqual(0b00111111, mPatterns[0]);
}

testF(NumberWriterTest, writeHexCharAt) {
  numberWriter.writeHexCharAt(0, 0);
  assertEqual(0b00111111, mPatterns[0]);

  numberWriter.writeHexCharAt(1, 0);
  ledDisplay.writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, mPatterns[1]);

  ledDisplay.writeDecimalPointAt(1, false);
  assertEqual(0b00111111, mPatterns[1]);
}

testF(NumberWriterTest, writeHexCharAt_outOfBounds) {
  mPatterns[4] = 0;

  numberWriter.writeHexCharAt(4, NumberWriter::kMinus);
  ledDisplay.writeDecimalPointAt(4);
  assertEqual(0, mPatterns[4]);
}

testF(NumberWriterTest, writeHexByteAt) {
  numberWriter.writeHexByteAt(0, 0x1F);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01110001, mPatterns[1]);
}

testF(NumberWriterTest, writeHexWordAt) {
  numberWriter.writeHexWordAt(0, 0x1FF1);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01110001, mPatterns[1]);
  assertEqual(0b01110001, mPatterns[2]);
  assertEqual(0b00000110, mPatterns[3]);
}

testF(NumberWriterTest, writeDecWordAt) {
  numberWriter.writeDecWordAt(0, 1234);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b01100110, mPatterns[3]);
}

// ----------------------------------------------------------------------
// Tests for ClockWriter.
// ----------------------------------------------------------------------

class ClockWriterTest: public TestOnce {
  protected:
    void setup() override {
      ledDisplay.clear();
      mPatterns = ledDisplay.getPatterns();
    }

    uint8_t* mPatterns;
};

testF(ClockWriterTest, writeCharAt) {
  clockWriter.writeCharAt(0, ClockWriter::kP);
  assertEqual(0b01110011, mPatterns[0]);
}

testF(ClockWriterTest, writeBcdAt) {
  clockWriter.writeBcdAt(0, 0x12);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);

  clockWriter.writeBcdAt(2, 0x89);
  assertEqual(0b01111111, mPatterns[2]);
  assertEqual(0b01101111, mPatterns[3]);
}

testF(ClockWriterTest, writeDecimalAt) {
  clockWriter.writeDecimalAt(0, 12);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);

  clockWriter.writeDecimalAt(2, 89);
  assertEqual(0b01111111, mPatterns[2]);
  assertEqual(0b01101111, mPatterns[3]);
}

testF(ClockWriterTest, writeBcdClock) {
  clockWriter.writeBcdClock(0x12, 0x30);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011 | 0x80, mPatterns[1]);
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b00111111, mPatterns[3]);
}

testF(ClockWriterTest, writeClock) {
  clockWriter.writeClock(12, 30);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011 | 0x80, mPatterns[1]);
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b00111111, mPatterns[3]);
}

testF(ClockWriterTest, writeColon) {
  clockWriter.writeClock(12, 30); // colon on by default
  assertEqual(0b01011011 | 0x80, mPatterns[1]);

  clockWriter.writeColon(false); // turns it off
  assertEqual(0b01011011, mPatterns[1]);
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
