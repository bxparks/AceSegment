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

#include <Arduino.h> // LOW and HIGH

#if LOW != 0 || HIGH != 1
  #error LOW is not 0 or HIGH is not 1
#endif

namespace ace_segment {

class Hardware;

/**
 * Class that represents the abstraction of a particular LED display wiring.
 * If the resistors are on the segments, then the segments become the
 * Elements and the digits become the Groups.
 * If the resistors are on the digits, then the digits become the Elements
 * and the segments become the Groups.
 * The setCathodeOnGroup() and setAnnodeOnGroup() methods determine the
 * polarity of the LED with respect to the Group line.
 * The invertGroupLevels() should be called when transistors are used
 * on the Group lines to handle higher currents.
 */
class LedMatrix {
  public:
    LedMatrix(Hardware* hardware, uint8_t numGroups, uint8_t numElements):
        mHardware(hardware),
        mNumGroups(numGroups),
        mNumElements(numElements)
    {}

    virtual ~LedMatrix() {}

    /** LED negative terminals are on the group line. Required. */
    void setCathodeOnGroup() {
      mGroupOn = LOW;
      mGroupOff = HIGH;
      mElementOn = HIGH;
      mElementOff = LOW;
    }

    /** LED positive terminals are on the group line. Required. */
    void setAnodeOnGroup() {
      mGroupOn = HIGH;
      mGroupOff = LOW;
      mElementOn = LOW;
      mElementOff = HIGH;
    }

    /** If a transistor drives the group, invert the logic levels. */
    void invertGroupLevels() {
      mGroupOn = 1 - mGroupOn;
      mGroupOff =  1 - mGroupOff;
    }

    /** Configure the pins for the given LED wiring. */
    virtual void configure() {}

    /** Turn off the pins by doing the opposite of configure(). */
    virtual void finish() {}

    virtual void enableGroup(uint8_t group) = 0;

    virtual void disableGroup(uint8_t group) = 0;

    virtual void drawElements(uint8_t pattern) = 0;

  protected:
    Hardware* const mHardware;
    const uint8_t mNumGroups;
    const uint8_t mNumElements;

    uint8_t mGroupOn;
    uint8_t mGroupOff;
    uint8_t mElementOn;
    uint8_t mElementOff;
};

}

#endif
