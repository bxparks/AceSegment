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

#ifndef ACE_SEGMENT_LED_DISPLAY_H
#define ACE_SEGMENT_LED_DISPLAY_H

#include <stdint.h>
#include <Arduino.h> // pgm_read_byte()

namespace ace_segment {

/**
 * General interface for writing LED segment patterns to the LED display module.
 * Various 'Writer' classes provide additional functionality on top of this
 * interface:
 *
 *  * NumberWriter: write numberse in decimal or hexadecimal format
 *  * ClockWriter: write strings of the form "HH:MM"
 *  * CharWriter: write ASCII characters in the range of 0-127
 *  * StringWriter: uses a CharWriter to write c-strings
 */
class LedDisplay {
  public:
    /**
     * Constructor.
     *
     * @param numDigits number of digits in the LED module; this value is
     *    returned by getNumDigits(). The value is usually a compile-time
     *    constant, so it is faster/cheaper to just use the constant value.
     *    However, sometimes your code needs this value but it has only a
     *    reference/pointer to the LedDisplay (or one of its subclasses). Then
     *    getNumDigits() can be used.
     */
    LedDisplay(uint8_t numDigits) : mNumDigits(numDigits) {}

    /**
     * Write the pattern for a given pos. If the pos is out of bounds, the
     * method does nothing.
     */
    virtual void writePatternAt(uint8_t pos, uint8_t pattern) = 0;

    /**
     * Write the array of `patterns` of length `len`, starting at `pos`. If an
     * element of patterns attempts to write to a digit beyond the last digit of
     * the LED module, nothing happens.
     *
     * The default implementation calls writePatternAt(), which should be
     * sufficient in most cases. Subclasses can override if needed.
     */
    virtual void writePatternsAt(uint8_t pos, const uint8_t patterns[],
        uint8_t len) {
      for (uint8_t i = 0; i < len; i++) {
        if (pos >= mNumDigits) break;
        writePatternAt(pos++, patterns[i]);
      }
    }

    /**
     * Write the array of `patterns` of length `len`, which are stored in flash
     * memory through PROGMEM, starting at `pos`. If an element of patterns
     * attempts to write to a digit beyond the last digit of the LED module,
     * nothing happens.
     *
     * The default implementation calls writePatternAt(), which should be
     * sufficient in most cases. Subclasses can override if needed.
     */
    virtual void writePatternsAt_P(uint8_t pos, const uint8_t patterns[],
        uint8_t len) {
      for (uint8_t i = 0; i < len; i++) {
        if (pos >= mNumDigits) break;
        uint8_t pattern = pgm_read_byte(patterns + i);
        writePatternAt(pos++, pattern);
      }
    }

    /**
     * Write the decimal point for the pos. Clock LED modules will attach the
     * colon segment to one of the decimal points.
     */
    virtual void writeDecimalPointAt(uint8_t pos, bool state = true) = 0;

    /**
     * Set global brightness of all digits. Different subclasses will interpret
     * the brightness integer value differently.
     */
    virtual void setBrightness(uint8_t brightness) = 0;

    /**
     * Clear all digits to blank pattern. The default implementation writes a
     * `0` into each digit using writePatternAt(). Subclasses can override if
     * needed.
     */
    virtual void clear() {
      for (uint8_t i = 0; i < mNumDigits + 1; ++i) {
        writePatternAt(i, 0);
      }
    }

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return mNumDigits; }

  private:
    uint8_t const mNumDigits;
};

} // ace_segment

#endif
