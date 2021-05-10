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

#ifndef ACE_SEGMENT_LED_MATRIX_DUAL_HC595_H
#define ACE_SEGMENT_LED_MATRIX_DUAL_HC595_H

#include "LedMatrixBase.h"

class LedMatrixDualHc595Test_draw;
class LedMatrixDualHc595Test_enableGroup;
class LedMatrixDualHc595Test_disableGroup;

namespace ace_segment {

/** Send the group bits first, then the element patterns. */
const uint8_t kByteOrderGroupHighElementLow = 0;

/** Send the element patterns first, then the group bits. */
const uint8_t kByteOrderElementHighGroupLow = 1;

/**
 * An LedMatrix that whose group pins are attached to one 74HC595 shift register
 * and the element pins are attached to another 74HC595 shift register. The 2
 * shift registers are daisy chained so that they can be accessed in a serial
 * transfer of 16-bits using hardware or software SPI.
 *
 * The group pins are assumed to be connected to the most significant byte. The
 * element pins are connected to the least signficiant byte.
 *
 * @tparam T_SPII interface to SPI, either SoftSpiInterface or HardSpiInterface
 */
template <typename T_SPII>
class LedMatrixDualHc595: public LedMatrixBase {
  public:
    LedMatrixDualHc595(
        const T_SPII& spiInterface,
        uint8_t elementOnPattern,
        uint8_t groupOnPattern,
        uint8_t byteOrder
    ) :
        LedMatrixBase(elementOnPattern, groupOnPattern),
        mSpiInterface(spiInterface),
        mByteOrder(byteOrder)
    {}

    void begin() const {}

    void end() const {}

    /**
     * Write out the group and element patterns in a single 16-bit stream
     * with the group bits in the MSB and the element bits in the LSB.
     */
    void draw(uint8_t group, uint8_t elementPattern) const {
      uint8_t groupPattern = 0x1 << group; // Would a lookup table be faster?
      drawPatterns(groupPattern, elementPattern);
      mPrevElementPattern = elementPattern;
    }

    /**
     * Turn on the given group, using the previous segment pattern. Useful for
     * blinking a group (e.g. a digit of an LED segment module).
     */
    void enableGroup(uint8_t group) const {
      draw(group, mPrevElementPattern);
    }

    /** Turn off the given group. Useful for blinking a group. */
    void disableGroup(uint8_t group) const {
      (void) group;
      drawPatterns(0x00, 0x00);
      // Don't update mPrevElementPattern.
    }

    /** Clear the entire display. */
    void clear() const {
      drawPatterns(0x00, 0x00);
      mPrevElementPattern = 0x00;
    }

  private:

    /**
     * Send the groupPattern and elementPattern to the display  through SPI. The
     * patterns are inverted if necessary due to wiring requirements (e.g.
     * common cathode versus common anode, or if a driver transitor inverts the
     * logic levels). The byte-order is determined by the mByteOrder setting.
     */
    void drawPatterns(uint8_t groupPattern, uint8_t elementPattern) const {
      uint8_t actualGroupPattern = (groupPattern ^ mGroupXorMask);
      uint8_t actualElementPattern = (elementPattern ^ mElementXorMask);
      uint16_t data = (mByteOrder == kByteOrderGroupHighElementLow)
          ? actualGroupPattern << 8 | actualElementPattern
          : actualElementPattern << 8 | actualGroupPattern;
      mSpiInterface.send16(data);
    }

  private:
    friend class ::LedMatrixDualHc595Test_draw;
    friend class ::LedMatrixDualHc595Test_enableGroup;
    friend class ::LedMatrixDualHc595Test_disableGroup;

    const T_SPII& mSpiInterface;
    const uint8_t mByteOrder;

    /**
     * Remember the previous element pattern to support disableGroup() and
     * enableGroup().
     */
    mutable uint8_t mPrevElementPattern;
};

}

#endif
