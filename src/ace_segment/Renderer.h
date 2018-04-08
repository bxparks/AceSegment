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

namespace ace_segment {

class StyledDigit;
class Hardware;

class Renderer {
  public:
    static const uint8_t kBlinkStateOff = 0;
    static const uint8_t kBlinkStateOn = 1;

    /** Default frame rate. */
    static const uint8_t kFramesPerSecondDefault = 120;

    /** Constructor. */
    explicit Renderer(Hardware* hardware, Driver* driver,
            StyledDigit* styledDigits, uint8_t numDigits):
        mHardware(hardware),
        mDriver(driver),
        mStyledDigits(styledDigits),
        mNumDigits(numDigits),
        mFramesPerSecond(kFramesPerSecondDefault),
        mBrightness(255),
        mFramesPerStatsReset(kFramesPerStatsReset),
        mBlinkSlowDurationMillis(kBlinkSlowDurationMillisDefault),
        mBlinkSlowState(kBlinkStateOn),
        mBlinkFastDurationMillis(kBlinkFastDurationMillisDefault),
        mBlinkFastState(kBlinkStateOn),
        mPulseSlowDurationMillis(kPulseSlowDurationMillisDefault),
        mPulseFastDurationMillis(kPulseFastDurationMillisDefault)
    {}

    // Start configuration methods. These methods are expected to be set during
    // setup() and configure() should be called after changing them.

    /**
     * Set the desired frame rate. Default is 120.
     *
     * Borrowing some terminology from motion picture videos, a "frame" is a
     * full rendering of an image, and a field is a partial rendering of the
     * image. Each call to renderField() will process a single field of a frame.
     * Different wiring will require different number of fields to produce an
     * entire frame.
     *
     * fieldsPerSec(resistorsOnDigits) =  mFramesPerSecond * kNumSegments
     * fieldsPerSec(resistorsOnSegments) =  mFramesPerSecond * mNumDigits
     *
     * This constant is the desired number of frames per second. Default of 120
     * should produce minimal flickering.
     */
    Renderer& setFramesPerSecond(uint8_t framesPerSecond) {
      mFramesPerSecond = framesPerSecond;
      return *this;
    }

    /** Set blink slow duration millis. Default 800 millis. */
    Renderer& setBlinkSlowDuration(uint16_t durationMillis) {
      mBlinkSlowDurationMillis = durationMillis;
      return *this;
    }

    /** Set blink fast duration millis. Default 400 millis. */
    Renderer& setBlinkFastDuration(uint16_t durationMillis) {
      mBlinkFastDurationMillis = durationMillis;
      return *this;
    }

    /** Set pulse slow duration millis. Default 3000 millis. */
    Renderer& setPulseSlowDuration(uint16_t durationMillis) {
      mPulseSlowDurationMillis = durationMillis;
      return *this;
    }

    /** Set pulse fast duration millis. Default 1000 millis. */
    Renderer& setPulseFastDuration(uint16_t durationMillis) {
      mPulseFastDurationMillis = durationMillis;
      return *this;
    }

    /**
     * Set the number of frames after which the stats variables are
     * periodicallly reset. Set to 0 for no auto reset. Periodic resets are
     * useful to remove spurious min and max. The default is
     * kFramesPerStatsReset which is 300 frames, which at 60 frames per second
     * is 5 seconds.
     */
    Renderer& setStatsResetInterval(uint16_t framesPerStatsReset) {
      mFramesPerStatsReset = framesPerStatsReset;
      return *this;
    }

    /**
     * Configure the driver with the parameters given by the various setXxx()
     * methods.
     */
    void configure();

    // End configuration methods

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
     * brightest and is the default. Not implemented in any driver.
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

    /**
     * Display one field of a frame when the time is right. This is a polling
     * method, so call this slightly more frequently than getFieldsPerSecond().
     */
    void renderFieldWhenReady();

