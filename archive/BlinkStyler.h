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

#ifndef ACE_SEGMENT_BLINK_STYLER_H
#define ACE_SEGMENT_BLINK_STYLER_H

#include <stdint.h>
#include "Styler.h"

namespace ace_segment {

/**
 * Implement a blinking style for a particular digit.
 * Not thread-safe.
 */
class BlinkStyler: public Styler {
  public:
    BlinkStyler(uint8_t framesPerSecond, uint16_t durationMillis):
      mFramesPerBlink((uint32_t) framesPerSecond * durationMillis / 1000)
    {}

    void calcForFrame() override {
      uint16_t middleOfBlink = mFramesPerBlink / 2;
      mBlinkState = (mCurrentFrame < middleOfBlink) ? kOn : kOff;
      Util::incrementMod(mCurrentFrame, mFramesPerBlink);
    }

    void apply(uint8_t* /* pattern */, uint8_t* brightness) override {
      if (mBlinkState == kOff) {
        *brightness = 0;
      }
    }

    bool requiresBrightness() override { return false; }

  private:
    static const uint8_t kOff = 0;
    static const uint8_t kOn = 1;

    const uint16_t mFramesPerBlink;
    uint16_t mCurrentFrame = 0;
    uint8_t mBlinkState;
};

}

#endif
