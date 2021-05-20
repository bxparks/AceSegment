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

#ifndef ACE_SEGMENT_LED_MATRIX_BASE_H
#define ACE_SEGMENT_LED_MATRIX_BASE_H

namespace ace_segment {

/** Bit pattern to indicate that logical 1 activates group or element. */
static const uint8_t kActiveHighPattern = 0xFF;

/** Bit pattern to indicate that logical 0 activates group or element. */
static const uint8_t kActiveLowPattern = 0x00;

/**
 * Class that represents the abstraction of a particular LED display wiring, and
 * knows how to turn off and turn on a specific group of LEDs with a specific
 * pattern. This class is conceptually stateless. The API exposes a class that
 * does not remember the LED element patterns that is currently being displayed.
 * (However, an implementation class may need to cache a small bit of
 * information to implement this API abstraction.)
 *
 * I have provided 3 wiring implementations:
 *
 *  * LedMatrixDirect
 *      * The element and group pins are directly attached to GPIO pins
 *        on the microcontroller.
 *  * LedMatrixSingleHc595
 *      * The element pins are attached to one 74HC595 shift register chip,
 *        accessed through SPI (either software or hardware).
 *      * The group pins are directly attached to GPIO pins.
 *  * LedMatrixDualHc595
 *    * Both the element and group pins are controlled by 74HC595 chips
 *      using SPI (software or hardware).
 *
 * In most cases, the resistors will be on the segments to control the current
 * on each segment, so the segments become elements and the digits become the
 * groups.
 *
 * If the resistors are on the digits, then the digits become the elements and
 * the segments become the groups. This configuration is not very useful and has
 * not been tested very much.
 *
 * The elementOnPattern and groupOnPattern are the bit patterns that activate
 * the element and group. For example, a Common Cathode LED module places the
 * negative end of the LED on the group pin and the element pins are positive.
 * So elementOnPattern should be kActiveHighPattern and groupOnPattern should be
 * kActiveLowPattern. However, if a driver transitor is placed on the group pins
 * to handle the higher current, then it inverts the logic on the group pins, so
 * groupOnPattern must be set to kActiveHighPattern.
 *
 * The elementPattern and groupOnPattern are compile-time constants so these
 * parameters could be moved into the template parameters. When I did that, the
 * flash size when down by only ~20 bytes on AVR, and ~40 bytes on an STM32. I
 * decided to leave them as instance variables, because the decrease in
 * readability of the code didn't seem worth 20 bytes.
 */
class LedMatrixBase {
  public:

    /**
     * @param elementOnPattern bit pattern that turns on the elements on group
     * @param groupOnPattern bit pattern that turns on the groups
     */
    LedMatrixBase(
        uint8_t elementOnPattern,
        uint8_t groupOnPattern
    ) :
        mElementXorMask(~elementOnPattern),
        mGroupXorMask(~groupOnPattern)
    {}

    /** Configure the pins for the given LED wiring. */
    void begin() const {}

    /** Turn off the pins by doing the opposite of begin(). */
    void end() const {}

    /** Write element patterns for the given group. */
    void draw(uint8_t /*group*/, uint8_t /*elementPattern*/) const {}

    /** Disable the elements of given group. */
    void disableGroup(uint8_t /*group*/) const {}

    /** Enable the elements of given group. */
    void enableGroup(uint8_t /*group*/) const {}

    /** Clear everything. */
    void clear() const {}

  protected:
    uint8_t const mElementXorMask;
    uint8_t const mGroupXorMask;
};

}

#endif
