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

#ifndef ACE_SEGMENT_CLOCK_WRITER_H
#define ACE_SEGMENT_CLOCK_WRITER_H

#include <stdint.h>
#include "StyledPattern.h"
#include "Renderer.h"

namespace ace_segment {

/**
 * The ClockWriter writes "hh:mm" to the Renderer's segment patterns. A few
 * other characters are supported: kSpace, kMinus, kA ("A" for AM) and kP ("P"
 * for "PM").
 */
class ClockWriter {
  public:
    /** Total number of characters in the character set. */
    static const uint8_t kNumCharacters;

    /** Blank digit. */
    static const uint8_t kSpace = 10;

    /** A minus ("-") sign. */
    static const uint8_t kMinus = 11;

    /** The "P" character for "PM". */
    static const uint8_t kA = 12;

    /** The "A" character for "AM". */
    static const uint8_t kP = 13;

    /** The superscript degrees symbol for temperature degrees. */
    static const uint8_t kDegrees = 14;

    /**
     * Constructor.
     *
     * @param colonDigit The digit which has the colon (":") character,
     * mapped to bit 7 (i.e. 'H' segment). In a stadard 4 digit clock display,
     * this is digit 1 (counting from the left, 0-based).
     */
    explicit ClockWriter(Renderer* renderer, uint8_t colonDigit = 1):
        mRenderer(renderer),
        mColonDigit(colonDigit)
    {}

    /** Get the number of digits. */
    uint8_t getNumDigits() { return mRenderer->getNumDigits(); }

    /** Write "hh:mm". */
    void writeClock(uint8_t hh, uint8_t mm);

    /** Write "hh:mm", with hh and mm in Binary Coded Decimal (BCD) format. */
    void writeBcdClock(uint8_t hhBcd, uint8_t mmBcd);

    /** Write the character at the specified position. */
    void writeCharAt(uint8_t digit, uint8_t c);

    /** Write the character at the specified position. */
    void writeCharAt(uint8_t digit, uint8_t c, uint8_t style);

    /** Write the style for a given digit, leaving character unchanged. */
    void writeStyleAt(uint8_t digit, uint8_t style) {
      if (digit >= getNumDigits()) return;
      mRenderer->writeStyleAt(digit, style);
    }

    /**
     * Write the colon symbol between 'hh' and 'mm'.
     *
     * @param state Set to false to turn off the colon.
     */
    void writeColon(bool state = true) {
      mRenderer->writeDecimalPointAt(mColonDigit, state);
    }

    /**
     * Convert decimal number into BCD (binary coded decimal).
     * VisibleForTesting.
     */
    static uint8_t toBcd(uint8_t d);

  private:
    // Bit pattern map for hex characters.
    static const uint8_t kCharacterArray[];

    // disable copy-constructor and assignment operator
    ClockWriter(const ClockWriter&) = delete;
    ClockWriter& operator=(const ClockWriter&) = delete;

    Renderer* const mRenderer;
    const uint8_t mColonDigit;
};

}

#endif