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

#include "LedMatrixSplit.h"

namespace ace_segment {

class Hardware;

/**
 * An implementation of LedMatrixSplit with an 74HC595 Serial-To-Parallel
 * converter chip on the segment pins, with the digit pins directly connected to
 * the microcontroller.
 *
 * The wiring is as follows:
 *
 *   latchPin/D10/SS -- ST_CP (Phillips) / RCK (TI) / Pin 12 (rising)
 *   dataPin/D11/MOSI -- DS (Phillips) / SER (TI) / Pin 14
 *   clockPin/D13/SCK -- SH_CP (Phillips) / SRCK (TI) / Pin 11 (rising)
 *
 * @tparam H class that provides access to hardware pins and timing functions
 * @tparam SA class providing SPI, either SwSpiAdapter or HwSpiAdapter
 */
template<typename H, typename SA>
class LedMatrixPartialSpi : public LedMatrixSplit<H> {
  public:
    LedMatrixPartialSpi(
        const H& hardware,
        const SA& spiAdapter,
        uint8_t groupOnPattern,
        uint8_t elementOnPattern,
        uint8_t numGroups,
        const uint8_t* groupPins
    ) :
        LedMatrixSplit<H>(
            hardware,
            groupOnPattern,
            elementOnPattern,
            numGroups,
            groupPins),
        mSpiAdapter(spiAdapter)
    {}

    void begin() override {
      LedMatrixSplit<H>::begin();

      mSpiAdapter.spiBegin();
    }

    void end() override {
      LedMatrixSplit<H>::end();

      mSpiAdapter.spiEnd();
    }

  protected:
    void drawElements(uint8_t pattern) override {
      uint8_t actualPattern = pattern ^ LedMatrixSplit<H>::mElementXorMask;
      mSpiAdapter.spiTransfer(actualPattern);
    }

  private:
    const SA& mSpiAdapter;
};

}
#endif
