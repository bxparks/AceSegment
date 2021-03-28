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

#ifndef ACE_SEGMENT_PULSE_STYLER_H
#define ACE_SEGMENT_PULSE_STYLER_H

#include <stdint.h>
#include "Styler.h"

namespace ace_segment {

/**
 * Implements a pulsing style for a given digit.
 * Not thread-safe.
 */
class PulseStyler: public Styler {
  public:
    PulseStyler(uint8_t framesPerSecond, uint16_t durationMillis):
      mFramesPerPulse((uint32_t) framesPerSecond * durationMillis / 1000),
      mFramesPerPulseInverse((uint32_t) 65536 * 1000
          / framesPerSecond / durationMillis)
    {}

    void calcForFrame() override {
      uint16_t middleOfPulse = mFramesPerPulse / 2;

      uint16_t fraction;
      if (mCurrentFrame < middleOfPulse) {
        fraction = (uint32_t) mFramesPerPulseInverse * mCurrentFrame / (256/2);
      } else if (mCurrentFrame < mFramesPerPulse) {
        uint16_t reverse = (mFramesPerPulse - mCurrentFrame - 1);
        fraction = (uint32_t) mFramesPerPulseInverse * reverse / (256/2);
      } else {
        fraction = 0;
      }
      if (fraction > 255) fraction = 255;

      mPulseFraction = fraction;
      Util::incrementMod(mCurrentFrame, mFramesPerPulse);
    }

    void apply(uint8_t* /* pattern */, uint8_t* brightness) override {
      *brightness = ((uint16_t) mPulseFraction * (*brightness)) / 256;
    }

    bool requiresBrightness() override { return true; }

  private:
    const uint16_t mFramesPerPulse;
    const uint16_t mFramesPerPulseInverse; // 65536/mFramesPerPulse
    uint16_t mCurrentFrame = 0;
    uint8_t mPulseFraction;
};

}

#endif
