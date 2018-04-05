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

#ifndef ACE_SEGMENT_SERIAL_TO_PARALLEL_DRIVER_H
#define ACE_SEGMENT_SERIAL_TO_PARALLEL_DRIVER_H

#include <stdint.h>
#include "Driver.h"

namespace ace_segment {

class DimmingDigit;
class Hardware;

class SerialToParallelDriver: public Driver {
  public:
    /** Constructor. */
    explicit SerialToParallelDriver(Hardware* hardware,
            DimmingDigit* dimmingDigits, int8_t numDigits):
        Driver(hardware, dimmingDigits, numDigits)
    {}

    // Start configuration methods

    /** Required. */
    void setLatchPin(uint8_t pin) { mLatchPin = pin; }

    /** Required. */
    void setClockPin(uint8_t pin) { mClockPin = pin; }

    /** Required. */
    void setDataPin(uint8_t pin) { mDataPin = pin; }

    /**
     * Set the pins of the digits, assuming mNumDigits number of StyledDigits.
     * Digit 0 is on the left. Required.
     */
    void setDigitPins(const uint8_t* pins) { mDigitPins = pins; }

    virtual void configure() override {
      Driver::configure();

      mHardware->setPinMode(mLatchPin, OUTPUT);
      mHardware->setPinMode(mDataPin, OUTPUT);
      mHardware->setPinMode(mClockPin, OUTPUT);
      // TODO: Do I need to do anything to display the initial values?

      for (uint8_t digit = 0; digit < mNumDigits; digit++) {
        uint8_t digitalPin = mDigitPins[digit];
        mHardware->setPinMode(digitalPin, OUTPUT);
        mHardware->writePin(digitalPin, mDigitOff);
      }

      mCurrentDigit = 0;
      mPrevDigit = mNumDigits - 1;
      mSegmentPattern = 0;
    }

    // End configuration methods

    virtual uint16_t getFieldsPerFrame() override { return mNumDigits; }

    virtual bool isBrightnessSupported() override { return false; }

    virtual void displayCurrentField() override;

    /** Write to digit pin identified by 'digit'. VisibleForTesting. */
    void writeDigitPin(uint8_t digit, uint8_t output);

  protected:
    // disable copy-constructor and assignment operator
    SerialToParallelDriver(const SerialToParallelDriver&) = delete;
    SerialToParallelDriver& operator=(const SerialToParallelDriver&) = delete;

    /** Draw the specified segments. */
    void drawSegments(SegmentPatternType segmentPattern);

    const uint8_t* mDigitPins;
    uint8_t mLatchPin;
    uint8_t mClockPin;
    uint8_t mDataPin;

    uint8_t mCurrentDigit;
    uint8_t mPrevDigit;
    bool mIsCurrentDigitOn; // whether the current digit is on or off
    SegmentPatternType mSegmentPattern;
};

}

#endif
