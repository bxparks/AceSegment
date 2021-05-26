/*
MIT License

Copyright (c) 2021 Brian T. Park

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

#ifndef ACE_SEGMENT_LEVEL_WRITER_H
#define ACE_SEGMENT_LEVEL_WRITER_H

#include <stdint.h>
#include "../LedDisplay.h"

namespace ace_segment {

/**
 * Emulate a level led module using a left vertical bar and a right
 * vertical bar on each digit. Since each digit can represent 2 levels, the
 * range of levels for the entire LED display is `[0, 2*numDigits]`.
 */
class LevelWriter {
  private:
    // left vertical bar
    static const uint8_t kLevelLeftPattern = 0b00110000;
    // right vertical bar
    static const uint8_t kLevelRightPattern = 0b00000110;

  public:
    /** Constructor. */
    explicit LevelWriter(LedDisplay& ledDisplay) :
        mLedDisplay(ledDisplay)
        {}

    /** Get the underlying LedDisplay. */
    LedDisplay& display() const { return mLedDisplay; }

    /**
     * Return the maximum level supported by this LED display. The range is [0,
     * maxLevel].
     */
    uint8_t getMaxLevel() const {
      return mLedDisplay.getNumDigits() * 2;
    }

    /** Write out the level bar, 2 levels per digit. */
    void writeLevel(uint8_t level) {
      uint8_t fullDigits = level / 2;
      uint8_t partialDigit = level & 0x1;
      uint8_t numDigits = mLedDisplay.getNumDigits();

      uint8_t pos = 0;
      while (pos < fullDigits && pos < numDigits) {
        mLedDisplay.writePatternAt(
            pos++, kLevelLeftPattern | kLevelRightPattern);
      }
      if (partialDigit && pos < numDigits) {
        mLedDisplay.writePatternAt(pos++, kLevelLeftPattern);
      }
      mLedDisplay.clearToEnd(pos);
    }

  private:
    // disable copy-constructor and assignment operator
    LevelWriter(const LevelWriter&) = delete;
    LevelWriter& operator=(const LevelWriter&) = delete;

  private:
    LedDisplay& mLedDisplay;
};

}

#endif
