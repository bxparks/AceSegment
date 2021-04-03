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
#include <AceCommon.h> // TimingStats
#include "LedDisplay.h"

class SegmentDisplayTest_displayCurrentField;

namespace ace_segment {

using ace_common::TimingStats;

/**
 * The user interface to the LED Segment display. Uses an implementation of
 * the LedMatrixBase class to multiplex the LED segments on the digits.
 *
 * A frame is divided into fields. A field is a partial rendering of a frame.
 * Normally, the one digit corresponds to one field. However if brightness
 * control is enabled by setting `SUBFIELDS > 1`, then a single digit will be
 * rendered for SUBFIELDS number of times so that the brightness of the digit
 * will be controlled by PWM.
 *
 * There are 2 ways to get the expected number of frames per second:
 *
 *  1) Call the renderField() an ISR, or
 *  2) Call renderFieldWhenReady() polling method repeatedly from the global
 *    loop(), and an internal timing parameter will trigger a renderField() at
 *    the appropriate time.
 *
 * @tparam H class that provides access to the hardware pin and timing functions
 * @tparam LM LedMatrixBase class that provides access to LED segments
      (elements) organized by digit (group)
 * @tparam DIGITS number of LED digits
 * @tparam SUBFIELDS number of rendering fields per digit for PWM, normally 1,
 *   but set to greater than 1 to get number of brightness levels
 */
template <typename H, typename LM, uint8_t DIGITS, uint8_t SUBFIELDS>
class SegmentDisplay : public LedDisplay {

  public:
    static const uint8_t kDefaultBrightness = 128;

    /**
     * Constructor.
     *
     * @param hardware pointer to an instance of Hardware. Required.
     * @param ledMatrix instance of LedMatrixBase that understanding the wiring
     * @param framesPerSecond the rate at which all digits of the LED display
     *    will be refreshed
     * @param numDigits number of digits in the LED display
     * @param patterns array of segment pattern per digit, not nullable
     * @param brightnesses array of brightness for each digit (default: nullptr)
     * @param timingStats instance of TimingStats (default: nullptr)
     */
    explicit SegmentDisplay(
        const H& hardware,
        const LM& ledMatrix,
        uint8_t framesPerSecond,
        TimingStats* timingStats = nullptr
    ):
        LedDisplay(DIGITS),
        mHardware(hardware),
        mLedMatrix(ledMatrix),
        mTimingStats(timingStats),
        mFramesPerSecond(framesPerSecond)
    {}

    /**
     * Configure the driver with the parameters given by in the constructor.
     * Normally, this should be called only once after construction. Unit tests
     * will sometimes change a parameter of FakeDriver and call this a second
     * time.
     */
    void begin() {
      // Set up durations for the renderFieldWhenReady() polling function.
      mMicrosPerField = (uint32_t) 1000000UL / getFieldsPerSecond();
      mLastRenderFieldMicros = mHardware.micros();

      // Initialize variables needed for multiplexing.
      mCurrentDigit = 0;
      mPrevDigit = DIGITS - 1;
      mCurrentSubField = 0;
      mCurrentSubFieldMax = 0;
      mPattern = 0;

      // misc
      mPreparedToSleep = false;
      mLedMatrix.clear();
      if (SUBFIELDS > 1) {
        setGlobalBrightness(kDefaultBrightness);
      }
    }


    /** A no-op end() function for consistency with other classes. */
    void end() {}

    /** Get the number of digits. */
    uint8_t getNumDigits() const { return DIGITS; }

    /** Return the requested frames per second. */
    uint16_t getFramesPerSecond() const { return mFramesPerSecond; }

    /** Return the fields per second. */
    uint16_t getFieldsPerSecond() const {
      return mFramesPerSecond * getFieldsPerFrame();
    }

    /** Total fields per frame across all digits. */
    uint16_t getFieldsPerFrame() const { return DIGITS * SUBFIELDS; }

    void writePatternAt(uint8_t pos, uint8_t pattern) override {
      if (pos >= DIGITS) return;
      mPatterns[pos] = pattern;
    }

    void setBrightnessAt(uint8_t pos, uint8_t brightness) override {
      if (SUBFIELDS > 1) {
        if (pos >= DIGITS) return;
        mBrightnesses[pos] = brightness;
      }
    }

    void writeDecimalPointAt(uint8_t pos, bool state = true) override {
      if (pos >= DIGITS) return;
      uint8_t pattern = mPatterns[pos];
      if (state) {
        pattern |= 0x80;
      } else {
        pattern &= ~0x80;
      }
      mPatterns[pos] = pattern;
    }

    void setGlobalBrightness(uint8_t brightness) override {
      if (SUBFIELDS > 1) {
        for (uint8_t i = 0; i < DIGITS; i++) {
          mBrightnesses[i] = brightness;
        }
      }
    }

    void clear() override {
      for (uint8_t i = 0; i < DIGITS; i++) {
        mPatterns[i] = 0;
      }
    }

    /**
     * Display one field of a frame when the time is right. This is a polling
     * method, so call this slightly more frequently than getFieldsPerSecond()
     * per second.
     *
     * @return Returns true if renderField() was called and the field was
     *    rendered. The flag can be used to optimize the polling of the
     *    TimingStats instance. If the return value is false, there's no need to
     *    check.
     */
    bool renderFieldWhenReady() {
      uint16_t now = mHardware.micros();
      uint16_t elapsedMicros = now - mLastRenderFieldMicros;
      if (elapsedMicros >= mMicrosPerField) {
        renderField();
        mLastRenderFieldMicros = now;
        return true;
      } else {
        return false;
      }
    }

