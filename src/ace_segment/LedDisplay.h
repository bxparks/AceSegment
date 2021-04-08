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
     */
    virtual void writePatternsAt(uint8_t pos, const uint8_t patterns[],
        uint8_t len) = 0;

    /**
     * Write the array of `patterns` of length `len`, which are stored in flash
     * memory through PROGMEM, starting at `pos`. If an element of patterns
     * attempts to write to a digit beyond the last digit of the LED module,
     * nothing happens.
     */
    virtual void writePatternsAt_P(uint8_t pos, const uint8_t patterns[],
        uint8_t len) = 0;

    /**
     * Write the brightness for a given pos, leaving pattern unchanged.
     * The maximum brightness is determined by specifics of the subclass. For
     * ScanningDisplay, the maximum brightness is the value of `SUBFIELDS-1`
     * template parameter.
     *
     * A subclass may not support the ability to control the brightness on a per
     * digit basis. In that case, this method does nothing. If the pos is out of
     * bounds, the method also does nothing.
     */
    virtual void setBrightnessAt(uint8_t pos, uint8_t brightness) = 0;

    /**
     * Write the decimal point for the pos. Clock LED modules will attach the
     * colon segment to one of the decimal points.
     */
    virtual void writeDecimalPointAt(uint8_t pos, bool state = true) = 0;

    /**
     * Set global brightness of all digits, with the brightness value expressed
     * as a fraction of 256. In other words, 255 is brightest (default); 0
     * represents off, and 1 is 1/256 of full brightness. If the specific
     * subclass does not support brightness, this method does nothing.
     */
    virtual void setGlobalBrightness(uint8_t brightness) = 0;

    /** Clear all digits to blank pattern. */
    virtual void clear() = 0;

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return mNumDigits; }

  private:
    uint8_t const mNumDigits;
};

} // ace_segment

#endif
