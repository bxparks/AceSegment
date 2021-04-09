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

#include "StringWriter.h"

namespace ace_segment {

void StringWriter::writeStringAt(uint8_t digit, const char* s, bool padRight) {
  LedDisplay& ledDisplay = mCharWriter.display();
  bool charWasWritten = false;
  uint8_t numDigits = ledDisplay.getNumDigits();

  while (true) {
    char c = *s;
    if (c == 0) break;
    if (digit >= numDigits) break;

    // Use the decimal point just after a digit to render the '.' character.
    if (c == '.') {
      if (charWasWritten) {
        ledDisplay.writeDecimalPointAt(digit - 1);
      } else {
        mCharWriter.writeCharAt(digit, '.');
        charWasWritten = false;
        digit++;
      }
    } else {
      mCharWriter.writeCharAt(digit, c);
      charWasWritten = true;
      digit++;
    }
    s++;
  }

  if (padRight) {
    for (; digit < numDigits; digit++) {
      mCharWriter.writeCharAt(digit, ' ');
    }
  }
}

}
