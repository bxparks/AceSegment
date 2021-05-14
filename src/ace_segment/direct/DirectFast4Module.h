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

#ifndef ACE_SEGMENT_DIRECT_FAST_4_MODULE_H
#define ACE_SEGMENT_DIRECT_FAST_4_MODULE_H

#include <stdint.h>
#include "../scanning/ScanningModule.h"
#include "../scanning/LedMatrixDirectFast4.h"

namespace ace_segment {

/**
 * An implementation of LedModule whose segment and digit pins are directly
 * connected to the GPIO pins of the microcontroller. This is a convenience
 * class that pairs together a ScanningModule and a LedMatrixDirectFast4 in a
 * single class. For ease of use, this class assumes that the number of segments
 * is always 8 and the number of digits is always 4.
 *
 * @tparam eX element (segment) pin numbers
 * @tparam gX group (digit) pin numbers
 * @tparam T_DIGITS number of digits in the LED module
 * @tparam T_SUBFIELDS number of subfields for each digit to get brightness
 *    control using PWM. The default is 1, but can be set to greater than 1 to
 *    get brightness control.
 * @tparam T_CI class that provides access to Arduino clock functions (millis()
 *    and micros()). The default is ClockInterface.
 * @tparam T_GPIOI (optional) class that provides access to the GPIO pins,
 *    default is GpioInterface (note: 'GPI' is already taken on ESP8266)
 */
template <
    uint8_t e0, uint8_t e1, uint8_t e2, uint8_t e3,
    uint8_t e4, uint8_t e5, uint8_t e6, uint8_t e7,
    uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3,
    uint8_t T_DIGITS,
    uint8_t T_SUBFIELDS = 1,
    typename T_CI = ClockInterface
>
class DirectFast4Module : public ScanningModule<
    LedMatrixDirectFast4<e0, e1, e2, e3, e4, e5, e6, e7, g0, g1, g2, g3>,
    T_DIGITS,
    T_SUBFIELDS,
    T_CI
> {
  private:
    using Super = ScanningModule<
        LedMatrixDirectFast4<e0, e1, e2, e3, e4, e5, e6, e7, g0, g1, g2, g3>,
        T_DIGITS,
        T_SUBFIELDS,
        T_CI
    >;

  public:
    DirectFast4Module(
        uint8_t segmentOnPattern,
        uint8_t digitOnPattern,
        uint8_t framesPerSecond
    ) :
        Super(mLedMatrix, framesPerSecond),
        mLedMatrix(
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
    LedMatrixDirectFast4<e0, e1, e2, e3, e4, e5, e6, e7, g0, g1, g2, g3>
        mLedMatrix;
};

} // ace_segment

#endif
