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

#ifndef ACE_SEGMENT_SINGLE_HC595_MODULE_H
#define ACE_SEGMENT_SINGLE_HC595_MODULE_H

#include "../scanning/ScanningModule.h"
#include "../scanning/LedMatrixSingleShiftRegister.h"

namespace ace_segment {

/**
 * An implementation of LedModule class that supports an LED module using a
 * single 74HC595 Shift Register chip on the segment pins. This is a convenience
 * class that pairs together a ScanningModule and a LedMatrixSingleShiftRegister
 * in a single class.
 */
template <
    typename T_SPII,
    uint8_t T_DIGITS,
    uint8_t T_SUBFIELDS = 1,
    typename T_CI = ClockInterface,
    typename T_GPIOI = GpioInterface
>
class SingleHc595Module : public ScanningModule<
    LedMatrixSingleShiftRegister<T_SPII, T_GPIOI>,
    T_DIGITS,
    T_SUBFIELDS,
    T_CI
> {
  public:
    SingleHc595Module(
        const T_SPII& spiInterface,
        uint8_t elementOnPattern,
        uint8_t groupOnPattern,
        uint8_t framesPerSecond,
        const uint8_t* groupPins
    ) :
        ScanningModule<
            LedMatrixSingleShiftRegister<T_SPII>,
            T_DIGITS,
            T_SUBFIELDS,
            T_CI
        >(mLedMatrix, framesPerSecond),
        mLedMatrix(
            spiInterface,
            elementOnPattern,
            groupOnPattern,
            T_DIGITS,
            groupPins
        )
    {}

  private:
    LedMatrixSingleShiftRegister<T_SPII, T_GPIOI> mLedMatrix;
};

} // ace_segment

#endif
