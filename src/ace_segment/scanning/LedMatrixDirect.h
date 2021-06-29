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

#ifndef ACE_SEGMENT_LED_MATRIX_DIRECT_H
#define ACE_SEGMENT_LED_MATRIX_DIRECT_H

#include <Arduino.h> // OUTPUT, INPUT
#include "../hw/GpioInterface.h"
#include "LedMatrixBase.h"

class LedMatrixDirectTest_drawElements;

namespace ace_segment {

/**
 * An LedMatrixBase that whose group pins and element pins are wired directly to
 * the MCU.
 *
 * @tparam T_GPIOI (optional) class that provides access to the GPIO pins,
 *    default is GpioInterface (note: 'GPI' is already taken on ESP8266)
 */
template <typename T_GPIOI = GpioInterface>
class LedMatrixDirect : public LedMatrixBase {
  public:
    /**
     * Constructor.
     * @param elementOnPattern bit pattern that turns on the elements (segments)
     * @param groupOnpattern bit pattern that turns on the groups (digits)
     * @param numElements number of LED segments, almost always 8
     * @param elementPins pointer to array of 'numElements' pin numbers
     * @param numGroups number of LED groups (digits)
     * @param groupPins pointer to array of 'numGroups' pin numbers
     */
    LedMatrixDirect(
        uint8_t elementOnPattern,
        uint8_t groupOnPattern,
        uint8_t numElements,
        const uint8_t* elementPins,
        uint8_t numGroups,
        const uint8_t* groupPins
    ) :
        LedMatrixBase(elementOnPattern, groupOnPattern),
        mElementPins(elementPins),
        mGroupPins(groupPins),
        mNumElements(numElements),
        mNumGroups(numGroups)
    {}

    void begin() const {
      // Set element pins to OUTPUT mode but set LEDs to OFF.
      uint8_t output = (0x00 ^ mElementXorMask) & 0x1;
      for (uint8_t element = 0; element < mNumElements; element++) {
        uint8_t elementPin = mElementPins[element];
        T_GPIOI::pinMode(elementPin, OUTPUT);
        T_GPIOI::digitalWrite(elementPin, output);
      }

      // Set group pins to OUTPUT mode but set LEDs to OFF.
      output = (0x00 ^ mGroupXorMask) & 0x1;
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        T_GPIOI::pinMode(pin, OUTPUT);
        T_GPIOI::digitalWrite(pin, output);
      }
    }

    void end() const {
      // Set element pins to INPUT mode.
      for (uint8_t element = 0; element < mNumElements; element++) {
        uint8_t elementPin = mElementPins[element];
        T_GPIOI::pinMode(elementPin, INPUT);
      }

      // Set group pins to INPUT mode.
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        T_GPIOI::pinMode(pin, INPUT);
      }
    }

    void draw(uint8_t group, uint8_t elementPattern) const {
      if (group != mPrevGroup) {
        disableGroup(mPrevGroup);
      }

      drawElements(elementPattern);
      enableGroup(group);
      mPrevGroup = group;
    }

    void enableGroup(uint8_t group) const {
      writeGroupPin(group, 0x1);
      mPrevGroup = group;
    }

    void disableGroup(uint8_t group) const {
      writeGroupPin(group, 0x0);
      mPrevGroup = group;
    }

    void clear() const {
      for (uint8_t group = 0; group < mNumGroups; group++) {
        disableGroup(group);
      }
      drawElements(0);
    }

  private:
    friend class ::LedMatrixDirectTest_drawElements;

    /** Send the pattern to the element pins. */
    void drawElements(uint8_t pattern) const {
      for (uint8_t element = 0; element < mNumElements; element++) {
        writeElementPin(element, pattern);
        pattern >>= 1;
      }
    }

    /** Write bit 0 of output to the element pin. */
    void writeElementPin(uint8_t element, uint8_t output) const {
      uint8_t elementPin = mElementPins[element];
      T_GPIOI::digitalWrite(elementPin, (output ^ mElementXorMask) & 0x1);
    }

    /** Write bit 0 of output to group pin. */
    void writeGroupPin(uint8_t group, uint8_t output) const {
      uint8_t groupPin = mGroupPins[group];
      T_GPIOI::digitalWrite(groupPin, (output ^ mGroupXorMask) & 0x1);
    }

  private:
    const uint8_t* const mElementPins;
    const uint8_t* const mGroupPins;
    uint8_t const mNumElements;
    uint8_t const mNumGroups;

    /** Store the previous group, to turn it off after moving to new group. */
    mutable uint8_t mPrevGroup = 0;
};

} // ace_segment

#endif
