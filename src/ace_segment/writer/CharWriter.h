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

#ifndef ACE_SEGMENT_CHAR_WRITER_H
#define ACE_SEGMENT_CHAR_WRITER_H

#include <stdint.h>
#include "../LedDisplay.h"

namespace ace_segment {

/**
 * The CharWriter supports mapping of ASCII (0 - 127) characters to segment
 * patterns supported by LedDisplay.
 */
class CharWriter {
  public:
    static const uint8_t kNumCharacters = 128;

    /** Constructor. */
    explicit CharWriter(LedDisplay& ledDisplay):
        mLedDisplay(ledDisplay)
    {}

    /** Write the character at the specified position. */
    void writeCharAt(uint8_t pos, char c);

    /** Get the underlying LedDisplay. */
    LedDisplay& display() const {
      return mLedDisplay;
    }

  private:
    // Bit pattern map for ASCII characters.
    static const uint8_t kCharacterArray[];

    // disable copy-constructor and assignment operator
    CharWriter(const CharWriter&) = delete;
    CharWriter& operator=(const CharWriter&) = delete;

    LedDisplay& mLedDisplay;
};

}

#endif
