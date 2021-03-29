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
 * The setCathodeOnGroup() and setAnnodeOnGroup() methods determine the
 * polarity of the LED with respect to the Group line.
 *
 * The invertGroupLevels() should be called when transistors are used
 * on the Group lines to handle higher currents.
 */
class LedMatrix {
  public:

    /**
     * @param cathodeOnGroup the LED common cathodes are on the groups
     * @param transistorsOnGroups transistors used on groups, which
     *    invert the logic level for the group lines
     * @param transistorsOnElements transistors used on elements, which
     *    invert the logic level for the element lines
     * @param numGroups number of group lines
     */
    LedMatrix(
        bool cathodeOnGroup,
        bool transistorsOnGroups,
        bool transistorsOnElements) {

      if (cathodeOnGroup) {
        setCathodeOnGroup();
      } else {
        setAnodeOnGroup();
      }

      if (transistorsOnGroups) {
        invertGroupLevels();
      }
      if (transistorsOnElements) {
        invertElementLevels();
      }
    }

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
    /** LED negative terminals are on the group line. */
    void setCathodeOnGroup() {
      mGroupOn = LOW;
      mGroupOff = HIGH;
      mElementOn = HIGH;
      mElementOff = LOW;
    }

    /** LED positive terminals are on the group line. */
    void setAnodeOnGroup() {
      mGroupOn = HIGH;
      mGroupOff = LOW;
      mElementOn = LOW;
      mElementOff = HIGH;
    }

    /** If a transistor drives the group, invert the logic levels. */
    void invertGroupLevels() {
      mGroupOn ^= 0x1;
      mGroupOff ^=  0x1;
    }

    /** If a transistor drives the elements, invert the logic levels. */
    void invertElementLevels() {
      mElementOn ^= 0x1;
      mElementOff ^=  0x1;
    }

  protected:
    // TODO: Change these to XorMask
    uint8_t mGroupOn;
    uint8_t mGroupOff;
    uint8_t mElementOn;
    uint8_t mElementOff;
};

}

#endif
