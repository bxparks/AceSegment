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

#ifndef ACE_SEGMENT_LED_MODULE_H
#define ACE_SEGMENT_LED_MODULE_H

#include <stdint.h>

namespace ace_segment {

/**
 * General interface that represents a generic seven-segment LED module with
 * multiple digits. Subclasses will support different driver chips (e.g. TM1637,
 * MAX7219, or even a 74HC595).
 *
 * The digit addressing scheme is normalized at this layer so that digit 0 is
 * the left most digit, and digit 'numDigits-1' is the right most digit. The
 * segment addressing is also normalized so that bit 0 is the 'A' segment, bit 6
 * is 'G' segment, and bit 7 is the decimal point. If an LED module does not
 * conform to this convention, the implement class for that hardware must remap
 * the digit and segment addresses.
 */
class LedModule {
  public:
    /**
     * Constructor.
     * @param numDigits number of digits in the LED module; this value is
     *    returned by getNumDigits(). The value is usually a compile-time
     *    constant, so it is faster/cheaper to just use the constant value.
     *    However, sometimes your code needs this value but it has only a
     *    reference/pointer to the LedModule (or one of its subclasses). Then
     *    getNumDigits() can be used.
     */
    explicit LedModule(uint8_t numDigits) : mNumDigits(numDigits) {}

    /**
     * Return the number of digits supported by this display instance.
     * This method is deliberately non-virtual for performance reasons.
     */
    uint8_t getNumDigits() const { return mNumDigits; }

    /** Set the led digit pattern at position pos. */
    virtual void setPatternAt(uint8_t pos, uint8_t pattern) = 0;

    /** Get the led digit pattern at position pos. */
    virtual uint8_t getPatternAt(uint8_t pos) const = 0;

    /**
     * Set global brightness of all digits. Different subclasses will interpret
     * the brightness integer value differently.
     */
    virtual void setBrightness(uint8_t brightness) = 0;

  private:
    // disable copy-constructor and assignment operator
    LedModule(const LedModule&) = delete;
    LedModule& operator=(const LedModule&) = delete;

  private:
    uint8_t const mNumDigits;
};

} // ace_segment

#endif
