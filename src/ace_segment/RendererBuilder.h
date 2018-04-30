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

#ifndef ACE_SEGMENT_RENDERER_BUILDER_H
#define ACE_SEGMENT_RENDERER_BUILDER_H

#include <stdint.h>
#include "Renderer.h"

namespace ace_segment {

class StyledPattern;
class Hardware;

/**
 * A builder for the Renderer. Create an instance of this object, optionally
 * call the various configuration methods, then call build() to get a new
 * instance of Renderer.
 */
class RendererBuilder {
  public:
    /** Constructor. */
    explicit RendererBuilder(Hardware* hardware, Driver* driver,
            StyledPattern* styledPatterns, uint8_t numDigits):
        mHardware(hardware),
        mDriver(driver),
        mStyledPatterns(styledPatterns),
        mNumDigits(numDigits),
        mFramesPerSecond(kFramesPerSecondDefault),
        mStatsResetInterval(kStatsResetIntervalDefault)
    {
      for (uint8_t i = 0; i < Renderer::kNumStyles; i++) {
        mStylers[i] = nullptr;
      }
    }

    /**
     * Set the desired frame rate. Default is 60.
     *
     * Borrowing some terminology from motion picture videos, a "frame" is a
     * full rendering of an image, and a field is a partial rendering of the
     * frame. Each call to renderField() will process a single field of a frame.
     * Different wiring will require different number of fields to produce an
     * entire frame.
     *
     * fieldsPerSec(resistorsOnDigits) =  mFramesPerSecond * kNumSegments
     * fieldsPerSec(resistorsOnSegments) =  mFramesPerSecond * mNumDigits
     */
    RendererBuilder& setFramesPerSecond(uint8_t framesPerSecond) {
      mFramesPerSecond = framesPerSecond;
      return *this;
    }

    /**
     * Set the maximum number of stats updates after which it is periodicallly
     * reset. Set to 0 for no auto reset. Periodic resets are useful to remove
     * spurious min and max. The default is kStatsResetIntervalDefault which is
     * 1200 samples. At 60 frames per second with 4 fields per frame, that's
     * about 5 seconds.
     */
    RendererBuilder& setStatsResetInterval(uint16_t statsResetInterval) {
      mStatsResetInterval = statsResetInterval;
      return *this;
    }

    /**
     * Set the Styler for the given styleIndex. Currently supports maximum of
     * kNumStyles which is 4.
     */
    RendererBuilder& setStyler(uint8_t styleIndex, Styler* styler) {
      if (styleIndex > 0 && styleIndex < Renderer::kNumStyles) {
        mStylers[styleIndex] = styler;
      }
      return *this;
    }

    /**
     * Return a new instance of Renderer with the various configurable
     * parameters. The Renderer::init() method must be called before using it.
     */
    Renderer* build() {
      return new Renderer(mHardware, mDriver, mStyledPatterns, mNumDigits,
          mFramesPerSecond, mStatsResetInterval, mStylers);
    }

  private:
    static const uint8_t kFramesPerSecondDefault = 60;
    static const uint16_t kStatsResetIntervalDefault = 1200;

    Hardware* const mHardware;
    Driver* const mDriver;
    StyledPattern* const mStyledPatterns;
    uint8_t const mNumDigits;
    uint8_t mFramesPerSecond;
    uint16_t mStatsResetInterval;
    Styler* mStylers[Renderer::kNumStyles];

};

}

#endif
