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

#ifndef ACE_SEGMENT_STRING_WRITER_H
#define ACE_SEGMENT_STRING_WRITER_H

#include <stdint.h>
#include <AceCommon.h> // FlashString
#include "CharWriter.h"

class __FlashStringHelper;

namespace ace_segment {

/**
 * Class that writes out a string, collapsing '.' characters into the decimal
 * point of the previous character. In other words, "0.1.2.3" takes up 4 digits,
 * not 7 digits.
 *
 * @tparam T_LED_MODULE the class of the underlying LED Module, often LedModule
 *    but other classes with the same generic public methods can be substituted
 */
template <typename T_LED_MODULE>
class StringWriter {
  public:
    /** Constructor. */
    explicit StringWriter(CharWriter<T_LED_MODULE>& charWriter) :
        mCharWriter(charWriter)
    {}

    /** Get the underlying LedModule. */
    T_LED_MODULE& ledModule() { return mCharWriter.ledModule(); }

    /** Get the underlying PatternWriter. */
    PatternWriter<T_LED_MODULE>& patternWriter() {
      return mCharWriter.patternWriter();
    }

    /** Get the underlying CharWriter. */
    CharWriter<T_LED_MODULE>& charWriter() { return mCharWriter; }

    /**
     * Write c-string `cs` at specified position `pos` up to `numChar`
     * characters.
     *
     * @return number of actual LED digits written
     */
    uint8_t writeStringAt(
        uint8_t pos,
        const char* cs,
        uint8_t numChar = 255
    ) {
      return writeStringInternalAt<const char*>(pos, cs, numChar);
    }

    /**
     * Write flash string `fs` at specified position `pos` up to `numChar`
     * characters.
     *
     * @return number of actual LED digits written
     */
    uint8_t writeStringAt(
        uint8_t pos,
        const __FlashStringHelper* fs,
        uint8_t numChar = 255
    ) {
      return writeStringInternalAt<ace_common::FlashString>(
          pos, ace_common::FlashString(fs), numChar);
    }

    /** Clear the entire display. */
    void clear() { mCharWriter.clear(); }

    /** Clear the display from `pos` to the end. */
    void clearToEnd(uint8_t pos) { mCharWriter.clearToEnd(pos); }

  private:
    // disable copy-constructor and assignment operator
    StringWriter(const StringWriter&) = delete;
    StringWriter& operator=(const StringWriter&) = delete;

    /**
     * Write the string beginning at the specified position. Return the number
     * of digits actually written.
     *
     * @param pos starting digit position, 0 on the left
     * @param s string to be rendered
     * @param numChar number of characters to send to LED display
     *
     * @tparam T a c-string (const char*) or an instance of
     *    FlashString(const __FlashStringHelper*)
     */
    template <typename T>
    uint8_t writeStringInternalAt(uint8_t pos, T s, uint8_t numChar) {
      const uint8_t numDigits = mCharWriter.getNumDigits();
      const uint8_t originalPos = pos;
      bool charWasWritten = false;

      while (numChar--) {
        char c = *s;
        if (c == '\0') break;
        if (pos >= numDigits) break;

        // Use the decimal point just after a digit to render the '.' character.
        if (c == '.') {
          if (charWasWritten) {
            mCharWriter.writeDecimalPointAt(pos - 1);
          } else {
            mCharWriter.writeCharAt(pos, '.');
            charWasWritten = false;
            pos++;
          }
        } else {
          mCharWriter.writeCharAt(pos, c);
          charWasWritten = true;
          pos++;
        }
        s++;
      }

      return pos - originalPos;
    }

  private:
    CharWriter<T_LED_MODULE>& mCharWriter;
};

}

#endif
