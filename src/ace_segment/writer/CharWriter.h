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

#ifndef ACE_SEGMENT_CHAR_WRITER_H
#define ACE_SEGMENT_CHAR_WRITER_H

#include <stdint.h>
#include <Arduino.h> // pgm_read_byte()
#include "PatternWriter.h"

namespace ace_segment {

/**
 * The segment pattern to display when an ASCII character cannot be displayed on
 * the LED Module. The original version of this character set used the '?'
 * pattern (0b10000011). But that symjbol does not render very well on LED
 * modules which don't have an active decimal point on all digits. Use a blank
 * digit instead.
 */
const uint8_t kCharUnknown = 0b00000000;

/** Number of characters in the kCharPatterns default ASCII character set. */
const uint8_t kNumCharPatterns = 128;

/** Segment patterns for the ASCII character set. */
extern const uint8_t kCharPatterns[kNumCharPatterns];

/**
 * The CharWriter supports mapping of an 8-bit character set to segment patterns
 * supported by LedModule. By default, the ASCII characters (0-127) is
 * provided, but can be overridden with a user-defined character set.
 *
 * @tparam T_LED_MODULE the class of the underlying LED Module, often LedModule
 *    but other classes with the same generic public methods can be substituted
 */
template <typename T_LED_MODULE>
class CharWriter {
  public:
    /**
     * Constructor.
     * @param ledModule reference to LedModule
     * @param charPatterns (optional) array of 7-segment character patterns in
     *    PROGMEM (default: an ASCII character set)
     * @param numChars (optional) number of characters in charPatterns, 0 means
     *    256 (default: 128)
     */
    explicit CharWriter(
        T_LED_MODULE& ledModule,
        const uint8_t charPatterns[] = kCharPatterns,
        uint8_t numChars = kNumCharPatterns
    ) :
        mPatternWriter(ledModule),
        mCharPatterns(charPatterns),
        mNumChars(numChars)
    {}

    /** Get the underlying LedModule. */
    T_LED_MODULE& ledModule() { return mPatternWriter.ledModule(); }

    /** Get the underlying PatternWriter. */
    PatternWriter<T_LED_MODULE>& patternWriter() { return mPatternWriter; }

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return mPatternWriter.getNumDigits(); }

    /** Get number of characters in current character set. */
    uint8_t getNumChars() const { return mNumChars; }

    /** Write the character at the specified position. */
    void writeCharAt(uint8_t pos, char c) {
      if (pos >= mPatternWriter.getNumDigits()) return;
      mPatternWriter.writePatternAt(pos, getPattern(c));
    }

    /** Get segment pattern for character 'c'. */
    uint8_t getPattern(char c) const {
      uint8_t pattern = ((mNumChars == 0) || ((uint8_t) c < mNumChars))
          ? pgm_read_byte(&mCharPatterns[(uint8_t) c])
          : kCharUnknown;
      return pattern;
    }

    /** Write the decimal point for the pos. */
    void writeDecimalPointAt(uint8_t pos, bool state = true) {
      mPatternWriter.writeDecimalPointAt(pos, state);
    }

    /** Clear the entire display. */
    void clear() { clearToEnd(0); }

    /** Clear the display from `pos` to the end. */
    void clearToEnd(uint8_t pos) { mPatternWriter.clearToEnd(pos); }

  private:
    // disable copy-constructor and assignment operator
    CharWriter(const CharWriter&) = delete;
    CharWriter& operator=(const CharWriter&) = delete;

  private:
    PatternWriter<T_LED_MODULE> mPatternWriter;
    const uint8_t* const mCharPatterns;
    uint8_t const mNumChars;
};

}

#endif
