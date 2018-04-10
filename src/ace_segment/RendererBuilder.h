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

class StyledDigit;
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
            StyledDigit* styledDigits, uint8_t numDigits):
        mHardware(hardware),
        mDriver(driver),
        mStyledDigits(styledDigits),
        mNumDigits(numDigits),
        mFramesPerSecond(kFramesPerSecondDefault),
        mStatsResetInterval(kStatsResetIntervalDefault),
        mBlinkSlowDurationMillis(kBlinkSlowDurationMillisDefault),
        mBlinkFastDurationMillis(kBlinkFastDurationMillisDefault),
        mPulseSlowDurationMillis(kPulseSlowDurationMillisDefault),
        mPulseFastDurationMillis(kPulseFastDurationMillisDefault)
    {}

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

    /** Set blink slow duration millis. Default 800 millis. */
    RendererBuilder& setBlinkSlowDuration(uint16_t durationMillis) {
      mBlinkSlowDurationMillis = durationMillis;
      return *this;
    }

    /** Set blink fast duration millis. Default 400 millis. */
    RendererBuilder& setBlinkFastDuration(uint16_t durationMillis) {
      mBlinkFastDurationMillis = durationMillis;
      return *this;
    }

    /** Set pulse slow duration millis. Default 3000 millis. */
    RendererBuilder& setPulseSlowDuration(uint16_t durationMillis) {
      mPulseSlowDurationMillis = durationMillis;
      return *this;
    }

    /** Set pulse fast duration millis. Default 1000 millis. */
    RendererBuilder& setPulseFastDuration(uint16_t durationMillis) {
      mPulseFastDurationMillis = durationMillis;
      return *this;
    }

    /**
     * Return a new instance of Renderer with the various configurable
     * parameters. The Renderer::init() method must be called before using it.
     */
    Renderer* build() {
      return new Renderer(mHardware, mDriver, mStyledDigits, mNumDigits,
          mFramesPerSecond, mStatsResetInterval,
          mBlinkSlowDurationMillis, mBlinkFastDurationMillis,
          mPulseSlowDurationMillis, mPulseFastDurationMillis);
    }

  private:
    static const uint8_t kFramesPerSecondDefault = 60;

    static const uint16_t kStatsResetIntervalDefault = 1200;

    static const uint16_t kBlinkSlowDurationMillisDefault = 800;

    static const uint16_t kBlinkFastDurationMillisDefault = 400;

    static const uint16_t kPulseSlowDurationMillisDefault = 3000;

    static const uint16_t kPulseFastDurationMillisDefault = 1000;

    Hardware* const mHardware;
    Driver* const mDriver;
    StyledDigit* const mStyledDigits;
    uint8_t const mNumDigits;
    uint8_t mFramesPerSecond;
    uint16_t mStatsResetInterval;
    uint16_t mBlinkSlowDurationMillis;
    uint16_t mBlinkFastDurationMillis;
    uint16_t mPulseSlowDurationMillis;
    uint16_t mPulseFastDurationMillis;
};

}

#endif