    /**
     * Render the current field immediately. Designed to be called from a timer
     * interrupt handler.
     */
    void renderField() {
      uint16_t now = mHardware.micros();
      displayCurrentField();
      uint16_t duration = mHardware.micros() - now;
      if (mTimingStats) mTimingStats->update(duration);
    }

    /**
     * Prepare to go to sleep by clearing the frame, and setting a flag so that
     * it doesn't turn itself back on through an interrupt.
     */
    void prepareToSleep() {
      mPreparedToSleep = true;
      mLedMatrix.clear();
    }

    /** Wake up from sleep. */
    void wakeFromSleep() { mPreparedToSleep = false; }

  private:
    friend class ::SegmentDisplayTest_displayCurrentField;

    // disable copy-constructor and assignment operator
    SegmentDisplay(const SegmentDisplay&) = delete;
    SegmentDisplay& operator=(const SegmentDisplay&) = delete;

    /**
     * Display the current field, usually just the current digit. If modulation
     * is supported, then each digit is PWM modulated over SUBFIELDS.
     */
    void displayCurrentField() {
      if (mPreparedToSleep) return;

      if (SUBFIELDS > 1) {
        displayCurrentFieldModulated();
      } else {
        displayCurrentFieldPlain();
      }
    }

    /** Display field normally without modulation. */
    void displayCurrentFieldPlain() {
      const uint8_t pattern = mPatterns[mCurrentDigit];
      mLedMatrix.draw(mCurrentDigit, pattern);
      mPrevDigit = mCurrentDigit;
      ace_common::incrementMod(mCurrentDigit, DIGITS);
    }

    /** Display field using subfield modulation. */
    void displayCurrentFieldModulated() {
      // Calculate the maximum subfield duration for a given digit.
      const uint8_t brightness = mBrightnesses[mCurrentDigit];
      if (mCurrentDigit != mPrevDigit) {
        mCurrentSubFieldMax = ((uint16_t) SUBFIELDS * brightness) / 256;
      }

      // No matter how small the SUBFIELDS, we want:
      // * If brightness == 0, then subfield 0 should be OFF.
      // * If brightness == 255, then special case that to be ON.
      const uint8_t pattern =
          (brightness == 0xFF || mCurrentSubField < mCurrentSubFieldMax)
          ? mPatterns[mCurrentDigit]
          : 0;

      if (pattern != mPattern || mCurrentDigit != mPrevDigit) {
        mLedMatrix.draw(mCurrentDigit, pattern);
        mPattern = pattern;
      }

      mCurrentSubField++;
      mPrevDigit = mCurrentDigit;
      if (mCurrentSubField >= SUBFIELDS) {
        ace_common::incrementMod(mCurrentDigit, DIGITS);
        mCurrentSubField = 0;
      }
    }

  private:
    static const uint8_t kMaxNumDigits = 8;

    // Ordered to save space on 32-bit processors.

    /** Indirection to the low level digitalWrite(), micros(). */
    const H& mHardware;

    /** LedMatrixBase instance that knows how to set and unset LED segments. */
    const LM& mLedMatrix;

    /** Pattern for each digit. Not nullable. */
    uint8_t mPatterns[DIGITS];

    /** Brightness for each digit. Nullable. */
    uint8_t mBrightnesses[DIGITS];

    /** Gather performance stats about renderField(). Nullable. */
    TimingStats* const mTimingStats;

    //-----------------------------------------------------------------------
    // Variables needed to render frames and fields at a certain rate per
    // second.
    //-----------------------------------------------------------------------

    // variables to support renderFieldWhenReady()
    uint16_t mMicrosPerField;
    uint16_t mLastRenderFieldMicros;

    /** Number of full frames (all digits) rendered per second. */
    uint8_t const mFramesPerSecond;

    //-----------------------------------------------------------------------
    // Variables needed to keep track of the multiplexing of the digits,
    // and PWM of a single digit.
    //-----------------------------------------------------------------------

    /**
     * Within the displayCurrentField() method, mCurrentDigit is the current
     * digit that is being drawn. It is incremented to the next digit just
     * before returning from that method.
     */
    uint8_t mCurrentDigit;

    /**
     * Within the displayCurrentField() method, the mPrevDigit is the digit
     * that was displayed on the previous call to displayCurrentField(). It is
     * set to the digit that was just displayed before returning. It will be
     * equal to mCurrentDigit when multiple fields are drawn for the same
     * digit.
     */
    uint8_t mPrevDigit;

    /**
     * Used by displayCurrentFieldModulated() and subclasses generated by
     * fast_driver.py.
     */
    uint8_t mCurrentSubField;

    /**
     * Used by displayCurrentFieldModulated() and subclasses generated by
     * fast_driver.py
     */
    uint8_t mCurrentSubFieldMax;

    /**
     * The segment pattern that is currently displaying on the LED. Used to
     * optimize the displayCurrentFieldModulated() method if the current
     * pattern is the same as the previous pattern.
     */
    uint8_t mPattern;

    //-----------------------------------------------------------------------
    // Variables to support low power sleeping on the MCU.
    //-----------------------------------------------------------------------

    /** Set to true just before going to sleep. */
    volatile bool mPreparedToSleep;
};

}

#endif