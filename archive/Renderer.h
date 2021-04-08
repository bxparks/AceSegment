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
#include "LedMatrix.h"

namespace ace_segment {

/**
 * A class that multiplexes an array of segment patterns through each digit,
 * using the provided LedMatrix set the led segments for each digit.
 * Multiplexing occurs by scanning through the digits. This class knows how to
 * render a single field of a frame, where a frame and field are defined below.
 * Interestingly, this classes does not know about the timing between successive
 * rendering of fields. This is provided by ScanningDisplay, either through
 * polling or through an ISR.
 *
 * A single frame is the rendering of all digits of the LED module. If it has 4
 * digis, then each frame is composed of 4 fields. Brightness control through
 * PWM modulation requires the brightnesses array and numSubFields > 1. Each
 * digit is rendered for numSubFields times, before moving to the next digit.
 * The numSubFields becomes the number of available brightness levels. The total
 * number of fields per frame is (digits * numSubFields).
 */
class Renderer {
  public:
    /**
     * Constructor.
     *
     * @param ledMatrix instance of LedMatrix that understanding the wiring
     * @param numSubFields split a single digit field into this many subfields
     *    so that we can control its apparent brightness
     * @param numDigits number of patterns
     * @param patterns array of segment pattern per digit, not nullable
     * @param brightnesses array of brightness for each digit, nullable
     */
    explicit Renderer(
        LedMatrix* ledMatrix,
        uint8_t numDigits,
        const uint8_t* patterns,
        const uint8_t* brightnesses = nullptr,
        uint8_t numSubFields = 1
    ) :
        mLedMatrix(ledMatrix),
        mNumDigits(numDigits),
        mPatterns(patterns),
        mBrightnesses(brightnesses),
        mNumSubFields(numSubFields)
    {}

    void begin() {
      mCurrentDigit = 0;
      mPrevDigit = mNumDigits - 1;
      mCurrentSubField = 0;
      mCurrentSubFieldMax = 0;
      mPattern = 0;
      mPreparedToSleep = false;

      mLedMatrix->clear();
    }

    void end() {}

    uint16_t getFieldsPerFrame() {
      return mNumDigits * mNumSubFields;
    }

    bool isBrightnessSupported() { return mNumSubFields > 1; }

    /**
     * Display the current field, usually just the current digit. If modulation
     * is supported, then each digit is PWM modulated over mNumSubFields.
     */
    void displayCurrentField();

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
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /** Display field normally without modulation. */
    void displayCurrentFieldPlain();

    /** Display field using subfield modulation. */
    void displayCurrentFieldModulated();

  private:
    /**
     * Number of segments on a single digit. To support a 14-segment LED display
     * or a 16-segment LED display, we would need to change various bit fields
     * from uint8_t to uint16_t. It's a bit of work but doable.
     */
    static const uint8_t kNumSegments = 8;

    /** LedMatrix instance that knows how to set and unset LED segments. */
    LedMatrix* const mLedMatrix;

    /** Number of digits. */
    uint8_t const mNumDigits;

    /** Pattern for each digit. Not nullable. */
    uint8_t const* const mPatterns;

    /** Brightness for each digit. Nullable. */
    const uint8_t* const mBrightnesses;

    /**
     * Number of subfields to render per digit.
     * If this is greater than 1, use displayCurrentFieldModulated().
     */
    uint8_t const mNumSubFields;

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

    /** Set to true just before going to sleep. */
    volatile bool mPreparedToSleep;
};

}

#endif
