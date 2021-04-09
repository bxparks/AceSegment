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

testF(NumberWriterTest, writeHexCharAt) {
  numberWriter.writeHexCharAt(0, 0);
  assertEqual(0b00111111, mPatterns[0]);

  numberWriter.writeHexCharAt(1, 0);
  numberWriter.display().writeDecimalPointAt(1);
  assertEqual(0b00111111 | 0x80, mPatterns[1]);

  numberWriter.display().writeDecimalPointAt(1, false);
  assertEqual(0b00111111, mPatterns[1]);
}

testF(NumberWriterTest, writeHexCharAt_outOfBounds) {
  mPatterns[4] = 0;

  numberWriter.writeHexCharAt(4, NumberWriter::kCharMinus);
  numberWriter.display().writeDecimalPointAt(4);
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

testF(NumberWriterTest, writeUnsignedDecimalAt) {
  uint8_t written = numberWriter.writeUnsignedDecimalAt(0, 123);
  assertEqual(3, written);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);
  assertEqual(0b01001111, mPatterns[2]);

  // Even if it off the end, the return value is the number of character that
  // would have been written if the LED module was long enough.
  written = numberWriter.writeUnsignedDecimalAt(3, 123);
  assertEqual(3, written);
}

testF(NumberWriterTest, writeUnsignedDecimalAt_boxed) {
  uint8_t written = numberWriter.writeUnsignedDecimalAt(0, 34, 4);
  assertEqual(4, written);
  assertEqual(0b00000000, mPatterns[0]);
  assertEqual(0b00000000, mPatterns[1]);
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b01100110, mPatterns[3]);

  written = numberWriter.writeUnsignedDecimalAt(0, 34, 2);
  assertEqual(2, written);
}

testF(NumberWriterTest, writeSignedDecimalAt) {
  uint8_t written = numberWriter.writeSignedDecimalAt(0, -23);
  assertEqual(3, written);
  assertEqual(0b01000000, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);
  assertEqual(0b01001111, mPatterns[2]);
}

testF(NumberWriterTest, writeSignedDecimalAt_boxed) {
  uint8_t written = numberWriter.writeSignedDecimalAt(0, -12, 4);
  assertEqual(4, written);
  assertEqual(0b00000000, mPatterns[0]);
  assertEqual(0b01000000, mPatterns[1]);
  assertEqual(0b00000110, mPatterns[2]);
  assertEqual(0b01011011, mPatterns[3]);
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

testF(ClockWriterTest, writeBcd2At) {
  clockWriter.writeBcd2At(0, 0x12);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);

  clockWriter.writeBcd2At(2, 0x34);
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b01100110, mPatterns[3]);
}

testF(ClockWriterTest, writeDec2At) {
  clockWriter.writeDec2At(0, 12);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);

  clockWriter.writeDec2At(2, 34);
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b01100110, mPatterns[3]);
}

testF(ClockWriterTest, writeDec4At) {
  clockWriter.writeDec4At(0, 1234);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011, mPatterns[1]);
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b01100110, mPatterns[3]);
}

testF(ClockWriterTest, writeColon) {
  clockWriter.writeDec2At(0, 12);
  clockWriter.writeDec2At(2, 34);

  // no colon by default
  assertEqual(0b01011011, mPatterns[1]);

  // turn on colon
  clockWriter.writeColon(true); // turns it on
  assertEqual(0b01011011 | 0x80, mPatterns[1]);
}

testF(ClockWriterTest, writeHourMinute) {
  clockWriter.writeHourMinute(12, 34);
  assertEqual(0b00000110, mPatterns[0]);
  assertEqual(0b01011011 | 0x80, mPatterns[1]); // colon on by default
  assertEqual(0b01001111, mPatterns[2]);
  assertEqual(0b01100110, mPatterns[3]);
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
