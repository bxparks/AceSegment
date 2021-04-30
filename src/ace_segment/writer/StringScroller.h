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
#include <AceCommon.h> // FlashString
#include "StringWriter.h"

class __FlashStringHelper;

namespace ace_segment {

/**
 * Class that scrolls a string left or right.
 */
class StringScroller {
  public:
    /** Constructor. */
    explicit StringScroller(LedDisplay& ledDisplay) :
        mCharWriter(ledDisplay)
    {}

    /** Get the underlying LedDisplay. */
    LedDisplay& display() const { return mCharWriter.display(); }

    /** Set scroll string, clear the display, and prepare to scroll left. */
    void initScrollLeft(const char* s) {
      mString = s;
      mStringLength = strlen(s);
      mStringPos = - display().getNumDigits();
      display().clear();
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
      mStringLength = strlen(s);
      mStringPos = mStringLength;
      display().clear();
    }

    /**
     * Scroll one position left. Return true when the scrolling is done and the
     * display is cleared
     */
    bool scrollRight() {
      uint8_t numDigits = display().getNumDigits();
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
      uint8_t numDigits = display().getNumDigits();
      int16_t stringPos = mStringPos;
      for (uint8_t pos = 0; pos < numDigits; pos++) {
        char c;
        if (stringPos < 0 || stringPos >= mStringLength) {
          c = ' ';
        } else {
          c = mString[stringPos];
        }
        mCharWriter.writeCharAt(pos, c);
        stringPos++;
      }
    }

  private:
    CharWriter mCharWriter;
    const char* mString;
    int16_t mStringPos;
    uint8_t mStringLength;
};

} // ace_segment

#endif
