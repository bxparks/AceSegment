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
#include "Driver.h"
#include "TimingStats.h"

namespace ace_segment {

class StyledPattern;
class Hardware;

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
    // VisibleForTesting.
    static const uint8_t kBlinkStateOff = 0;
    static const uint8_t kBlinkStateOn = 1;

    /** Constructor. */
    explicit Renderer(Hardware* hardware, Driver* driver,
            StyledPattern* styledPatterns, uint8_t numDigits,
            uint8_t framesPerSecond,
            uint16_t statsResetInterval,
            uint16_t blinkSlowDurationMillis,
            uint16_t blinkFastDurationMillis,
            uint16_t pulseSlowDurationMillis,
            uint16_t pulseFastDurationMillis):
        mHardware(hardware),
        mDriver(driver),
        mStyledPatterns(styledPatterns),
        mNumDigits(numDigits),
        mFramesPerSecond(framesPerSecond),
        mStatsResetInterval(statsResetInterval),
        mBlinkSlowDurationMillis(blinkSlowDurationMillis),
        mBlinkFastDurationMillis(blinkFastDurationMillis),
        mPulseSlowDurationMillis(pulseSlowDurationMillis),
        mPulseFastDurationMillis(pulseFastDurationMillis),
        mBrightness(255),
        mIsPulseEnabled(false),
        mBlinkSlowState(kBlinkStateOn),
        mBlinkFastState(kBlinkStateOn)
    {}

    /** Destructor. */
    virtual ~Renderer() {}

    /**
     * Configure the driver with the parameters given by the various setXxx()
     * methods.
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
     * brightest (default); 1 is 1/256 of full brightness. Requires the support
     * of useModulatingDriver() option in DriverBuilder. If the driver doesn't
     * support brightness, then anything above 0 is full brightness and 0 turns
     * off the digit.
     */
    void writeBrightness(uint8_t brightness) {
      mBrightness = brightness;
    }

    /** Write the pattern and style for a given digit. */
    void writePatternAt(uint8_t digit, uint8_t pattern, uint8_t style);

    /** Write the pattern for a given digit, leaving style unchanged. */
    void writePatternAt(uint8_t digit, uint8_t pattern);

    /** Write the style for a given digit, leaving pattern unchanged. */
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
     * interrupt handler. The statistics variables give us the following
     * benchmar on a 16MHz ATmega328P using a 4-digit LED display, at 60 fps,
     * using 8 sub-fields per frame, with 2 digits pulsing, and 2 digits
     * blinking:
     *
     * DigitDriver w/ LedMatrixDirect
     *    min: 16us; avg: 73; max: 120us
     * DigitDriver w/ LedMatrixSerial
     *    min: 16us; avg: 128us; max: 200us
     * DigitDriver w/ LedMatrixSpi
     *    min: 16us; avg: 30-55us; max: 80us
     * ModulatingDigitDriver w/ LedMatrixDirect
     *    min: 8us; avg: 16us; max: 140us
     * ModulatingDigitDriver w/ LedMatrixDirect w/ calcPulseFractionForFrame():
     *    min: 8us; avg: 12-20us; max: 192us
     * ModulatingDigitDriver w/ LedMatrixSerial
     *    min: 8us; avg: 19us; max: 212us
     * ModulatingDigitDriver w/ LedMatrixSpi
     *    min: 8us; avg: 14us; max: 104us
     * SegmentDriver w/ LedMatrixDirect
     *    min: 32us; avg: 57us; max: 96us
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

    /** Calculate the blink state. VisibleForTesting. */
    static void calcBlinkStateForFrame(uint16_t framesPerBlink,
        uint16_t& currentFrame, uint8_t& blinkState);

    /** Calculate the pulse fraction. VisibleForTesting. */
    static void calcPulseFractionForFrame(uint16_t framesPerPulse,
        uint16_t& currentFrame, uint8_t& pulseFraction);

    /**
     * Calculate the pulse fraction using the reciprocal of the framesPerPulse,
     * which avoid a long division, which is very slow in 8-bit processors.
     * VisibleForTesting.
     */
    static void calcPulseFractionForFrameUsingInverse(
        uint16_t framesPerPulseInverse, uint16_t framesPerPulse,
        uint16_t& currentFrame, uint8_t& pulseFraction);

    /**
     * Calculate the effective brightness of a digit with the given style.
     * VisibleForTesting.
     *
     * @param brightness global brightness
     * @param style the style of the digit
     * @param blinkSlowState state of the blinkSlow cycle (on or off)
     * @param blinkFastState state of the blinkFast cycle (on or off)
     * @param isPulseEnabled true if the driver supports greyscale brightness,
     *    probably using PWM
     * @param pulseSlowFraction current brightness of the pulseSlow cycle
     * @param pulseFastFraction current brightness of the pulseSlow cycle
     *
     * @return the effective brightness of a digit with the given style
     */
    static uint8_t calcBrightness(uint8_t style, uint8_t brightness,
        uint8_t blinkSlowState, uint8_t blinkFastState, bool isPulseEnabled,
        uint8_t pulseSlowFraction, uint8_t pulseFastFraction);

  protected:
    // disable copy-constructor and assignment operator
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /** Perform things that need to be done each frame. */
    void updateFrame();

    /** Calculate the blink and pulse states for current frame. */
    void calcBlinkAndPulseForFrame();

    /** Translate the StyledPatterns to DimmablePatterns for the Driver. */
    void renderStyledPatterns();

    Hardware* const mHardware;
    Driver* const mDriver;
    StyledPattern* const mStyledPatterns;
    const uint8_t mNumDigits;
    const uint8_t mFramesPerSecond;

    // statistics
    const uint16_t mStatsResetInterval;
    TimingStats mStats;

    // digit style attributes
    const uint16_t mBlinkSlowDurationMillis;
    const uint16_t mBlinkFastDurationMillis;
    const uint16_t mPulseSlowDurationMillis;
    const uint16_t mPulseFastDurationMillis;

    // global brightness, can be changed during runtime
    uint8_t mBrightness;

    // depends on the Driver
    bool mIsPulseEnabled;

    // variables to support renderFieldWhenReady()
    uint16_t mMicrosPerField;
    uint16_t mLastRenderFieldMicros;

    // each call to renderField() increment current field counter modulo the
    // fieldsPerFrame, so certain calculations can be performed once per frame
    uint16_t mFieldsPerFrame;
    uint16_t mCurrentField;

    // TODO: Maybe generalize the following into an array of styles, because the
    // calculations seem so similar.

    uint16_t mFramesPerBlinkSlow;
    uint16_t mCurrentBlinkSlowFrame;
    uint8_t mBlinkSlowState;

    uint16_t mFramesPerBlinkFast;
    uint16_t mCurrentBlinkFastFrame;
    uint8_t mBlinkFastState;

    uint16_t mFramesPerPulseSlow;
    uint16_t mFramesPerPulseSlowInverse; // 65536/mFramesPerPulseSlow
    uint16_t mCurrentPulseSlowFrame;
    uint8_t mPulseSlowFraction;

    uint16_t mFramesPerPulseFast;
    uint16_t mFramesPerPulseFastInverse; // 65536/mFramesPerPulseFast
    uint16_t mCurrentPulseFastFrame;
    uint8_t mPulseFastFraction;
};

}

#endif
