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

#ifndef ACE_SEGMENT_SCANNING_DISPLAY_H
#define ACE_SEGMENT_SCANNING_DISPLAY_H

#include <stdint.h>
#include <Arduino.h> // pgm_read_byte()
#include <AceCommon.h> // incrementMod()
#include "LedDisplay.h"

class ScanningDisplayTest_displayCurrentField;

namespace ace_segment {

/**
 * An implementation of `LedDisplay` for display modules which do not have
 * hardware controller chips, so they require the microcontroller to perform the
 * multiplexed scanning across the digits. The matrix wiring of the segment and
 * digit pins allow only a single digit to be turned on at any given time, so
 * multiplexing across all the digits quickly (e.g. 60 Hz) gives the appearance
 * of activating all the digits at the same time. For LED modules with a
 * hardware controller (e.g. TM1637), the controller chip performs the
 * multiplexing. Note that a 74HC595 Shift Register chip does *not* perform the
 * multiplexing, it only provides a conversion from serial to parallel output.
 *
 * This class depends on one of the implementations of the `LedMatrixBase` class
 * to multiplex the LED segments on the digits, and potentially  one of the of
 * `SwSpiAdapter` or `HwSpiAdapter` classes if a 74HC595 shift register chip is
 * used.
 *
 * A frame is divided into fields. A field is a partial rendering of a frame.
 * Normally, the one digit corresponds to one field. However if brightness
 * control is enabled by setting `SUBFIELDS > 1`, then a single digit will be
 * rendered for SUBFIELDS number of times so that the brightness of the digit
 * will be controlled by PWM.
 *
 * There are 2 ways to get the expected number of frames per second:
 *
 *  1) Call the renderFieldNow() in an ISR, or
 *  2) Call renderFieldWhenReady() polling method repeatedly from the global
 *    loop(), and an internal timing parameter will trigger a renderFieldNow()
 *    at the appropriate time.
 *
 * @tparam HW class that provides access to hardware pins and timing functions
 * @tparam LM LedMatrixBase class that provides access to LED segments
      (elements) organized by digit (group)
 * @tparam DIGITS number of LED digits
 * @tparam SUBFIELDS number of rendering fields per digit for PWM, normally 1,
 *   but set to greater than 1 to get intermediate brightness levels
 */
template <typename HW, typename LM, uint8_t DIGITS, uint8_t SUBFIELDS>
class ScanningDisplay : public LedDisplay {

  public:
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
     */
    explicit ScanningDisplay(
        const HW& hardware,
        const LM& ledMatrix,
        uint8_t framesPerSecond
    ):
        LedDisplay(DIGITS),
        mHardware(hardware),
        mLedMatrix(ledMatrix),
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
      mPattern = 0;

      // Set initial patterns and global brightness.
      mLedMatrix.clear();
      if (SUBFIELDS > 1) {
        setGlobalBrightness(SUBFIELDS / 2); // half brightness
      }

      // Sleep mode support.
      mPreparedToSleep = false;
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

    void writePatternsAt(uint8_t pos, const uint8_t patterns[],
        uint8_t len) override {
      for (uint8_t i = 0; i < len; i++) {
        if (pos >= DIGITS) break;
        mPatterns[pos++] = patterns[i];
      }
    }

    void writePatternsAt_P(uint8_t pos, const uint8_t patterns[],
        uint8_t len) override {
      for (uint8_t i = 0; i < len; i++) {
        if (pos >= DIGITS) break;
        mPatterns[pos++] = pgm_read_byte(patterns + i);
      }
    }

