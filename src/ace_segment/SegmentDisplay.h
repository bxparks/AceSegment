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

#ifndef ACE_SEGMENT_SEGMENT_DISPLAY_H
#define ACE_SEGMENT_SEGMENT_DISPLAY_H

#include <stdint.h>
#include "TimingStats.h"
#include "Renderer.h"

namespace ace_segment {

class Hardware;

/**
 * The user interface to the LED Segment display. Uses the Renderer, LedMatrix
 * and optionally the SpiAdapter.
 *
 * A frame is divided into fields, which is a partial rendering of a frame. The
 * renderField() (or renderFieldWhenReady()) method should be called to
 * successively render each field of a frame.
 */
class SegmentDisplay {
  public:
    static const uint8_t kDefaultBrightness = 128;

    /**
     * Constructor.
     *
     * @param hardware pointer to an instance of Hardware. Required.
     * @param renderer pointer to a Renderer instance. Required.
     * @param framesPerSecond the rate at which all digits of the LED display
     *    will be refreshed
     * @param numDigits number of digits in the LED display
     * @param patterns array of segment pattern per digit, not nullable
     * @param brightnesses array of brightness for each digit, nullable
     * @param timingStats optional instance of TimingStats
     */
    explicit SegmentDisplay(
        Hardware* hardware,
        Renderer* renderer,
        uint8_t framesPerSecond,
        uint8_t numDigits,
        uint8_t* patterns,
        uint8_t* brightnesses = nullptr,
        TimingStats* timingStats = nullptr
    ):
        mHardware(hardware),
        mRenderer(renderer),
        mFramesPerSecond(framesPerSecond),
        mNumDigits(numDigits),
        mPatterns(patterns),
        mBrightnesses(brightnesses),
        mTimingStats(timingStats)
    {}

    /**
     * Configure the driver with the parameters given by in the constructor.
     * Normally, this should be called only once after construction. Unit tests
     * will sometimes change a parameter of FakeDriver and call this a second
     * time.
     */
    void begin();

    /** Get the number of digits. */
    uint8_t getNumDigits() const { return mNumDigits; }

    /** Return the frames per second. */
    uint16_t getFramesPerSecond() const { return mFramesPerSecond; }

    /** Return the fields per second. */
    uint16_t getFieldsPerSecond() const {
      return mFramesPerSecond * mRenderer->getFieldsPerFrame();
    }

    /**
     * Set global brightness expressed as a fraction of 256. In other words, 255
     * is brightest (default); 1 is 1/256 of full brightness. Requires Renderer
     * modulation using numSubFields greater than 1. If the driver doesn't
     * support brightness, then anything above 0 is full brightness and 0 turns
     * off the digit.
     */
    void setGlobalBrightness(uint8_t brightness) {
      if (! mBrightnesses) return;
      for (uint8_t i = 0; i < mNumDigits; i++) {
        mBrightnesses[i] = brightness;
      }
    }

    /**
     * Write the pattern for a given pos. If the pos is out of bounds, the
     * method does nothing.
     */
    void writePatternAt(uint8_t pos, uint8_t pattern) {
      if (pos >= mNumDigits) return;
      mPatterns[pos] = pattern;
    }

    /**
     * Write the brightness for a given pos, leaving pattern unchanged.
     * If the pos is out of bounds, the method does nothing.
     */
    void setBrightnessAt(uint8_t pos, uint8_t brightness) {
      if (! mBrightnesses) return;
      if (pos >= mNumDigits) return;
      mBrightnesses[pos] = brightness;
    }


    /** Write the decimal point for the pos. */
    void writeDecimalPointAt(uint8_t pos, bool state = true);

    /** Clear all digits to blank pattern and all brightness to 0. */
    void clear();

    /**
     * Display one field of a frame when the time is right. This is a polling
     * method, so call this slightly more frequently than getFieldsPerSecond().
     *
     * @return Returns true if renderField() was called and the field was
     *    rendered. The flag can be used to optimize the polling of the
     *    TimingStats instance. If the return value is false, there's no need to
     *    check.
     */
    bool renderFieldWhenReady();

    /**
     * Render the current field immediately. Designed to be called from a timer
     * interrupt handler.
     */
    void renderField();

  private:
    // disable copy-constructor and assignment operator
    SegmentDisplay(const SegmentDisplay&) = delete;
    SegmentDisplay& operator=(const SegmentDisplay&) = delete;

  private:
    static const uint8_t kMaxNumDigits = 8;

    Hardware* const mHardware;
    Renderer* const mRenderer;
    const uint8_t mFramesPerSecond;
    const uint8_t mNumDigits;
    uint8_t* const mPatterns;
    uint8_t* const mBrightnesses;
    TimingStats* mTimingStats;

    // global brightness, can be changed during runtime
    uint8_t mBrightness = kDefaultBrightness;

    // does the Renderer support brightness?
    bool mIsBrightnessEnabled = false;

    // variables to support renderFieldWhenReady()
    uint16_t mMicrosPerField;
    uint16_t mLastRenderFieldMicros;

    // each call to renderField() increment current field counter modulo the
    // fieldsPerFrame, so certain calculations can be performed once per frame
    uint16_t mFieldsPerFrame;
    uint16_t mCurrentField;
};

}

#endif
