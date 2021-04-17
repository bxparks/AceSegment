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

#ifndef ACE_SEGMENT_TESTABLE_LED_MODULE_H
#define ACE_SEGMENT_TESTABLE_LED_MODULE_H

#include "../LedModule.h"

namespace ace_segment {
namespace testing {

/**
 * An implementation of LedModule for unit testing purposes.
 *
 * @tparam DIGITS number of digits supported by this class
 */
template <uint8_t DIGITS>
class TestableLedModule : public LedModule {
  public:
    TestableLedModule() : LedModule(DIGITS) {}

    void setBrightness(uint8_t /*brightness*/) override {}

    void setPatternAt(uint8_t pos, uint8_t pattern) override {
      mPatterns[pos] = pattern;
    }

    uint8_t getPatternAt(uint8_t pos) override {
      return mPatterns[pos];
    }

    uint8_t* getPatterns() { return mPatterns; }

  private:
    uint8_t mPatterns[DIGITS + 1]; // + 1 to test overflow
};

} // testing
} // ace_segment

#endif
