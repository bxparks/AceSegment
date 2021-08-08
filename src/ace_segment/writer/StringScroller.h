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

#ifndef ACE_SEGMENT_STRING_SCROLLER_H
#define ACE_SEGMENT_STRING_SCROLLER_H

#include <stdint.h>
#include <Arduino.h> // pgm_read_byte(), strlen_P()
#include "StringWriter.h"

class __FlashStringHelper;

namespace ace_segment {

/**
 * Class that scrolls a string left or right.
 *
 * @tparam T_LED_MODULE the class of the underlying LED Module, often LedModule
 *    but other classes with the same generic public methods can be substituted
 */
template <typename T_LED_MODULE>
class StringScroller {
  public:
    /** Constructor. */
    explicit StringScroller(CharWriter<T_LED_MODULE>& charWriter) :
        mCharWriter(charWriter)
    {}

    /** Get the underlying LedModule. */
    T_LED_MODULE& ledModule() { return mCharWriter.ledModule(); }

    /** Get the underlying PatternWriter. */
    PatternWriter<T_LED_MODULE>& patternWriter() {
      return mCharWriter.patternWriter();
    }

    /** Get the underlying LedModule. */
    CharWriter<T_LED_MODULE>& charWriter() { return mCharWriter; }

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return mCharWriter.getNumDigits(); }

    /** Set scroll string, clear the display, and prepare to scroll left. */
    void initScrollLeft(const char* s) {
      mString = s;
      mIsFlashString = false;
      mStringLength = strlen(s);
      mStringPos = -getNumDigits(); // start with clear display
      mCharWriter.clear();
    }

    /** Set scroll string, clear the display, and prepare to scroll left. */
    void initScrollLeft(const __FlashStringHelper* fs) {
      mString = fs;
      mIsFlashString = true;
      mStringLength = strlen_P((const char*) fs);
      mStringPos = -getNumDigits(); // start with clear display
      mCharWriter.clear();
    }

    /**
     * Scroll one position left. Return true when the scrolling is done and the
     * display is cleared
     */
    bool scrollLeft() {
      bool isDone = mStringPos >= mStringLength;
      if (! isDone) {
        mStringPos++;
      }
      writeString();
      return isDone;
    }

    /** Set scroll string, clear the display, and prepare to scroll right. */
    void initScrollRight(const char* s) {
      mString = s;
      mIsFlashString = false;
      mStringLength = strlen(s);
      mStringPos = mStringLength; // start with clear display
      mCharWriter.clear();
    }

    /** Set scroll string, clear the display, and prepare to scroll right. */
    void initScrollRight(const __FlashStringHelper* fs) {
      mString = fs;
      mIsFlashString = true;
      mStringLength = strlen_P((const char*) fs);
      mStringPos = mStringLength; // start with clear display
      mCharWriter.clear();
    }

    /**
     * Scroll one position left. Return true when the scrolling is done and the
     * display is cleared
     */
    bool scrollRight() {
      uint8_t numDigits = getNumDigits();
      bool isDone = (mStringPos <= -numDigits);
      if (! isDone) {
        mStringPos--;
      }
      writeString();
      return isDone;
    }

  private:
    // disable copy-constructor and assignment operator
    StringScroller(const StringScroller&) = delete;
    StringScroller& operator=(const StringScroller&) = delete;

    void writeString() {
      uint8_t numDigits = getNumDigits();
      int16_t stringPos = mStringPos;
      for (uint8_t pos = 0; pos < numDigits; pos++) {
        char c;
        if (stringPos < 0 || stringPos >= mStringLength) {
          c = ' ';
        } else {
          // Normally inserting an if-conditional inside a for-loop is not a
          // good idea. But this is not a speed-critical loop and it loops over
          // only the number of digits, which will almost always be <= 8. So I
          // think this is better than creating a duplicate version of this
          // function using a template function.
          if (mIsFlashString) {
            c = pgm_read_byte((const uint8_t*) mString + stringPos);
          } else {
            c = *((const char*) mString + stringPos);
          }
        }
        mCharWriter.writeCharAt(pos, c);
        stringPos++;
      }
    }

  private:
    // The order of these fields is partially motivated to reduce memory
    // consumption on 32-bit processors.
    CharWriter<T_LED_MODULE>& mCharWriter;
    const void* mString;
    int16_t mStringPos; // can become negative
    uint8_t mStringLength;
    bool mIsFlashString;
};

} // ace_segment

#endif
