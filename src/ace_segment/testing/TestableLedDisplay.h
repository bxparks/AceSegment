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

#ifndef ACE_SEGMENT_TESTABLE_LED_DISPLAY_H
#define ACE_SEGMENT_TESTABLE_LED_DISPLAY_H

#include "../LedDisplay.h"

namespace ace_segment {
namespace testing {

/**
 * An implementation of LedDisplay for unit testing purposes.
 * Implements most of the simple features of ScanningDisplay.
 *
 * @tparam DIGITS number of digits supported by this class
 */
template <uint8_t DIGITS>
class TestableLedDisplay : public LedDisplay {
  public:
    TestableLedDisplay() : LedDisplay(DIGITS) {}

    virtual void writePatternAt(uint8_t pos, uint8_t pattern) override {
      if (pos >= DIGITS) return;
      mPatterns[pos] = pattern;
    }

    void writePatternsAt(uint8_t pos, const uint8_t patterns[],
        uint8_t len) override {
      for (uint8_t i = 0; i < len; i++) {
        if (pos >= DIGITS) break;
        mPatterns[pos++] = patterns[i];
      }
    }

    void writePatternsAt_P(uint8_t pos, const uint8_t patterns[],
        uint8_t len) override {
      for (uint8_t i = 0; i < len; i++) {
        if (pos >= DIGITS) break;
        mPatterns[pos++] = pgm_read_byte(patterns + i);
      }
    }

    void writeDecimalPointAt(uint8_t pos, bool state = true) override {
      if (pos >= DIGITS) return;

      uint8_t pattern = mPatterns[pos];
      if (state) {
        pattern |= 0x80;
      } else {
        pattern &= ~0x80;
      }
      mPatterns[pos] = pattern;
    }

    void setBrightnessAt(uint8_t /*pos*/, uint8_t /*brightness*/) override {}

    void setGlobalBrightness(uint8_t /*brightness*/) override {}

    void clear() override {
      for (uint8_t i = 0; i < DIGITS + 1; ++i) {
        mPatterns[i] = 0;
      }
    }

    uint8_t* getPatterns() { return mPatterns; }

  private:
    uint8_t mPatterns[DIGITS + 1];
};

} // testing
} // ace_segment

#endif
