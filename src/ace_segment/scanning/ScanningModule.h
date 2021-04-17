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

#ifndef ACE_SEGMENT_SCANNING_MODULE_H
#define ACE_SEGMENT_SCANNING_MODULE_H

#include <stdint.h>
#include <AceCommon.h> // incrementMod()
#include "../LedModule.h"

class ScanningModuleTest_displayCurrentField;

namespace ace_segment {

/**
 * An implementation of `LedModule` for display modules which do not have
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
 * @tparam LM the LedMatrixBase class that provides access to LED segments
      (elements) organized by digit (group)
 * @tparam DIGITS number of LED digits
 * @tparam SUBFIELDS number of subfields for each digit to get brightness
 *    control using PWM. The default is 1, but can be set to greater than 1 to
 *    get brightness control.
 * @tparam CI class that provides access to Arduino clock functions (millis()
 *    and micros()). The default is ClockInterface.
 */
template <
    typename LM,
    uint8_t DIGITS,
    uint8_t SUBFIELDS = 1,
    typename CI = ClockInterface>
class ScanningModule : public LedModule {

  public:
    /**
     * Constructor.
     *
     * @param ledMatrix instance of LedMatrixBase that understanding the wiring
     * @param framesPerSecond the rate at which all digits of the LED display
     *    will be refreshed
     * @param numDigits number of digits in the LED display
     * @param patterns array of segment pattern per digit, not nullable
     * @param brightnesses array of brightness for each digit (default: nullptr)
     */
    explicit ScanningModule(
        const LM& ledMatrix,
        uint8_t framesPerSecond
    ):
        LedModule(DIGITS),
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
      mLastRenderFieldMicros = CI::micros();

      // Initialize variables needed for multiplexing.
      mCurrentDigit = 0;
      mPrevDigit = DIGITS - 1;
      mCurrentSubField = 0;
      mPattern = 0;

      // Set initial patterns and global brightness.
      mLedMatrix.clear();
      if (SUBFIELDS > 1) {
        setBrightness(SUBFIELDS / 2); // half brightness
      }
    }


    /** A no-op end() function for consistency with other classes. */
    void end() {}

    //-----------------------------------------------------------------------
    // Implement the LedModule interface.
    //-----------------------------------------------------------------------

    /** Get the number of digits. */
    uint8_t getNumDigits() const { return DIGITS; }

    void setPatternAt(uint8_t pos, uint8_t pattern) override {
      mPatterns[pos] = pattern;
    }

    uint8_t getPatternAt(uint8_t pos) override {
      return mPatterns[pos];
    }

    /**
     * @copydoc
     *
     * See the documentation for setBrightnessAt() for information about the
     * range of values of `brightness` and how it is interpreted.
     */
    void setBrightness(uint8_t brightness) override {
      for (uint8_t i = 0; i < DIGITS; i++) {
        setBrightnessAt(i, brightness);
      }
    }

    //-----------------------------------------------------------------------
    // Additional brightness control. ScanningModule allows brightness to be
    // defined on a per-digit basis.
    //-----------------------------------------------------------------------

    /**
     * Set the brightness for a given pos, leaving pattern unchanged.
     * Not all implementation of `LedClass` can support brightness for each
     * digit, so this is implemented at the ScanningModule class.
     *
     * The maximum brightness should is exactly `SUBFIELDS` which turns on the
     * LED 100% of the time. The minimum brightness is 0, which turns OFF the
     * digit. For example, if `SUBFIELDS==16`, the the maximum brightness is 16
     * which turns ON the digit 100% of the time. The relative brightness of
     * each brightness level is in units of 1/SUBFIELDS.
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
    void setBrightnessAt(uint8_t pos, uint8_t brightness) {
      if (pos >= DIGITS) return;
      mBrightnesses[pos] = (brightness >= SUBFIELDS) ? SUBFIELDS : brightness;
    }

    //-----------------------------------------------------------------------
    // Methods related to rendering.
    //-----------------------------------------------------------------------

    /** Return the requested frames per second. */
    uint16_t getFramesPerSecond() const { return mFramesPerSecond; }

    /** Return the fields per second. */
    uint16_t getFieldsPerSecond() const {
      return mFramesPerSecond * getFieldsPerFrame();
    }

    /** Total fields per frame across all digits. */
    uint16_t getFieldsPerFrame() const { return DIGITS * SUBFIELDS; }

    /**
     * Display one field of a frame when the time is right. This is a polling
     * method, so call this slightly more frequently than getFieldsPerSecond()
     * per second.
     *
     * @return Returns true if renderFieldNow() was called and the field was
     *    rendered.
     */
    bool renderFieldWhenReady() {
      uint16_t now = CI::micros();
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
      if (SUBFIELDS > 1) {
        displayCurrentFieldModulated();
      } else {
        displayCurrentFieldPlain();
      }
    }

  private:
    friend class ::ScanningModuleTest_displayCurrentField;

    // disable copy-constructor and assignment operator
    ScanningModule(const ScanningModule&) = delete;
    ScanningModule& operator=(const ScanningModule&) = delete;

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

      // Implement pulse width modulation PWM, using the following boundaries:
      //
      // * If brightness == 0, then turn the digit OFF 100% of the time.
      // * If brightness >= SUBFIELDS, turn the digit ON 100% of the time.
      //
      // The mCurrentSubField is incremented modulo SUBFIELDS, so will always be
      // in the range of [0, SUBFIELDS-1]. The brightness will always be <=
      // SUBFIELDS, with the value of SUBFIELDS being 100% bright. So if we turn
      // on the LED when (mCurrentSubField < brightness), we get the desired
      // outcome.
      const uint8_t pattern = (mCurrentSubField < brightness)
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
    // The ordering of the fields below partially motivated to save memory on
    // 32-bit processors.

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
};

}

#endif
