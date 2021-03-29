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

#ifndef ACE_SEGMENT_DRIVER_H
#define ACE_SEGMENT_DRIVER_H

#include <stdint.h>
#include "DimmablePattern.h"

namespace ace_segment {

class LedMatrix;

/**
 * Base class of drivers which knows how to transfer the bit patterns stored in
 * the array of DimmablePattern objects to the actual LED display. Different
 * wiring configuration will require different drivers.
 *
 * Each call to displayCurrentField() displays one field of a frame. The object
 * is expected to keep internal state so that the next call to
 * displayCurrentField() displays the next field.
 */
class Driver {
  public:
    /**
     * Integer type used to store the segment bit patterns of a single digit.
     * For an 7-segment LED, this is a uint8_t because we need 8 bits including
     * the decimal point.
     */
    typedef uint8_t SegmentPatternType;

    /**
     * Integer type used to store the digit bit patterns of a single segment. In
     * the current implementations Driver, this must be able to hold
     * 'mNumDigits' number of bits.
     */
    typedef uint8_t DigitPatternType;

    /**
     * Virtual destructor. Needed in unit tests where multiple drivers are
     * created and deleted.
     */
    virtual ~Driver() {}

    /**
     * Configure the driver. Usually called only once in a program.
     * Sometimes unit tests may call it twice.
     */
    virtual void configure();

    /** Turn off the LEDs by doing the opposite of configure(). */
    virtual void finish();

    /**
     * Display the current field of the frame. Automatically advances to the
     * next field for the next call. A frame is one complete rendering of all
     * the digits. A field is a slice of that frame. If the digits are
     * multiplexed, then a field is a rendering of a single digit with all its
     * segments. If the segments are multiplexed, then a field is a rendering of
     * a single segment across multiple digits.
     */
    virtual void displayCurrentField() = 0;

    /**
     * Prepare to go to sleep by clearing the frame, and setting a flag so that
     * it doesn't turn itself back on through an interrupt.
     */
    virtual void prepareToSleep() {
      mPreparedToSleep = true;
    }

    /** Wake up from sleep. */
    virtual void wakeFromSleep() {
      mPreparedToSleep = false;
    }

    /** Return number of fields per frame. */
    virtual uint16_t getFieldsPerFrame() = 0;

    /**
     * Returns true if the driver supports brightness. If not, any brightness
     * greater than 0 will be considered ON, and 0 will be OFF.
     */
    virtual bool isBrightnessSupported() = 0;

    /**
     * Set the pattern for a given digit.
     *
     * @param digit the digit index, 0 is the left most digit
     * @param pattern the segment bit pattern
     * @param brightness optional brightness fraction of the digit in units of
     *    1/256, set to 255 if not specified
     */
    void setPattern(uint8_t digit, SegmentPatternType pattern,
        uint8_t brightness = DimmablePattern::kOn);

    /** Set the brightness of the given digit. */
    void setBrightness(uint8_t digit, uint8_t brightness);

  protected:
    /**
     * Number of segments on a single digit. To support a 14-segment LED display
     * or a 16-segment LED display, we would need to change various bit fields
     * from uint8_t to uint16_t. It's a bit of work but doable.
     */
    static const uint8_t kNumSegments = 8;

    // disable copy-constructor and assignment operator
    Driver(const Driver&) = delete;
    Driver& operator=(const Driver&) = delete;

    /**
     * Constructor. The driver takes ownership of the ledMatrix and will delete
     * it in the destructor.
     *
     * @param ledMatrix The ledMatrix can be null for a FakeDriver but normally
     * it is expected to be non-null.
     * @param dimmablePatterns An array of DimmablePattern objects that
     *    represents each digit. This resource is provided by the caller
     * @param numDigits
     */
    explicit Driver(
        LedMatrix* ledMatrix,
        DimmablePattern* dimmablePatterns,
        uint8_t numDigits
    ) :
        mLedMatrix(ledMatrix),
        mDimmablePatterns(dimmablePatterns),
        mNumDigits(numDigits),
        mPreparedToSleep(false)
    {}

    LedMatrix* const mLedMatrix;
    DimmablePattern* const mDimmablePatterns;
    const uint8_t mNumDigits;

    volatile bool mPreparedToSleep;
};

}

#endif
