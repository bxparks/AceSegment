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

#ifndef ACE_SEGMENT_DUAL_HC595_MODULE_H
#define ACE_SEGMENT_DUAL_HC595_MODULE_H

#include "../scanning/ScanningModule.h"
#include "../scanning/LedMatrixDualHc595.h"

namespace ace_segment {

/** Send the digit bits first, then the segment patterns. */
const uint8_t kByteOrderDigitHighSegmentLow = kByteOrderGroupHighElementLow;

/** Send the segment patterns first, then the digit bits. */
const uint8_t kByteOrderSegmentHighDigitLow = kByteOrderElementHighGroupLow;

/**
 * The 8-digit LED module from diymore.cc using dual 74HC595 controller chips
 * are wired such that the left 4-digits and right 4-digits are flipped.
 * This remap array fixes that.
 *
 * You can create your own remap array to handle other LED modules with
 * different physical ordering compared to the logical ordering.
 */
extern const uint8_t kDigitRemapArray8Hc595[8];

/**
 * An implementation of LedModule class that supports an LED module using 2
 * 74HC595 Shift Register chips. This is a convenience class that pairs together
 * a ScanningModule and a LedMatrixDualHc595 in a single class.
 *
 * @tparam T_SPII interface to SPI, either SoftSpiInterface or HardSpiInterface
 * @tparam T_DIGITS number of LED digits
 * @tparam T_SUBFIELDS number of subfields for each digit to get brightness
 *    control using PWM. The default is 1, but can be set to greater than 1 to
 *    get brightness control.
 * @tparam T_CI class that provides access to Arduino clock functions (millis()
 *    and micros()). The default is ClockInterface.
 */
template <
    typename T_SPII,
    uint8_t T_DIGITS,
    uint8_t T_SUBFIELDS = 1,
    typename T_CI = ClockInterface
>
class DualHc595Module : public ScanningModule<
    LedMatrixDualHc595<T_SPII>,
    T_DIGITS,
    T_SUBFIELDS,
    T_CI
> {
  private:
    using Super = ScanningModule<
        LedMatrixDualHc595<T_SPII>,
        T_DIGITS,
        T_SUBFIELDS,
        T_CI
    >;

  public:
    /**
     * @param spiInterface object that knows how to send SPI packets
     * @param segmentOnPattern the bit pattern that indicates whether the
     *    segment pins are wired to be active high (LedMatrixBase::kActiveHigh)
     *    or active low (LedMatrixBase::kActiveLow)
     * @param digitOnPattern the bit pattern that indicates whether the digit
     *    pins are wired to be active high (LedMatrixBase::kActiveHigh)
     *    or active low (LedMatrixBase::kActiveLow)
     * @param framesPerSecond desired number of frames per second (usually
     *    greater than or equal to 60 to avoid flickering)
     * @param byteOrder whether to send the digit patterns first
     *    (kByteOrderDigitHighSegmentLow) or segment patterns first
     *    (kByteOrderSegmentHighDigitLow)
     * @param remapArray (optional) some LED modules using the 74HC595 chip need
     *    their physical digit positions remapped to their logical positions
     *    (e.g. the 8-digit LED modules from diymore.cc have the left 4 and
     *    right 4 LED digits swapped)
     */
    DualHc595Module(
        const T_SPII& spiInterface,
        uint8_t segmentOnPattern,
        uint8_t digitOnPattern,
        uint8_t framesPerSecond,
        uint8_t byteOrder,
        const uint8_t* remapArray = nullptr
    ) :
        Super(mLedMatrix, framesPerSecond),
        mLedMatrix(
            spiInterface,
            segmentOnPattern /*elementOnPattern*/,
            digitOnPattern /*groupOnPattern*/,
            byteOrder,
            remapArray
        )
    {}

    void begin() {
      mLedMatrix.begin();
      Super::begin();
    }

    void end() {
      mLedMatrix.end();
      Super::end();
    }

  private:
    LedMatrixDualHc595<T_SPII> mLedMatrix;
};

} // ace_segment

#endif
