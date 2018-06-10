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

#ifndef ACE_SEGMENT_RENDERER_H
#define ACE_SEGMENT_RENDERER_H

#include <stdint.h>
#include "TimingStats.h"
#include "Driver.h"
#include "StyleTable.h"

namespace ace_segment {

class StyledPattern;
class Hardware;
class Styler;

/**
 * A class that knows how to translate an array of led segement bit patterns
 * with style attributes to a displayable frame for the Driver class.
 * The supported style for each digit are (blinkSlow, blinkFast, pulseSlow,
 * pulseFast).
 *
 * A frame is divided into fields, which is a partial rendering of a frame. The
 * renderField() (or renderFieldWhenReady()) method should be called to
 * successively render each field of a frame.
 */
class Renderer {
  public:
    /**
     * Constructor.
     *
     * @param hardware pointer to an instance of Hardware. Required.
     * @param driver pointer to a Driver instance. Required.
     * @param styledPatterns an array of StyledPattern representing the LED
     *    digits
     * @param styleTable a pointer to an instance of StyleTable. Optional,
     *    can be nullptr.
     * @param numDigits number of digits in the LED display
     * @param framesPerSecond the rate at which the LED display will be
     *    refreshed
     * @param statsResetInterval milliseconds between TimingStats reset
     */
    explicit Renderer(Hardware* hardware, Driver* driver,
            StyledPattern* styledPatterns, const StyleTable *styleTable,
            uint8_t numDigits, uint8_t framesPerSecond,
            uint16_t statsResetInterval):
        mHardware(hardware),
        mDriver(driver),
        mStyledPatterns(styledPatterns),
        mStyleTable(styleTable),
        mNumDigits(numDigits),
        mFramesPerSecond(framesPerSecond),
        mStatsResetInterval(statsResetInterval)
    {}

    /** Destructor. */
    virtual ~Renderer() {}

    /**
     * Configure the driver with the parameters given by in the constructor.
     * Normally, this should be called only once after construction. Unit tests
     * will sometimes change a parameter of FakeDriver and call this a second
     * time.
     */
    virtual void configure();

    /** Get the number of digits. */
    uint8_t getNumDigits() { return mNumDigits; }

    /** Return the frames per second. */
    uint16_t getFramesPerSecond() { return mFramesPerSecond; }

    /** Return the fields per second. */
    uint16_t getFieldsPerSecond() {
      return mFramesPerSecond * mDriver->getFieldsPerFrame();
    }

    /**
     * Set brightness expressed as a fraction of 256. In other words, 255 is
     * brightest (default); 1 is 1/256 of full brightness. Requires Driver
     * modulation using numSubFields greater than 1. If the driver doesn't
     * support brightness, then anything above 0 is full brightness and 0 turns
     * off the digit.
     */
    void writeBrightness(uint8_t brightness) {
      mBrightness = brightness;
    }

    /**
     * Write the pattern and style for a given digit.
     * If the digit is out of bounds, the method does nothing.
     * If the style is out of bounds or not registered, the method does nothing.
     */
    void writePatternAt(uint8_t digit, uint8_t pattern, uint8_t style);

    /**
     * Write the pattern for a given digit, leaving style unchanged.
     * If the digit is out of bounds, the method does nothing.
     */
    void writePatternAt(uint8_t digit, uint8_t pattern);

    /**
     * Write the style for a given digit, leaving pattern unchanged.
     * If the style is out of bounds or not registered, the method does nothing.
     */
    void writeStyleAt(uint8_t digit, uint8_t style);

    /** Write the decimal point for the digit. */
    void writeDecimalPointAt(uint8_t digit, bool state = true);

    /** Clear all digits, preserving the styles at each digit. */
    void clear();

    /**
     * Display one field of a frame when the time is right. This is a polling
     * method, so call this slightly more frequently than getFieldsPerSecond().
     *
     * @return Returns true if renderField() was called and the field was
     * rendered. The flag can be used to optimize the polling of
     * getTimingStats(). If the return value is false, there's no need to call
     * getTimingStats() again, since it will not have changed.
     */
    bool renderFieldWhenReady();

    /**
     * Render the current field immediately. Designed to be called from a timer
     * interrupt handler.
     */
    void renderField();

    /**
     * Return stats. For debugging only. TimingStats is returned by 'value' to
     * be safe from interrupts.
     */
    // TODO: make this a pointer and make stats gathering optional
    TimingStats getTimingStats();

    /** Return a reference the styled digit. VisibleForTesting. */
    StyledPattern& getStyledPattern(uint8_t i) {
      return mStyledPatterns[i];
    }

    /**
     * Return true if the given Styler is supported by the current Driver.
     * VisibleForTesting.
     */
    bool isStylerSupported(Styler* styler);

    /** Retrieve the array of active styles. VisibleForTesting. */
    uint8_t* getActiveStyles() { return mActiveStyles; }

  private:
    // disable copy-constructor and assignment operator
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /** Perform things that need to be done each frame. */
    void updateFrame();

    /** Update the stylers active stylers indicated by mActiveStyles. */
    void updateStylers();

    /** Translate the StyledPatterns to DimmablePatterns for the Driver. */
    void renderStyledPatterns();

    Hardware* const mHardware;
    Driver* const mDriver;
    StyledPattern* const mStyledPatterns;
    const StyleTable* const mStyleTable;
    const uint8_t mNumDigits;
    const uint8_t mFramesPerSecond;
    const uint16_t mStatsResetInterval;

    // TODO: change to a pointer to allow disabling it
    TimingStats mStats;

    // Count of the number of times the given style index is used in the
    // mStyledPatterns array. We update this map during writePatternAt() and
    // writeStyleAt() to avoid calculating this in renderField() which saves
    // CPU cycles.
    uint8_t mActiveStyles[StyleTable::kNumStyles];

    // global brightness, can be changed during runtime
    uint8_t mBrightness = 255;

    // does the Driver support brightness?
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
