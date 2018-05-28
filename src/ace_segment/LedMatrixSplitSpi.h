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

#ifndef ACE_SEGMENT_LED_MATRIX_SPI_H
#define ACE_SEGMENT_LED_MATRIX_SPI_H

#include "LedMatrixSplitSerial.h"

namespace ace_segment {

class Hardware;

/**
 * Similar to LedMatrixSplitSerial but uses SPI to talk to the 74HC595 chip
 * instead of the shiftOut() method.
 */
class LedMatrixSplitSpi: public LedMatrixSplitSerial {
  public:
    LedMatrixSplitSpi(Hardware* hardware, bool cathodeOnGroup,
        bool transistorsOnGroups, bool transistorsOnElements,
        uint8_t numGroups, uint8_t numElements, const uint8_t* groupPins,
        uint8_t latchPin, uint8_t dataPin, uint8_t clockPin):
        LedMatrixSplitSerial(hardware, cathodeOnGroup, transistorsOnGroups,
            transistorsOnElements, numGroups, numElements,
            groupPins, latchPin, dataPin, clockPin)
    {}

    virtual void configure() override;

    virtual void finish() override;

    virtual void drawElements(uint8_t pattern) override;
};

}

#endif
