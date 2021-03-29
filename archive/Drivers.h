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

#ifndef ACE_SEGMENT_DRIVERS_H
#define ACE_SEGMENT_DRIVERS_H

#include "LedMatrixDirect.h"
#include "LedMatrixPartialSpi.h"
#include "LedMatrixFullSpi.h"
#include "Driver.h"
#include "SplitDigitDriver.h"
#include "MergedDigitDriver.h"
#include "SwSpiAdapter.h"
#include "HwSpiAdapter.h"

namespace ace_segment {

/**
 * A driver that uses a GPIO pin to drive each digit and segment pins of
 * the LED display. The resistors are assumed to be on the segments and can be
 * driven at the same time, so the digits are multiplexed.
 */
class SplitDirectDigitDriver : public SplitDigitDriver {

  public:
    SplitDirectDigitDriver(
        Hardware* hardware,
        DimmablePattern* dimmablePatterns,
        bool commonCathode,
        bool transistorsOnDigits,
        bool transistorsOnSegments,
        uint8_t numDigits,
        uint8_t numSegments,
        uint8_t numSubFields,
        const uint8_t* digitPins,
        const uint8_t* segmentPins
    ) :
        SplitDigitDriver(
            &mLedMatrix, dimmablePatterns, numDigits, numSubFields),
        mLedMatrix(
            hardware,
            commonCathode,
            transistorsOnDigits,
            transistorsOnSegments,
            numDigits,
            numSegments,
            digitPins,
            segmentPins)
    {}

  private:
    LedMatrixDirect mLedMatrix;
};

/**
 * A driver which uses an 74HC595 serial-to-parallel converter on the segment
 * pins which have resistors, so that they can driven at the same time. The
 * 74HC595 is programmed using the shiftOut() method on the Arduino. The digit
 * pins are driven directly by the GPIO pins.
 */
class SplitSerialDigitDriver : public SplitDigitDriver {

  public:
    SplitSerialDigitDriver(
        Hardware* hardware,
        DimmablePattern* dimmablePatterns,
        bool commonCathode,
        bool transistorsOnDigits,
        bool transistorsOnSegments,
        uint8_t numDigits,
        uint8_t numSegments,
        uint8_t numSubFields,
        const uint8_t* digitPins,
        const uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        SplitDigitDriver(
            &mLedMatrix, dimmablePatterns, numDigits, numSubFields),
        mLedMatrix(
            hardware,
            &mSpiAdapter,
            commonCathode,
            transistorsOnDigits,
            transistorsOnSegments,
            numDigits,
            numSegments,
            digitPins),
        mSpiAdapter(latchPin, dataPin, clockPin)
    {}

  private:
    LedMatrixPartialSpi mLedMatrix;
    SwSpiAdapter mSpiAdapter;
};

/**
 * A driver which uses an 74HC595 serial-to-parallel converter on the segment
 * pins which have resistors, so that they can driven at the same time. The
 * 74HC595 is programmed using hardware SPI on the Arduino. The digit pins are
 * driven directly by the GPIO pins.
 */
class SplitSpiDigitDriver : public SplitDigitDriver {

  public:
    SplitSpiDigitDriver(
        Hardware* hardware,
        DimmablePattern* dimmablePatterns,
        bool commonCathode,
        bool transistorsOnDigits,
        bool transistorsOnSegments,
        uint8_t numDigits,
        uint8_t numSegments,
        uint8_t numSubFields,
        const uint8_t* digitPins,
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        SplitDigitDriver(
            &mLedMatrix, dimmablePatterns, numDigits, numSubFields),
        mLedMatrix(
            hardware,
            &mSpiAdapter,
            commonCathode,
            transistorsOnDigits,
            transistorsOnSegments,
            numDigits,
            numSegments,
            digitPins),
        mSpiAdapter(latchPin, dataPin, clockPin)

    {}

  private:
    LedMatrixPartialSpi mLedMatrix;
    HwSpiAdapter mSpiAdapter;
};

/**
 * A driver which uses 2 74HC595 chips on both the segment and digit pins.
 * The segments are assumed to have the resistors so can be driven at the same
 * time. The digits are multiplexed. The 74HC595 chipes are programmed using
 * the shiftOut() method.
 */
class MergedSerialDigitDriver : public MergedDigitDriver {

  public:
    MergedSerialDigitDriver(
        DimmablePattern* dimmablePatterns,
        bool commonCathode,
        bool transistorsOnDigits,
        bool transistorsOnSegments,
        uint8_t numDigits,
        uint8_t numSegments,
        uint8_t numSubFields,
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        MergedDigitDriver(
            &mLedMatrix, dimmablePatterns, numDigits, numSubFields),
        mLedMatrix(
            &mSpiAdapter,
            commonCathode,
            transistorsOnDigits,
            transistorsOnSegments,
            numDigits,
            numSegments),
        mSpiAdapter(latchPin, dataPin, clockPin)
    {}

  private:
    LedMatrixFullSpi mLedMatrix;
    SwSpiAdapter mSpiAdapter;
};

/**
 * A driver which uses 2 74HC595 chips on both the segment and digit pins.
 * The segments are assumed to have the resistors so can be driven at the same
 * time. The digits are multiplexed. The 74HC595 chipes are programmed using
 * hardware SPI.
 */
class MergedSpiDigitDriver : public MergedDigitDriver{

  public:
    MergedSpiDigitDriver(
        DimmablePattern* dimmablePatterns,
        bool commonCathode,
        bool transistorsOnDigits,
        bool transistorsOnSegments,
        uint8_t numDigits,
        uint8_t numSegments,
        uint8_t numSubFields,
        uint8_t latchPin,
        uint8_t dataPin,
        uint8_t clockPin
    ) :
        MergedDigitDriver(
            &mLedMatrix, dimmablePatterns, numDigits, numSubFields),
        mLedMatrix(
            &mSpiAdapter,
            commonCathode,
            transistorsOnDigits,
            transistorsOnSegments,
            numDigits,
            numSegments),
        mSpiAdapter(latchPin, dataPin, clockPin)
    {}

  private:
    LedMatrixFullSpi mLedMatrix;
    HwSpiAdapter mSpiAdapter;
};

}

#endif
