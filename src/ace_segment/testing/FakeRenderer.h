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

#ifndef ACE_SEGMENT_FAKE_RENDERER_H
#define ACE_SEGMENT_FAKE_RENDERER_H

#include <stdint.h>
#include "../Renderer.h"

namespace ace_segment {

class StyledDigit;

namespace testing {

/**
 * A fake version of Renderer for testing purposes.
 */
class FakeRenderer: public Renderer {
  public:
    FakeRenderer(StyledDigit* styledDigits, uint8_t numDigits):
        Renderer(nullptr /* hardware */, nullptr /* driver */,
            styledDigits, numDigits,
            60 /* framesPerSecond */,
            1200 /* statsResetInterval */,
            600 /* blinkSlowDurationMillis */,
            300 /* blinkFastDurationMillis */,
            1200 /* pulseSlowDurationMillis */,
            600 /* pulseFastDurationMillis */)
    {}

    /** A stub implementation to prevent dependency on Hardware and Driver. */
    virtual void configure()  override {}
};

}
}

#endif
