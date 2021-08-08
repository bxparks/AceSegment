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
#include "NumberWriter.h"

namespace ace_segment {

// Bit patterns for Hex characters (0x00 - 0x0F) plus a few symbols.
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
const uint8_t kHexCharPatterns[kNumHexCharPatterns] PROGMEM = {
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
  0b01110111, /* A */
  0b01111100, /* b */
  0b00111001, /* C */
  0b01011110, /* d */
  0b01111001, /* E */
  0b01110001, /* F */
  0b00000000, /* (space) */
  0b01000000, /* - */
};

} // ace_segment
