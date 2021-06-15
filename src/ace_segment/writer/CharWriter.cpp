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
#include "CharWriter.h"

namespace ace_segment {

// A poor-man's '?': 0b10000011
//static const uint8_t UNKNOWN = 0b10000011;

// Set unknown character to be blank. The problem with '?' is that some LED
// modules don't have an active decimal point on all digits, so the '?' does not
// look right.
static const uint8_t UNKNOWN = 0b00000000;

// Bit patterns for ASCII characters (0 - 127).
// Adapted from https://github.com/dmadison/LED-Segment-ASCII.
//
// 7-segment map:
//       AAA       000
//      F   B     5   1
//      F   B     5   1
//       GGG       666
//      E   C     4   2
//      E   C     4   2
//       DDD  DP   333  77
//
// Segment: DP G F E D C B A
//    Bits: 7  6 5 4 3 2 1 0
//
const uint8_t CharWriter::kCharPatterns[] PROGMEM = {
     UNKNOWN, /* 00 */
     UNKNOWN, /* 01 */
     UNKNOWN, /* 02 */
     UNKNOWN, /* 03 */
     UNKNOWN, /* 04 */
     UNKNOWN, /* 05 */
     UNKNOWN, /* 06 */
     UNKNOWN, /* 07 */
     UNKNOWN, /* 08 */
     UNKNOWN, /* 09 */
     UNKNOWN, /* 10 */
     UNKNOWN, /* 11 */
     UNKNOWN, /* 12 */
     UNKNOWN, /* 13 */
     UNKNOWN, /* 14 */
     UNKNOWN, /* 15 */
     UNKNOWN, /* 16 */
     UNKNOWN, /* 17 */
     UNKNOWN, /* 18 */
     UNKNOWN, /* 19 */
     UNKNOWN, /* 20 */
     UNKNOWN, /* 21 */
     UNKNOWN, /* 22 */
     UNKNOWN, /* 23 */
     UNKNOWN, /* 24 */
     UNKNOWN, /* 25 */
     UNKNOWN, /* 26 */
     UNKNOWN, /* 27 */
     UNKNOWN, /* 28 */
     UNKNOWN, /* 29 */
     UNKNOWN, /* 30 */
     UNKNOWN, /* 31 */
  0b00000000, /* (space) */
  0b10000110, /* ! */
  0b00100010, /* " */
  0b01111110, /* # */
  0b01101101, /* $ */
  0b11010010, /* % */
  0b01000110, /* & */
  0b00100000, /* ' */
  0b00101001, /* ( */
  0b00001011, /* ) */
  0b00100001, /* * */
  0b01110000, /* + */
  0b00010000, /* , */
  0b01000000, /* - */
  0b10000000, /* . */
  0b01010010, /* / */
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
  0b00001001, /* : */
  0b00001101, /* ; */
  0b01100001, /* < */
  0b01001000, /* = */
  0b01000011, /* > */
  0b11010011, /* ? */
  0b01011111, /* @ */
  0b01110111, /* A */
  0b01111100, /* B */
  0b00111001, /* C */
  0b01011110, /* D */
  0b01111001, /* E */
  0b01110001, /* F */
  0b00111101, /* G */
  0b01110110, /* H */
  0b00110000, /* I */
  0b00011110, /* J */
  0b01110101, /* K */
  0b00111000, /* L */
  0b00010101, /* M */
  0b00110111, /* N */
  0b00111111, /* O */
  0b01110011, /* P */
  0b01101011, /* Q */
  0b00110011, /* R */
  0b01101101, /* S */
  0b01111000, /* T */
  0b00111110, /* U */
  0b00111110, /* V */
  0b00101010, /* W */
  0b01110110, /* X */
  0b01101110, /* Y */
  0b01011011, /* Z */
  0b00111001, /* [ */
  0b01100100, /* \ */
  0b00001111, /* ] */
  0b00100011, /* ^ */
  0b00001000, /* _ */
  0b00000010, /* ` */
  0b01011111, /* a */
  0b01111100, /* b */
  0b01011000, /* c */
  0b01011110, /* d */
  0b01111011, /* e */
  0b01110001, /* f */
  0b01101111, /* g */
  0b01110100, /* h */
  0b00010000, /* i */
  0b00001100, /* j */
  0b01110101, /* k */
  0b00110000, /* l */
  0b00010100, /* m */
  0b01010100, /* n */
  0b01011100, /* o */
  0b01110011, /* p */
  0b01100111, /* q */
  0b01010000, /* r */
  0b01101101, /* s */
  0b01111000, /* t */
  0b00011100, /* u */
  0b00011100, /* v */
  0b00010100, /* w */
  0b01110110, /* x */
  0b01101110, /* y */
  0b01011011, /* z */
  0b01000110, /* { */
  0b00110000, /* | */
  0b01110000, /* } */
  0b00000001, /* ~ */
     UNKNOWN, /* (del) */
};

uint8_t CharWriter::getPattern(char c) const {
  uint8_t pattern = ((mNumChars == 0) || ((uint8_t) c < mNumChars))
      ? pgm_read_byte(&mCharPatterns[(uint8_t) c])
      : UNKNOWN;
  return pattern;
}

void CharWriter::writeCharAt(uint8_t pos, char c) {
  if (pos >= mPatternWriter.getNumDigits()) return;
  mPatternWriter.writePatternAt(pos, getPattern(c));
}

}
