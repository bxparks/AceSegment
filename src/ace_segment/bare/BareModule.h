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

#ifndef ACE_SEGMENT_BARE_MODULE_H
#define ACE_SEGMENT_BARE_MODULE_H

#include "../scanning/ScanningModule.h"
#include "../scanning/LedMatrixDirect.h"

namespace ace_segment {

/**
 * An implementation of LedModule class that supports an LED module whose
 * segment and digit pins are directly connected to the GPIO pins of the
 * microcontroller. This is a convenience class that pairs together a
 * ScanningModule and a LedMatrixDirect in a single class. For ease of use, this
 * class assumes that the number of segments is always 8.
 *
 * @tparam T_DIGITS number of digits in the LED module
 */
template <
    uint8_t T_DIGITS,
    uint8_t T_SUBFIELDS = 1,
    typename T_CI = ClockInterface,
    typename T_GPIOI = GpioInterface
>
class BareModule : public ScanningModule<
    LedMatrixDirect<T_GPIOI>,
    T_DIGITS,
    T_SUBFIELDS,
    T_CI
> {
  private:
    using Super = ScanningModule<
        LedMatrixDirect<T_GPIOI>,
        T_DIGITS,
        T_SUBFIELDS,
        T_CI
    >;

  public:
    BareModule(
        uint8_t segmentOnPattern,
        uint8_t digitOnPattern,
        uint8_t framesPerSecond,
        const uint8_t* segmentPins,
        const uint8_t* digitPins
    ) :
        Super(mLedMatrix, framesPerSecond),
        mLedMatrix(
            segmentOnPattern /*elementOnPattern*/,
            digitOnPattern /*groupOnPattern*/,
            8 /* numElements */,
            segmentPins /*elementPins*/,
            T_DIGITS /*numGroups*/,
            digitPins /* groupPins */
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
    LedMatrixDirect<T_GPIOI> mLedMatrix;
};

} // ace_segment

#endif

