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
    DualHc595Module(
        const T_SPII& spiInterface,
        uint8_t segmentOnPattern,
        uint8_t digitOnPattern,
        uint8_t framesPerSecond
    ) :
        Super(mLedMatrix, framesPerSecond),
        mLedMatrix(
            spiInterface,
            segmentOnPattern /*elementOnPattern*/,
            digitOnPattern /*groupOnPattern*/
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
