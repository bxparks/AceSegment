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

#include "LedMatrixMergedSerial.h"

namespace ace_segment {

class Hardware;

/**
 * An LedMatrix with an 74HC595 Serial-To-Parallel converter chip on the
 * both the digit and segment pins. Uses hardware SPI. The wiring is as follows:
 *
 *   latchPin/D10/SS -- ST_CP (Phillips) / RCK (TI) / Pin 12 (rising)
 *   dataPin/D11/MOSI -- DS (Phillips) / SER (TI) / Pin 14
 *   clockPin/D13/SCK -- SH_CP (Phillips) / SRCK (TI) / Pin 11 (rising)
 */
class LedMatrixMergedSpi: public LedMatrixMergedSerial {
  public:
    LedMatrixMergedSpi(Hardware* hardware, bool cathodeOnGroup,
            bool transistorsOnGroups, bool transistorsOnElements,
            uint8_t numGroups, uint8_t numElements,
            uint8_t latchPin, uint8_t dataPin, uint8_t clockPin):
        LedMatrixMergedSerial(hardware, cathodeOnGroup, transistorsOnGroups,
            transistorsOnElements, numGroups, numElements, latchPin,
            dataPin, clockPin)
    {}

    virtual void configure() override;

    virtual void finish() override;

    virtual void draw(uint8_t groupPattern, uint8_t elementPattern) override;
};

}

#endif
