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

#ifndef ACE_SEGMENT_DIMMING_DIGIT_H
#define ACE_SEGMENT_DIMMING_DIGIT_H

#include <stdint.h>

namespace ace_segment {

class DimmingDigit {
  public:
    static const uint8_t kOff = 0;
    static const uint8_t kOn = 255;

    DimmingDigit():
      pattern(0),
      brightness(kOn)
    {}

    uint8_t pattern; // segment bit pattern of the digit
    uint8_t brightness; // units of 1/256

  private:
    // disable copy-constructor and assignment operator
    DimmingDigit(const DimmingDigit&) = delete;
    DimmingDigit& operator=(const DimmingDigit&) = delete;
};

}

#endif
