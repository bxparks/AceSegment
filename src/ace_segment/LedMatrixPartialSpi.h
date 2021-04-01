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

#ifndef ACE_SEGMENT_LED_MATRIX_PARTIAL_SPI_H
#define ACE_SEGMENT_LED_MATRIX_PARTIAL_SPI_H

#include <Arduino.h> // OUTPUT, INPUT
#include "LedMatrixBase.h"

class LedMatrixPartialSpiTest_drawElements;

namespace ace_segment {

/**
 * An implementation of LedMatrixBase with an 74HC595 Serial-To-Parallel
 * converter chip on the segment pins, with the digit pins directly connected to
 * the microcontroller.
 *
 * The wiring is as follows:
 *
 *   latchPin/D10/SS -- ST_CP (Phillips) / RCK (TI) / Pin 12 (rising)
 *   dataPin/D11/MOSI -- DS (Phillips) / SER (TI) / Pin 14
 *   clockPin/D13/SCK -- SH_CP (Phillips) / SRCK (TI) / Pin 11 (rising)
 *
 * @tparam H class that provides access to hardware pins functions
 * @tparam SA class that provides SPI, either SwSpiAdapter or HwSpiAdapter
 */
template<typename H, typename SA>
class LedMatrixPartialSpi : public LedMatrixBase {
  public:
    LedMatrixPartialSpi(
        const H& hardware,
        const SA& spiAdapter,
        uint8_t groupOnPattern,
        uint8_t elementOnPattern,
        uint8_t numGroups,
        const uint8_t* groupPins
    ) :
        LedMatrixBase(groupOnPattern, elementOnPattern),
        mHardware(hardware),
        mSpiAdapter(spiAdapter),
        mGroupPins(groupPins),
        mNumGroups(numGroups)
    {}

    void begin() const {
      // Set pins to OUTPUT mode but set LEDs to OFF.
      uint8_t output = (0x00 ^ mGroupXorMask) & 0x1;
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware.pinMode(pin, OUTPUT);
        mHardware.digitalWrite(pin, output);
      }
    }

    void end() const {
      // Set pins to INPUT mode.
      for (uint8_t group = 0; group < mNumGroups; group++) {
        uint8_t pin = mGroupPins[group];
        mHardware.pinMode(pin, INPUT);
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
      drawElements(0x00);
    }

  private:
    friend class ::LedMatrixPartialSpiTest_drawElements;

    /** Send the pattern to the element pins. */
    void drawElements(uint8_t pattern) const {
      uint8_t actualPattern = pattern ^ mElementXorMask;
      mSpiAdapter.transfer(actualPattern);
    }

    /** Write bit 0 of output to group pin. */
    void writeGroupPin(uint8_t group, uint8_t output) const {
      uint8_t groupPin = mGroupPins[group];
      mHardware.digitalWrite(groupPin, (output ^ mGroupXorMask) & 0x1);
    }

  private:
    const H& mHardware;
    const SA& mSpiAdapter;
    const uint8_t* const mGroupPins;
    uint8_t const mNumGroups;

    /** Store the previous group, to turn it off after moving to new group. */
    mutable uint8_t mPrevGroup = 0;
};

}
#endif
