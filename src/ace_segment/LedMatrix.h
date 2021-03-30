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

#ifndef ACE_SEGMENT_LED_MATRIX_H
#define ACE_SEGMENT_LED_MATRIX_H

// TODO: Replace with kLow (0x00) and kHigh (0xFF)
#include <Arduino.h> // LOW and HIGH
#if LOW != 0 || HIGH != 1
  #error LOW is not 0 or HIGH is not 1
#endif

namespace ace_segment {

class Hardware;

/**
 * Class that represents the abstraction of a particular LED display wiring, and
 * knows how to turn off and turn on a specific group of LEDs with a specific
 * pattern. This class is stateless, it does not know what is currently being
 * displayed on the LED segments.
 *
 * There are roughly 2 slightly different APIs:
 *
 *  * LedMatrixSplit
 *    * Assumes that the Group pins are directly controllable using
 *      digitalWrite() methods.
 *    * The rendering methods are `enableGroup()`, `disableGroup()`
 *      and `drawElements()`.
 *    * Two subclasses are provided: 
 *      * LedMatrixDirect: The Segment pins are directly controlled.
 *      * LedMatrixPartialSpi: The Segment pins are attached to an 74HC595 chip,
 *        and is configured using SPI (either software or hardware).
 *  * LedMatrixFullSpi
 *    * Both the Group and Element pins are controlled by 74HC595 chips
 *      using SPI (software or hardware).
 *    * The rendering method is just the `draw()` method which sets both
 *      the group and element states.
 *
 * If the resistors are on the segments, then the segments become the Elements
 * and the digits become the Groups.
 *
 * If the resistors are on the digits, then the digits become the Elements and
 * the segments become the Groups. This configuration is not very useful and has
 * not been tested very much.
 *
 * The groupOnPattern and elementOnPattern is the bit pattern that activates
 * the group or element. For example, a Common Cathode places the negative
 * end of the LED on the group pin and the element pins are positive. So
 * groupOnPattern should be kActiveLowPattern and elementOnPattern should be
 * kActiveHighPattern. However, if a driver transitor is placed on the group
 * pins to handle the higher current, then it inverts the logic on the group
 * pins, so groupOnPattern must be set to kActiveHighPattern.
 */
class LedMatrix {
  public:

    /** Bit pattern to indicate that logical 1 activates group or element. */
    static constexpr uint8_t kActiveHighPattern = 0xFF;

    /** Bit pattern to indicate that logical 0 activates group or element. */
    static constexpr uint8_t kActiveLowPattern = 0x00;

    /**
     * @param groupOnPattern bit pattern that turns on the groups
     * @param elementOnPattern bit pattern that turns on the elements on group
     */
    LedMatrix(
        uint8_t groupOnPattern,
        uint8_t elementOnPattern
    ) :
        mGroupXorMask(~groupOnPattern),
        mElementXorMask(~elementOnPattern)
    {}

    /** Configure the pins for the given LED wiring. */
    virtual void begin() = 0;

    /** Turn off the pins by doing the opposite of begin(). */
    virtual void end() = 0;

    /** Write element patterns for the given group. */
    virtual void draw(uint8_t group, uint8_t elementPattern) = 0;

    /** Disable the elements of given group. */
    virtual void disableGroup(uint8_t group) = 0;

    /** Enable the elements of given group. */
    virtual void enableGroup(uint8_t group) = 0;

    /** Clear everything. */
    virtual void clear() = 0;

  protected:
    uint8_t const mGroupXorMask;
    uint8_t const mElementXorMask;
};

}

#endif