    /**
     * Render the current field immediately. Designed to be called from a timer
     * interrupt handler. The statistics variables give us the following
     * benchmar on a 16MHz ATmega328P using a 4-digit LED display, at 60 fps,
     * using 8 sub-fields per frame, with 2 digits pulsing, and 2 digits
     * blinking:
     *
     * SegmentDriver w/ LedMatrixDirect
     *    min: 28us; avg: 36-59us; max: 80us
     * DigitDriver w/ LedMatrixDirect
     *    min: 16us; avg: 47us; max: 96us
     * DigitDriver w/ LedMatrixSerial
     *    min: 16us; avg: 132us; max: 180us
     * DigitDriver w/ LedMatrixSpi
     *    min: 16us; avg: 40us; max: 72us
     * ModulatingDigitDriver w/ LedMatrixDirect
     *    min: 8us; avg: 12-20us; max: 124us
     * ModulatingDigitDriver w/ LedMatrixDirect w/ calcPulseFractionForFrame():
     *    min: 8us; avg: 12-20us; max: 192us
     * ModulatingDigitDriver w/ LedMatrixSerial
     *    min: 8us; avg: 10-12us; max: 204
     * ModulatingDigitDriver w/ LedMatrixSerial w/ FastHardware
     *    min: 8us; avg: 10-12us; max: 196
     * ModulatingDigitDriver w/ LedMatrixSpi
     *    min: 8us; avg: 12-20us; max: 92us
     * ModulatingDigitDriver w/ LedMatrixSpi w/ FastHardware
     *    min: 8us; avg: 12-20us; max: 84us
     */
    void renderField();

    /**
     * Return the average duration of the renderField() method.
     * Currently implemented as an exponential decay average.
     * For debugging only.
     */
    uint16_t getRenderFieldDurationAverage();

    /**
     * Return the min duration of the renderField() method.
     * For debugging only.
     */
    uint16_t getRenderFieldDurationMin();

    /**
     * Return the min duration of the renderField() method.
     * For debugging only.
     */
    uint16_t getRenderFieldDurationMax();

    /**
     * Return a running counter (the integer may overflow) of how many times
     * renderField() was called since the most recent stats reset. For debugging
     * purposes only.
     */
    uint16_t getRenderFieldCounter();

    /** Return a reference the styled digit. VisibleForTesting. */
    StyledDigit& getStyledDigit(uint8_t i) {
      return mStyledDigits[i];
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
    static const uint16_t kBlinkSlowDurationMillisDefault = 800;

    static const uint16_t kBlinkFastDurationMillisDefault = 400;

    static const uint16_t kPulseSlowDurationMillisDefault = 3000;

    static const uint16_t kPulseFastDurationMillisDefault = 1000;

    static const uint16_t kFramesPerStatsReset = 300;

    // disable copy-constructor and assignment operator
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /** Perform things that need to be done each frame. */
    void updateFrame();

    /** Calculate the blink and pulse states for current frame. */
    void calcBlinkAndPulseForFrame();

    /** Translate the StyledDigits to DimmingDigits for the Driver. */
    void renderStyledDigits();

    /** Reset the stats every setStatsResetInterval(). VisibleForTesting. */
    void resetStats();

    /** Update the stats given the duration of the renderField() method. */
    void updateStats(uint16_t duration);

    Hardware* const mHardware;
    Driver* const mDriver;
    StyledDigit* const mStyledDigits;
    const uint8_t mNumDigits;

    uint8_t mFramesPerSecond;
    uint8_t mBrightness;
    bool mIsPulseEnabled;

    // support polling
    uint16_t mMicrosPerField;
    uint16_t mLastRenderFieldMicros;

    // each call to renderField() increment current field counter modulo the
    // fieldsPerFrame, so certain calculations can be performed once per frame
    uint16_t mFieldsPerFrame;
    uint16_t mCurrentField;

    // statistics
    uint16_t mFramesPerStatsReset;
    uint16_t mCurrentStatsResetFrame;
    uint16_t mRenderFieldDurationAverage;
    uint16_t mRenderFieldDurationMin;
    uint16_t mRenderFieldDurationMax;
    uint16_t mRenderFieldCounter;

    // TODO: Maybe generalize the following into an array of styles, because the
    // calculations seem so similar.

    uint16_t mBlinkSlowDurationMillis;
    uint16_t mFramesPerBlinkSlow;
    uint16_t mCurrentBlinkSlowFrame;
    uint8_t mBlinkSlowState;

    uint16_t mBlinkFastDurationMillis;
    uint16_t mFramesPerBlinkFast;
    uint16_t mCurrentBlinkFastFrame;
    uint8_t mBlinkFastState;

    uint16_t mPulseSlowDurationMillis;
    uint16_t mFramesPerPulseSlow;
    uint16_t mFramesPerPulseSlowInverse; // 65536/mFramesPerPulseSlow
    uint16_t mCurrentPulseSlowFrame;
    uint8_t mPulseSlowFraction;

    uint16_t mPulseFastDurationMillis;
    uint16_t mFramesPerPulseFast;
    uint16_t mFramesPerPulseFastInverse; // 65536/mFramesPerPulseFast
    uint16_t mCurrentPulseFastFrame;
    uint8_t mPulseFastFraction;
};

}

#endif
