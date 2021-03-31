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
#include "Hardware.h"
#include "LedMatrix.h"

namespace ace_segment {

using ace_common::TimingStats;

class Hardware;

/**
 * The user interface to the LED Segment display. Uses the LedMatrix to
 * multiplex the LED segments on the digits.
 *
 * A frame is divided into fields. A field is a partial rendering of a frame.
 * Normally, the one digit corresponds to one field. However if brightness
 * control is enabled by setting `numSubFields > 1`, then a single digit will be
 * rendered for numSubFields number of times so that the brightness of the digit
 * will be controlled by PWM.
 *
 * There are 2 ways to get the expected number of frames per second:
 *
 *  1) Call the renderField() an ISR, or
 *  2) Call renderFieldWhenReady() polling method repeatedly from the global
 *    loop(), and an internal timing parameter will trigger a renderField() at
 *    the appropriate time.
 */
class SegmentDisplay {

  public:
    static const uint8_t kDefaultBrightness = 128;

    /**
     * Constructor.
     *
     * @param hardware pointer to an instance of Hardware. Required.
     * @param ledMatrix instance of LedMatrix that understanding the wiring
     * @param framesPerSecond the rate at which all digits of the LED display
     *    will be refreshed
     * @param numDigits number of digits in the LED display
     * @param patterns array of segment pattern per digit, not nullable
     * @param brightnesses array of brightness for each digit (default: nullptr)
     * @param numSubFields number of fields per digit so that we can control its
     *    apparent brightness using PWM (default: 1)
     * @param timingStats instance of TimingStats (default: nullptr)
     */
    explicit SegmentDisplay(
        Hardware* hardware,
        LedMatrix* ledMatrix,
        uint8_t framesPerSecond,
        uint8_t numDigits,
        uint8_t* patterns,
        uint8_t* brightnesses = nullptr,
        uint8_t numSubFields = 1,
        TimingStats* timingStats = nullptr
    ):
        mNumDigits(numDigits),
        mFramesPerSecond(framesPerSecond),
        mNumSubFields(numSubFields),
        mHardware(hardware),
        mLedMatrix(ledMatrix),
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
    void begin() {
      // Set up durations for the renderFieldWhenReady() polling function.
      mMicrosPerField = (uint32_t) 1000000UL / getFieldsPerSecond();
      mLastRenderFieldMicros = mHardware->micros();

      // Initialize variables needed for multiplexing.
      mCurrentDigit = 0;
      mPrevDigit = mNumDigits - 1;
      mCurrentSubField = 0;
      mCurrentSubFieldMax = 0;
      mPattern = 0;

      // misc
      mPreparedToSleep = false;
      mLedMatrix->clear();
    }


    /** A no-op end() function for consistency with other classes. */
    void end() {}

    /** Get the number of digits. */
    uint8_t getNumDigits() const { return mNumDigits; }

    /** Return the requested frames per second. */
    uint16_t getFramesPerSecond() const { return mFramesPerSecond; }

    /** Return the fields per second. */
    uint16_t getFieldsPerSecond() const {
      return mFramesPerSecond * getFieldsPerFrame();
    }

    /** Total fields per frame across all digits. */
    uint16_t getFieldsPerFrame() const { return mNumDigits * mNumSubFields; }

    /**
     * Set global brightness expressed as a fraction of 256. In other words, 255
     * is brightest (default); 1 is 1/256 of full brightness. Requires
     * `numSubFields` greater than 1 and a non-null `brightnesses` array.
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
    void writeDecimalPointAt(uint8_t pos, bool state = true) {
      if (pos >= mNumDigits) return;
      uint8_t pattern = mPatterns[pos];
      if (state) {
        pattern |= 0x80;
      } else {
        pattern &= ~0x80;
      }
      mPatterns[pos] = pattern;
    }

    /** Clear all digits to blank pattern and all brightness to 0. */
    void clear() {
      for (uint8_t i = 0; i < mNumDigits; i++) {
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
    bool renderFieldWhenReady();

    /**
     * Render the current field immediately. Designed to be called from a timer
     * interrupt handler.
     */
    void renderField();

    /**
     * Prepare to go to sleep by clearing the frame, and setting a flag so that
     * it doesn't turn itself back on through an interrupt.
     */
    void prepareToSleep() {
      mPreparedToSleep = true;
      mLedMatrix->clear();
    }

    /** Wake up from sleep. */
    void wakeFromSleep() { mPreparedToSleep = false; }

  private:
    // disable copy-constructor and assignment operator
    SegmentDisplay(const SegmentDisplay&) = delete;
    SegmentDisplay& operator=(const SegmentDisplay&) = delete;

    /**
     * Display the current field, usually just the current digit. If modulation
     * is supported, then each digit is PWM modulated over mNumSubFields.
     */
    void displayCurrentField() {
      if (mPreparedToSleep) return;

      if (mNumSubFields == 1) {
        displayCurrentFieldPlain();
      } else {
        displayCurrentFieldModulated();
      }
    }

    /** Display field normally without modulation. */
    void displayCurrentFieldPlain();

    /** Display field using subfield modulation. */
    void displayCurrentFieldModulated();

  private:
    static const uint8_t kMaxNumDigits = 8;

    // Ordered to save space on 32-bit processors.

    /** Number of digits. */
    uint8_t const mNumDigits;

    /** Number of full frames (all digits) rendered per second. */
    uint8_t const mFramesPerSecond;

    /**
     * Number of subfields to render per digit.
     * If this is greater than 1, use displayCurrentFieldModulated().
     */
    uint8_t const mNumSubFields;

    /** Indirection to the low level digitalWrite(), micros(). */
    Hardware* const mHardware;

    /** LedMatrix instance that knows how to set and unset LED segments. */
    LedMatrix* const mLedMatrix;

    /** Pattern for each digit. Not nullable. */
    uint8_t* const mPatterns;

    /** Brightness for each digit. Nullable. */
    uint8_t* const mBrightnesses;

    /** Gather performance stats about renderField(). Nullable. */
    TimingStats* const mTimingStats;

    //-----------------------------------------------------------------------
    // Variables needed to render frames and fields at a certain rate per
    // second.
    //-----------------------------------------------------------------------

    // variables to support renderFieldWhenReady()
    uint16_t mMicrosPerField;
    uint16_t mLastRenderFieldMicros;

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
