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
#include "PatternWriter.h"

namespace ace_segment {

/** Total number of characters in the kHexCharPatterns[] set. */
const uint8_t kNumHexCharPatterns = 18;

/** Bit pattern map for hex characters. */
extern const uint8_t kHexCharPatterns[kNumHexCharPatterns];

/**
 * The type of the character set supported by many methods in the NumberWriter
 * class usually containing the string "HexChar" in its name. This custom
 * character set is not ASCII to save flash memory. It is a restricted set that
 * starts with 0 and goes up to 0xF to support hexadecimal digits. In addition,
 * the character set adds few more characters for convenience:
 *
 *  * kHexCharSpace = 0x10 = space character
 *  * kHexCharMinus = 0x11 = minus character
 *
 * The `hexchar_t` typedef is an alias for `uint8_t` and unfortunately, C++
 * will not prevent mixing a normal `char` or `uint8_t` with a `hexchar_t`.
 * However, it does make the documentation of the various methods more
 * self-explanatory.
 */
typedef uint8_t hexchar_t;

/** The code point in kHexCharPatterns[] corresponding to a space character. */
const hexchar_t kHexCharSpace = 0x10;

/** The code point in kHexCharPatterns[] corresponding to a minus character. */
const hexchar_t kHexCharMinus = 0x11;

/**
 * The NumberWriter supports converting decimal and hexadecimal numbers to
 * segment patterns expected by LedModule. The character set includes 0 to F,
 * and a few other characters which should be self-explanatory: kHexCharSpace
 * and kHexCharMinus.
 *
 * @tparam T_LED_MODULE the class of the underlying LED Module, often LedModule
 *    but other classes with the same generic public methods can be substituted
 */
template <typename T_LED_MODULE>
class NumberWriter {
  public:
    /** Constructor. */
    explicit NumberWriter(T_LED_MODULE& ledModule) :
        mPatternWriter(ledModule)
    {}

    /** Get the underlying LedModule. */
    T_LED_MODULE& ledModule() { return mPatternWriter.ledModule(); }

    /** Get the underlying PatternWriter. */
    PatternWriter<T_LED_MODULE>& patternWriter() { return mPatternWriter; }

    /**
     * Write the hex character `c` at position `pos`. If `c` falls outside the
     * valid range of the kHexCharPatterns set, a `kHexCharSpace` character is
     * printed instead.
     */
    void writeHexCharAt(uint8_t pos, hexchar_t c) {
      writeInternalHexCharAt(
          pos, ((uint8_t) c < kNumHexCharPatterns) ? c : kHexCharSpace);
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
        int8_t boxSize = 0) {

      const uint8_t bufSize = 5;
      hexchar_t buf[bufSize];
      uint8_t start = toDecimal(num, buf, bufSize);

      return writeHexCharsInsideBoxAt(
          pos, &buf[start], bufSize - start, boxSize);
    }

    /** Same as writeUnsignedDecimalAt() but prepends a '-' sign if negative. */
    uint8_t writeSignedDecimalAt(
        uint8_t pos,
        int16_t num,
        int8_t boxSize = 0) {

      // Even -32768 turns into +32768, which is exactly what we want
      bool negative = num < 0;
      uint16_t absNum = negative ? -num : num;

      const uint8_t bufSize = 6;
      hexchar_t buf[bufSize];
      uint8_t start = toDecimal(absNum, buf, bufSize);
      if (negative) {
        buf[--start] = kHexCharMinus;
      }

      return writeHexCharsInsideBoxAt(
          pos, &buf[start], bufSize - start, boxSize);
    }

    /**
     * Write the 2 digit decimal number at pos. This method always writes 2
     * characters. The padding is always a space, so 9 is printed as " 9" not
     * "09". If num >= 100, then this prints "99".
     *
     * This method is meant to be a lighter version of writeUnsignedDecimalAt()
     * when only 2 digits are needed. Seems to save about 110 bytes of flash.
     */
    void writeUnsignedDecimal2At(uint8_t pos, uint8_t num) {
      if (num >= 100) {
        writeHexCharAt(pos++, 9);
        writeHexCharAt(pos++, 9);
      } else {
        uint8_t high = num / 10;
        uint8_t low = num - high * 10;
        writeHexCharAt(pos++, (high == 0) ? kHexCharSpace : high);
        writeHexCharAt(pos++, low);
      }
    }

    /**
     * Write the decimal point for the pos. Clock LED modules will attach the
     * colon segment to one of the decimal points.
     */
    void writeDecimalPointAt(uint8_t pos, bool state = true) {
      mPatternWriter.writeDecimalPointAt(pos, state);
    }

    /** Clear the entire display. */
    void clear() { clearToEnd(0); }

    /**
     * Clear the display from `pos` to the end. Useful after calling
     * writeSignedDecimalAt() and writeUnsignedDecimalAt() which prints a
     * variable number of digits.
     */
    void clearToEnd(uint8_t pos) { mPatternWriter.clearToEnd(pos); }

  private:
    // disable copy-constructor and assignment operator
    NumberWriter(const NumberWriter&) = delete;
    NumberWriter& operator=(const NumberWriter&) = delete;

    /** Similar to writeHexCharAt() without performing bounds check. */
    void writeInternalHexCharAt(uint8_t pos, hexchar_t c) {
      uint8_t pattern = pgm_read_byte(&kHexCharPatterns[(uint8_t) c]);
      mPatternWriter.writePatternAt(pos, pattern);
    }

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
        int8_t boxSize) {

      uint8_t absBoxSize = (boxSize < 0) ? -boxSize : boxSize;

      // if the box is too small, print normally
      if (len >= absBoxSize) {
        writeInternalHexCharsAt(pos, s, len);
        return len;
      }

      // Print either left justified or right justified inside box
      uint8_t padSize = absBoxSize - len;
      if (boxSize < 0) {
        // left justified
        writeInternalHexCharsAt(pos, s, len);
        pos += len;
        while (padSize--) writeInternalHexCharAt(pos++, kHexCharSpace);
      } else {
        // right justified
        while (padSize--) writeInternalHexCharAt(pos++, kHexCharSpace);
        writeInternalHexCharsAt(pos, s, len);
      }

      return absBoxSize;
    }

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
    PatternWriter<T_LED_MODULE> mPatternWriter;
};

}

#endif
