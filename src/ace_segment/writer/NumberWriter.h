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

#ifndef ACE_SEGMENT_NUMBER_WRITER_H
#define ACE_SEGMENT_NUMBER_WRITER_H

#include <stdint.h>
#include "../LedDisplay.h"

namespace ace_segment {

/**
 * The NumberWriter supports converting decimal and hexadecimal numbers to
 * segment patterns expected by LedDisplay. The character set includes 0 to F,
 * and a few other characters which should be self-explanatory: kCharSpace and
 * kCharMinus.
 */
class NumberWriter {
  public:
    /**
     * The type of the character set supported by many methods in this class,
     * usually containing the string "HexChar" in its name. This custom
     * character set is not ASCII to save flash memory. It is a restricted set
     * that starts with 0 and goes up to 0xF to support hexadecimal digits. In
     * addition, the character set adds few more characters for convenience:
     *
     *  * kCharSpace = 0x10 = space character
     *  * kCharMinus = 0x11 = minus character
     *
     * The `hexchar_t` typedef is an alias for `uint8_t` and unfortunately, C++
     * will not prevent mixing a normal `char` or `uint8_t` with a `hexchar_t`.
     * However, it does make the documentation of the various methods more
     * self-explanatory.
     */
    typedef uint8_t hexchar_t;

    /** Total number of characters in the HexCharacter set. */
    static const uint8_t kNumHexChars = 18;

    /** A space character. */
    static const hexchar_t kCharSpace = 0x10;

    /** A minus character. */
    static const hexchar_t kCharMinus = 0x11;

    /** Constructor. */
    explicit NumberWriter(LedDisplay& ledDisplay) :
        mLedDisplay(ledDisplay)
    {}

    /** Get the underlying LedDisplay. */
    LedDisplay& display() const {
      return mLedDisplay;
    }

    /**
     * Write the hex character `c` at position `pos`. If `c` falls outside the
     * valid range of the kHexCharPatterns set, a `kCharSpace` character is
     * printed instead.
     */
    void writeHexCharAt(uint8_t pos, hexchar_t c) {
      writeInternalHexCharAt(
          pos, ((uint8_t) c < kNumHexChars) ? c : kCharSpace);
    }

    /** Write the `len` hex characters given by `s` starting at `pos`. */
    void writeHexCharsAt(uint8_t pos, const hexchar_t s[], uint8_t len) {
      for (uint8_t i = 0; i < len; ++i) {
        writeHexCharAt(pos++, s[i]);
      }
    }

    /** Write the 2-digit (8-bit) hexadecimal byte 'b' at pos. */
    void writeHexByteAt(uint8_t pos, uint8_t b) {
      uint8_t low = (b & 0x0F);
      b >>= 4;
      uint8_t high = (b & 0x0F);

      writeInternalHexCharAt(pos++, high);
      writeInternalHexCharAt(pos++, low);
    }

    /** Write the 4 digit (16-bit) hexadecimal word at pos. */
    void writeHexWordAt(uint8_t pos, uint16_t w) {
      uint8_t low = (w & 0xFF);
      uint8_t high = (w >> 8) & 0xFF;
      writeHexByteAt(pos, high);
      writeHexByteAt(pos + 2, low);
    }

    /**
     * Write the 16-bit unsigned number `num` as a decimal number at pos.
     *
     * @param pos start position of the number
     * @param num unsigned decimal number, 0-65535
     * @param boxSize size of box. This is meant to be similar to the "%-5d" or
     *    "%5d" specifier to the printf() function
     *    * 0 means no boxing, printing from left
     *    * > 0 means right justified inside box
     *    * < 0 means left justified inside box
     *
     * @return number of characters actually written, even if the characters
     *    bled over the end of the LED segments
     */
    uint8_t writeUnsignedDecimalAt(
        uint8_t pos,
        uint16_t num,
        int8_t boxSize = 0);

    /** Same as writeUnsignedDecimalAt() but prepends a '-' sign if negative. */
    uint8_t writeSignedDecimalAt(
        uint8_t pos,
        int16_t num,
        int8_t boxSize = 0);

    /**
     * Write the 2 digit decimal number at pos. This method always writes 2
     * characters. The padding is always a space, so 9 is printed as " 9" not
     * "09". If num >= 100, then this prints "99".
     *
     * This method is meant to be a lighter version of writeUnsignedDecimalAt()
     * when only 2 digits are needed. Seems to save about 110 bytes of flash.
     */
    void writeUnsignedDecimal2At(uint8_t pos, uint8_t num);

    /**
     * Clear the display from `pos` to the end. Useful after calling
     * writeSignedDecimalAt() and writeUnsignedDecimalAt() which prints a
     * variable number of digits.
     */
    void clearToEnd(uint8_t pos) {
      for (uint8_t i = pos; i < mLedDisplay.getNumDigits(); ++i) {
        writeInternalHexCharAt(i, kCharSpace);
      }
    }

  private:
    // disable copy-constructor and assignment operator
    NumberWriter(const NumberWriter&) = delete;
    NumberWriter& operator=(const NumberWriter&) = delete;

    /** Similar to writeHexCharAt() without performing bounds check. */
    void writeInternalHexCharAt(uint8_t pos, hexchar_t c);

    /** Similar to writeHexCharsAt() without performing bounds check. */
    void writeInternalHexCharsAt(uint8_t pos, const hexchar_t s[],
        uint8_t len) {
      for (uint8_t i = 0; i < len; ++i) {
        writeInternalHexCharAt(pos++, s[i]);
      }
    }

    /**
     * Print the hex characters in `s` inside a box of size `boxSize` at
     * position `pos`.
     *
     * @param boxSize if negative, left justified; if postive, right justified
     */
    uint8_t writeHexCharsInsideBoxAt(
        uint8_t pos,
        const hexchar_t s[],
        uint8_t len,
        int8_t boxSize);

    /**
     * Convert the integer num to an array of HexChar in the provided buf, with
     * the least significant digit going to buf[bufSize-1], and then working
     * backwards to the most significant digit.
     *
     * @param num number to convert
     * @param buf buffer of hex characters
     * @param bufSize must be 5 or larger (largest uint16_t is 65535, plus
     *    an optional sign bit if called from a signed version)
     *
     * @return index into buf that points to the start of the converted number,
     * e.g. for a single digit number, the returned value will be `bufSize-1`.
     */
    uint8_t toDecimal(uint16_t num, hexchar_t buf[], uint8_t bufSize) {
      uint8_t pos = bufSize;
      while (true) {
        if (num < 10) {
          buf[--pos] = num;
          break;
        }
        uint16_t quot = num / 10;
        buf[--pos] = num - quot * 10;
        num = quot;
      }
      return pos;
    }

  private:
    /** Bit pattern map for hex characters. */
    static const uint8_t kHexCharPatterns[kNumHexChars];

    LedDisplay& mLedDisplay;
};

}

#endif