    /**
     * @copyDoc
     *
     * The maximum brightness should theorectically be exactly `SUBFIELDS`, but
     * it is slightly cleaner to make `SUBFIELDS-1` behave as if it was the same
     * 100% brightness as the value of `SUBFIELDS`. The minimum brightness is 0,
     * which turns OFF the digit.
     *
     * For example, if `SUBFIELDS==16`, the the maximum brightness is 15 which
     * turns ON the digit 100% of the time. The relative brightness of each
     * brightness level is in units of 1/SUBFIELDS.
     *
     * The brightness scale is *not* normalized to [0,255]. A previous version
     * of this class tried to do that, but I found that this introduced
     * discretization errors which made it difficult to control the brightness
     * at intermediate values. For example, suppose SUBFIELDS=16. If we changed
     * the normalized brightness value from 32 to 40, it was impossible to
     * determine without actually running the program if the 2 values actually
     * differed in brightness. Instead, the calling program is expected to keep
     * track of the value of SUBFIELDS, and create an array of brightness values
     * in these raw units. The side benefit of using raw brightness values is
     * that it makes displayCurrentFieldModulated() easier to implement.
     */
    void setBrightnessAt(uint8_t pos, uint8_t brightness) override {
      if (SUBFIELDS > 1) {
        if (pos >= DIGITS) return;
        mBrightnesses[pos] = (brightness >= SUBFIELDS)
            ? SUBFIELDS - 1
            : brightness;
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

    /**
     * @copyDoc
     *
     * See the documentation for setBrightnessAt() for information about the
     * range of values of `brightness` and how it is interpreted.
     */
    void setGlobalBrightness(uint8_t brightness) override {
      if (SUBFIELDS > 1) {
        for (uint8_t i = 0; i < DIGITS; i++) {
          setBrightnessAt(i, brightness);
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
     * @return Returns true if renderFieldNow() was called and the field was
     *    rendered.
     */
    bool renderFieldWhenReady() {
      uint16_t now = mHardware.micros();
      uint16_t elapsedMicros = now - mLastRenderFieldMicros;
      if (elapsedMicros >= mMicrosPerField) {
        renderFieldNow();
        mLastRenderFieldMicros = now;
        return true;
      } else {
        return false;
      }
    }

    /**
     * Render the current field immediately. If modulation is off (i.e.
     * SUBFIELDS == 1), then the field corresponds to the single digit. If
     * modulation is enabled (SUBFIELDS > 1), then each digit is PWM modulated
     * over SUBFIELDS number of renderings.
     *
     * This method is intended to be called directly from a timer interrupt
     * handler.
     */
    void renderFieldNow() {
      if (mPreparedToSleep) return;

      if (SUBFIELDS > 1) {
        displayCurrentFieldModulated();
      } else {
        displayCurrentFieldPlain();
      }
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
    friend class ::ScanningDisplayTest_displayCurrentField;

    // disable copy-constructor and assignment operator
    ScanningDisplay(const ScanningDisplay&) = delete;
    ScanningDisplay& operator=(const ScanningDisplay&) = delete;

    /** Display field normally without modulation. */
    void displayCurrentFieldPlain() {
      const uint8_t pattern = mPatterns[mCurrentDigit];
      mLedMatrix.draw(mCurrentDigit, pattern);
      mPrevDigit = mCurrentDigit;
      ace_common::incrementMod(mCurrentDigit, DIGITS);
    }

    /** Display field using subfield modulation. */
    void displayCurrentFieldModulated() {
      // Calculate the maximum subfield duration for current digit.
      const uint8_t brightness = mBrightnesses[mCurrentDigit];

      // Implement pulse width modulation PWM.
      // No matter how small the SUBFIELDS:
      // * If brightness == 0, then turn the digit OFF 100% of the time.
      // * If brightness >= (SUBFIELDS - 1), turn the digit ON 100% of the time.
      const uint8_t pattern = (brightness >= SUBFIELDS - 1
              || mCurrentSubField < brightness)
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
    // Ordered to save space on 32-bit processors.

    /** Indirection to the digitalWrite(), micros() hardware functions. */
    const HW& mHardware;

    /** LedMatrixBase instance that knows how to set and unset LED segments. */
    const LM& mLedMatrix;

    /** Pattern for each digit. */
    uint8_t mPatterns[DIGITS];

    /** Brightness for each digit. Unused if SUBFIELDS <= 1. */
    uint8_t mBrightnesses[DIGITS];

    //-----------------------------------------------------------------------
    // Variables needed by renderFieldWhenReady() to render frames and fields at
    // a certain rate per second.
    //-----------------------------------------------------------------------

    /** Number of micros between 2 successive calls to renderFieldNow(). */
    uint16_t mMicrosPerField;

    /** Timestamp in micros of the last call to renderFieldNow(). */
    uint16_t mLastRenderFieldMicros;

    /** Number of full frames (all digits) rendered per second. */
    uint8_t const mFramesPerSecond;

    //-----------------------------------------------------------------------
    // Variables needed to keep track of the multiplexing of the digits,
    // and PWM of a single digit.
    //-----------------------------------------------------------------------

    /**
     * Within the renderFieldNow() method, mCurrentDigit is the current
     * digit that is being drawn. It is incremented to the next digit just
     * before returning from that method.
     */
    uint8_t mCurrentDigit;

    /**
     * Within the renderFieldNow() method, the mPrevDigit is the digit
     * that was displayed on the previous call to renderFieldNow(). It is
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
