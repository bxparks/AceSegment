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
#include "../LedDisplay.h"
#include "CharWriter.h"

namespace ace_segment {

/**
 * Class that writes out a string, collapsing '.' characters into the decimal
 * point of the previous character. In other words, "0.2.3.4" takes up 4 digits,
 * not 7 digits.
 */
class StringWriter {
  public:
    /** Constructor. */
    explicit StringWriter(CharWriter& charWriter):
        mCharWriter(charWriter)
    {}

    /** Get the underlying LedDisplay. */
    LedDisplay& display() const {
      return mCharWriter.display();
    }

    /**
     * Write the string beginning at the specified position, filling up to
     * numDigits.
     *
     * @param pos starting digit position, 0 on the left
     * @param s string to be rendered
     * @param padRight fill the right side with empty spaces if we run out of
     *  characters in 's'
     */
    void writeStringAt(uint8_t pos, const char* s, bool padRight = false);

  private:
    // disable copy-constructor and assignment operator
    StringWriter(const StringWriter&) = delete;
    StringWriter& operator=(const StringWriter&) = delete;

    CharWriter& mCharWriter;
};

}

#endif
