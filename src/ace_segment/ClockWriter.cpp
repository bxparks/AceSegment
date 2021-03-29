/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Arduino.h>
#include "ClockWriter.h"

namespace ace_segment {

// Bit patterns for clock characters (0 - 9) plus a few symbols needed by a
// clock.
// Adapted from https://github.com/dmadison/LED-Segment-ASCII.
//
// 7-segment map:
//       AAA         000
//      F   B       5   1
//      F   B  H    5   1  7
//       GGG         666
//      E   C  H    4   2  7
//      E   C       4   2
//       DDD  H     333  7
//
// Segment: H G F E D C B A
//    Bits: 7 6 5 4 3 2 1 0
//
// On some LED modules, the H segment is attached to the ":" on Digit 2
// (counting from the left) and the decimal point on all other digits.  On
// other LED modules, the H is attached only to the ":" on Digit 2, and and
// there are no working decimal points.
//
const uint8_t ClockWriter::kCharacterArray[] PROGMEM = {
  0b00111111, /* 0 */
  0b00000110, /* 1 */
  0b01011011, /* 2 */
  0b01001111, /* 3 */
  0b01100110, /* 4 */
  0b01101101, /* 5 */
  0b01111101, /* 6 */
  0b00000111, /* 7 */
  0b01111111, /* 8 */
  0b01101111, /* 9 */
  0b00000000, /* (space) */
  0b01000000, /* - */
  0b01110111, /* A */
  0b01110011, /* P */
  0b01100011, /* degrees symbol */
};

const uint8_t ClockWriter::kNumCharacters =
    sizeof(kCharacterArray)/sizeof(kCharacterArray[0]);

void ClockWriter::writeCharAt(uint8_t pos, uint8_t c) {
  if (pos >= mSegmentDisplay->getNumDigits()) return;
  uint8_t pattern = ((uint8_t) c < kNumCharacters)
      ? pgm_read_byte(&kCharacterArray[(uint8_t) c])
      : kSpace;
  mSegmentDisplay->writePatternAt(pos, pattern);
}

void ClockWriter::writeBcdAt(uint8_t pos, uint8_t bcd) {
  uint8_t c0 = (bcd & 0xF0) >> 4;
  uint8_t c1 = (bcd & 0x0F);
  if (c0 > 9) c0 = kSpace;
  if (c1 > 9) c1 = kSpace;
  writeCharAt(pos++, c0);
  writeCharAt(pos, c1);
}

void ClockWriter::writeDecimalAt(uint8_t pos, uint8_t d) {
  uint8_t bcd = toBcd(d);
  writeBcdAt(pos, bcd);
}

void ClockWriter::writeClock(uint8_t hh, uint8_t mm) {
  uint8_t hhBcd = toBcd(hh);
  uint8_t mmBcd = toBcd(mm);
  writeBcdClock(hhBcd, mmBcd);
  writeColon(true);
}

void ClockWriter::writeBcdClock(uint8_t hhBcd, uint8_t mmBcd) {
  writeBcdAt(0, hhBcd);
  writeBcdAt(2, mmBcd);
  writeColon(true);
}

uint8_t ClockWriter::toBcd(uint8_t d) {
  if (d >= 100) return 0xFF;
  uint8_t h1 = d / 10;
  uint8_t h0 = d - (h1 * 10);
  return (h1 << 4) | h0;
}

}
