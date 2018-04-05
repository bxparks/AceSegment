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

#ifndef ACE_SEGMENT_DIRECT_DRIVER_H
#define ACE_SEGMENT_DIRECT_DRIVER_H

#include <stdint.h>
#include "DimmingDigit.h"
#include "Driver.h"

namespace ace_segment {

class Hardware;

/**
 * A version of Driver where the LED segment and digit pins are connected
 * directly to the microcontroller. This is the simplest and cheapest but
 * requires the most number of available pins on the microcontroller.
 */
class DirectDriver: public Driver {
  public:

    // Start configuration methods

    /**
     * Set the pins of the digits, assuming mNumDigits number of StyledDigits.
     * Digit 0 is on the left. Required.
     */
    void setDigitPins(const uint8_t* pins) {
      mDigitPins = pins;
    }

    /**
     * Set the pins of the segments, assuming kNumSegments number of segments.
     * Standard 7-segment bit mapping: hgfedcba -> 76543210 (h = decimal
     * point). Required.
     */
    void setSegmentPins(const uint8_t* pins) {
      mSegmentPins = pins;
    }

    virtual void configure();

    // End configuration methods

    /** Write to digit pin identified by 'digit'. VisibleForTesting. */
    void writeDigitPin(uint8_t digit, uint8_t output);

    /** Write to the segment pin identified by 'segment'. VisibleForTesting. */
    void writeSegmentPin(uint8_t segment, uint8_t output);

  protected:
    // disable copy-constructor and assignment operator
    DirectDriver(const DirectDriver&) = delete;
    DirectDriver& operator=(const DirectDriver&) = delete;

    /** Constructor. */
    explicit DirectDriver(Hardware* hardware, DimmingDigit* dimmingDigits,
            uint8_t numDigits):
        Driver(hardware, dimmingDigits, numDigits)
    {}

    const uint8_t* mDigitPins;
    const uint8_t* mSegmentPins;
};

}

#endif
