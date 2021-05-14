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

#ifndef ACE_SEGMENT_CLOCK_WRITER_H
#define ACE_SEGMENT_CLOCK_WRITER_H

#include <stdint.h>
#include "../LedDisplay.h"
#include "NumberWriter.h"

namespace ace_segment {

/**
 * The ClockWriter writes "hh:mm" and "yyyy" to the LedDisplay. A few other
 * characters are supported.
 */
class ClockWriter {
  public:
    using hexchar_t = NumberWriter::hexchar_t;

    /** Blank digit. */
    static const hexchar_t kCharSpace = NumberWriter::kCharSpace;

    /** A minus ("-") sign. */
    static const hexchar_t kCharMinus = NumberWriter::kCharMinus;

    /** The "A" character for "AM". */
    static const uint8_t kPatternA = 0b01110111;

    /** The "P" character for "PM". */
    static const uint8_t kPatternP = 0b01110011;

    /**
     * Constructor.
     *
     * @param ledDisplay instance of LedDisplay
     * @param colonDigit The digit which has the colon (":") character,
     *    mapped to bit 7 (i.e. 'H' segment). In many 4-digit LED clock
     *    display modules, this is digit 1 (counting from the left, 0-based,
     *    so the second digit from the left).
     */
    explicit ClockWriter(
        LedDisplay& ledDisplay,
        uint8_t colonDigit = 1
    ) :
        mNumberWriter(ledDisplay),
        mColonDigit(colonDigit)
    {}

    /** Get the underlying LedDisplay. */
    LedDisplay& display() const {
      return mNumberWriter.display();
    }

    /** Write the hexchar_t 'c' at 'pos'. */
    void writeCharAt(uint8_t pos, hexchar_t c) {
      mNumberWriter.writeHexCharAt(pos, c);
    }

    /**
     * Write the 2 hexchar_t 'c0' and 'c1' at 'pos' and 'pos+1'.
     * This is a convenience method because the need to write 2 digits (or 2
     * spaces) occurs quite frequently when implementing clocks.
     */
    void writeChars2At(uint8_t pos, hexchar_t c0, hexchar_t c1) {
      writeCharAt(pos++, c0);
      writeCharAt(pos++, c1);
    }

    /**
     * Write a 2-digit BCD number at position, which involves just printing the
     * number as a hexadecimal number. For example, 0x12 is printed as "12", but
     * 0x1A is printed as "1 ".
     */
    void writeBcd2At(uint8_t pos, uint8_t bcd) {
      uint8_t high = (bcd & 0xF0) >> 4;
      uint8_t low = (bcd & 0x0F);
      if (high > 9) high = kCharSpace;
      if (low > 9) low = kCharSpace;
      writeChars2At(pos, high, low);
    }

    /**
     * Write a 2-digit decimal number at position digit, right justified. If the
     * number is greater than 100, then print "  " (2 spaces). Useful for day,
     * hour, minute, seconds.
     */
    void writeDec2At(uint8_t pos, uint8_t d) {
      if (d >= 100) {
        writeChars2At(pos++, kCharSpace, kCharSpace);
      } else {
        uint8_t bcd = ace_common::decToBcd(d);
        writeBcd2At(pos, bcd);
      }
    }

    /**
     * Write the 4 digit decimal number at pos, right justified, padded with a
     * '0' character. Useful for year.
     */
    void writeDec4At(uint8_t pos, uint16_t dd) {
      uint8_t high = dd / 100;
      uint8_t low = dd - high * 100;
      writeDec2At(pos, high);
      writeDec2At(pos + 2, low);
    }

    /**
     * Write the hour and minutes, and the colon in one-shot, assuming the LED
     * module is a 4-digit clock module. This is a convenience function.
     */
    void writeHourMinute(uint8_t hh, uint8_t mm) {
      writeDec2At(0, hh);
      writeDec2At(2, mm);
      writeColon();
    }

    /**
     * Write the colon symbol between 'hh' and 'mm'.
     *
     * @param state Set to false to turn off the colon.
     */
    void writeColon(bool state = true) {
      display().writeDecimalPointAt(mColonDigit, state);
    }

  private:
    // disable copy-constructor and assignment operator
    ClockWriter(const ClockWriter&) = delete;
    ClockWriter& operator=(const ClockWriter&) = delete;

    NumberWriter mNumberWriter;
    uint8_t const mColonDigit;
};

} // ace_segment

#endif
