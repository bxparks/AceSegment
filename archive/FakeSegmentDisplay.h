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

#ifndef ACE_SEGMENT_FAKE_SCANNING_DISPLAY_H
#define ACE_SEGMENT_FAKE_SCANNING_DISPLAY_H

#include <stdint.h>
#include "../Renderer.h"

namespace ace_segment {

namespace testing {

/**
 * A fake version of ScanningDisplay for testing purposes. Previously called
 * "FakeRenderer".
 */
class FakeScanningDisplay: public ScanningDisplay {
  public:
    FakeScanningDisplay(DimmablePattern* dimmablePatterns, uint8_t numDigits)
      : ScanningDisplay(
          nullptr /* hardware */,
          nullptr /* driver */,
          dimmablePatterns,
          numDigits,
          60 /* framesPerSecond */,
          1200 /* statsResetInterval */)
    {}

    /** A stub implementation to prevent dependency on Hardware and Driver. */
    void configure()  override {}
};

}
}

#endif
