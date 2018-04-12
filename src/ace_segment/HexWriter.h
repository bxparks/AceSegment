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

#ifndef ACE_SEGMENT_HEX_WRITER_H
#define ACE_SEGMENT_HEX_WRITER_H

#include <stdint.h>
#include "StyledDigit.h"
#include "Renderer.h"

namespace ace_segment {

/**
 * The HexWriter supports mapping of Hex characters to segment patterns
 * supported by Renderer. A few other characters are supported which
 * should be self-explanatory: kSpace, kPeriod, kMinus.
 */
class HexWriter {
  public:
    static const uint8_t kNumCharacters;
    static const uint8_t kSpace = 0x10;
    static const uint8_t kPeriod = 0x11;
    static const uint8_t kMinus = 0x12;

    /** Constructor. */
    explicit HexWriter(Renderer* renderer):
        mRenderer(renderer)
    {}

    /** Get the number of digits. */
    uint8_t getNumDigits() { return mRenderer->getNumDigits(); }

    /** Write the hex at the specified position. */
    void writeHexAt(uint8_t digit, uint8_t c);

    /** Write the hex at the specified position. */
    void writeHexAt(uint8_t digit, uint8_t c, StyledDigit::StyleType style);

    /** Write the style for a given digit, leaving hex unchanged. */
    void writeStyleAt(uint8_t digit, StyledDigit::StyleType style) {
      if (digit >= getNumDigits()) return;
      mRenderer->writeStyleAt(digit, style);
    }

    /** Write the decimal point at digit. */
    void writeDecimalPointAt(uint8_t digit, bool state = true) {
      if (digit >= getNumDigits()) return;
      mRenderer->writeDecimalPointAt(digit, state);
    }

  private:
    // Bit pattern map for hex characters.
    static const uint8_t kCharacterArray[];

    // disable copy-constructor and assignment operator
    HexWriter(const HexWriter&) = delete;
    HexWriter& operator=(const HexWriter&) = delete;

    Renderer* const mRenderer;
};

}

#endif
