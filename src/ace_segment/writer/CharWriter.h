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
#include "PatternWriter.h"

namespace ace_segment {

/**
 * The CharWriter supports mapping of an 8-bit character set to segment patterns
 * supported by LedModule. By default, the ASCII characters (0-127) is
 * provided, but can be overridden with a user-defined character set.
 */
class CharWriter {
  public:
    // Segment patterns for the ASCII character set.
    static const uint8_t kCharPatterns[];

    // Number of characters in the default ASCII character set
    static const uint8_t kNumChars = 128;

    /**
     * Constructor.
     * @param ledModule reference to LedModule
     * @param charPatterns (optional) array of 7-segment character patterns in
     *    PROGMEM (default: an ASCII character set)
     * @param numChars (optional) number of characters in charPatterns, 0 means
     *    256 (default: 128)
     */
    explicit CharWriter(
        LedModule& ledModule,
        const uint8_t charPatterns[] = kCharPatterns,
        uint8_t numChars = kNumChars
    ) :
        mPatternWriter(ledModule),
        mCharPatterns(charPatterns),
        mNumChars(numChars)
    {}

    /** Get the underlying LedModule. */
    LedModule& ledModule() const { return mPatternWriter.ledModule(); }

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return mPatternWriter.getNumDigits(); }

    /** Get number of characters in current character set. */
    uint8_t getNumChars() const { return mNumChars; }

    /** Write the character at the specified position. */
    void writeCharAt(uint8_t pos, char c);

    /** Get segment pattern for character 'c'. */
    uint8_t getPattern(char c) const;

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
    PatternWriter mPatternWriter;
    const uint8_t* const mCharPatterns;
    uint8_t const mNumChars;
};

}

#endif
