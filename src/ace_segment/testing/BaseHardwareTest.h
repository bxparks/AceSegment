#line 2 "BaseHardwareTest.h"

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

#ifndef ACE_SEGMENT_BASE_HARDWARE_TEST_H
#define ACE_SEGMENT_BASE_HARDWARE_TEST_H

#include <AUnit.h>
#include "TestableHardware.h"

using aunit::TestOnce;

namespace ace_segment {
namespace testing {

/**
 * Base class to assert various hardware events on TestableHardware.
 */
class BaseHardwareTest: public TestOnce {
  protected:
    virtual void setup() override {
      TestOnce::setup();
      mHardware = new TestableHardware();
    }

    virtual void teardown() override {
      delete mHardware;
      TestOnce::teardown();
    }

    /**
     * assertEvents(numEvents,
     *    type, arg1, arg2[, ...],
     *    ...
     *    type, arg1, arg2[, ...]);
     */
    void assertEvents(uint8_t n, ...);

    TestableHardware* mHardware;
};

} // namespace testing
} // namespace ace_segment


#endif
