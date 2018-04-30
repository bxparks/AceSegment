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

#ifndef ACE_SEGMENT_STYLED_DIGIT_H
#define ACE_SEGMENT_STYLED_DIGIT_H

#include <stdint.h>

namespace ace_segment {

/**
 * Data structure that keeps track of the state of each digit (its segment bit
 * pattern and its style).
 */
class StyledPattern {
  public:
    static const uint8_t kStyleNormal = 0;

    StyledPattern():
      pattern(0),
      style(kStyleNormal)
    {}

    void setDecimalPoint() {
      pattern |= 0x80;
    }

    void clearDecimalPoint() {
      pattern &= ~0x80;
    }

    uint8_t pattern;
    uint8_t style;
};

}

#endif
