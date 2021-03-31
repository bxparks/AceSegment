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

#ifndef ACE_SEGMENT_LED_MATRIX_FULL_SPI_H
#define ACE_SEGMENT_LED_MATRIX_FULL_SPI_H

#include "LedMatrix.h"

namespace ace_segment {

/**
 * An LedMatrix that writes to both group and element pins via SPI. The group
 * pins are assumed to be connected to the most significant byte. The element
 * pins are connected to the least signficiant byte.
 *
 * @tparam SA class providing SPI, either SwSpiAdapter or HwSpiAdapter
 */
template<typename SA>
class LedMatrixFullSpi: public LedMatrix {
  public:
    LedMatrixFullSpi(
        const SA& spiAdapter,
        uint8_t groupOnPattern,
        uint8_t elementOnPattern
    ) :
        LedMatrix(groupOnPattern, elementOnPattern),
        mSpiAdapter(spiAdapter)
    {}

    void begin() override {
      mSpiAdapter.spiBegin();
    }

    void end() override {
      mSpiAdapter.spiEnd();
    }

    /**
     * Write out the group and element patterns in a single 16-bit stream
     * with the group bits in the MSB and the element bits in the LSB.
     */
    void draw(uint8_t group, uint8_t elementPattern) override {
      uint8_t groupPattern = 0x1 << group; // Would a lookup table be faster?

      uint8_t actualGroupPattern = (groupPattern ^ mGroupXorMask);
      uint8_t actualElementPattern = (elementPattern ^ mElementXorMask);

      mSpiAdapter.spiTransfer16(
          actualGroupPattern << 8 | actualElementPattern);
      mPrevElementPattern = elementPattern;
    }

    void enableGroup(uint8_t group) override {
      draw(group, mPrevElementPattern);
    }

    void disableGroup(uint8_t group) override {
      (void) group;
      clear();
    }

    void clear() override {
      uint8_t actualGroupPattern = 0x00 ^ mGroupXorMask;
      uint8_t actualElementPattern = 0x00 ^ mElementXorMask;
      mSpiAdapter.spiTransfer16(
          actualGroupPattern << 8 | actualElementPattern);
    }

  private:
    /** Not used in this class. */
    void drawElements(uint8_t /*pattern*/) override {}

  private:
    const SA& mSpiAdapter;

    /**
     * Remember the previous element pattern to support disableGroup() and
     * enableGroup().
     */
    uint8_t mPrevElementPattern;
};

}

#endif
