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

#include "LedMatrixSplitDirect.h"
#include "LedMatrixMergedSerial.h"
#include "LedMatrixMergedSpi.h"
#include "LedMatrixSplitSerial.h"
#include "LedMatrixSplitSpi.h"
#include "Driver.h"
#include "DigitDriver.h"
#include "SegmentDriver.h"
#include "MergedDigitDriver.h"

namespace ace_segment {

class Wrapper {
  public:
    virtual Driver* getDriver() = 0;
};

class DirectDigitWrapper: public Wrapper {
  public:
    DirectDigitWrapper(Hardware* hardware, 
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments, uint8_t numSubFields,
            const uint8_t* digitPins, const uint8_t* segmentPins):
        mLedMatrix(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments, digitPins,
            segmentPins),
        mDriver(&mLedMatrix, dimmablePatterns, numDigits, numSubFields)
    {}

    virtual Driver* getDriver() override { return &mDriver; }

  private:
    LedMatrixSplitDirect mLedMatrix;
    DigitDriver mDriver;
};

class DirectSegmentWrapper: public Wrapper {
  public:
    DirectSegmentWrapper(Hardware* hardware, 
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments, uint8_t numSubFields,
            const uint8_t* digitPins, const uint8_t* segmentPins):
        mLedMatrix(hardware, !commonCathode, transistorsOnSegments,
            transistorsOnDigits, numSegments, numDigits, segmentPins,
            digitPins),
        mDriver(&mLedMatrix, dimmablePatterns, numDigits, numSubFields)
    {}

    virtual Driver* getDriver() override { return &mDriver; }

  private:
    LedMatrixSplitDirect mLedMatrix;
    SegmentDriver mDriver;
};

class SplitSerialDigitWrapper: public Wrapper {
  public:
    SplitSerialDigitWrapper(Hardware* hardware, 
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments, uint8_t numSubFields,
            const uint8_t* digitPins, const uint8_t latchPin,
            uint8_t dataPin, uint8_t clockPin):
        mLedMatrix(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments, digitPins,
            latchPin, dataPin, clockPin),
        mDriver(&mLedMatrix, dimmablePatterns, numDigits, numSubFields)
    {}

    virtual Driver* getDriver() override { return &mDriver; }

  private:
    LedMatrixSplitSerial mLedMatrix;
    DigitDriver mDriver;
};

class SplitSpiDigitWrapper: public Wrapper {
  public:
    SplitSpiDigitWrapper(Hardware* hardware, 
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments, uint8_t numSubFields,
            const uint8_t* digitPins, uint8_t latchPin,
            uint8_t dataPin, uint8_t clockPin):
        mLedMatrix(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments, digitPins,
            latchPin, dataPin, clockPin),
        mDriver(&mLedMatrix, dimmablePatterns, numDigits, numSubFields)
    {}

    virtual Driver* getDriver() override { return &mDriver; }

  private:
    LedMatrixSplitSpi mLedMatrix;
    DigitDriver mDriver;
};

class MergedSerialDigitWrapper: public Wrapper {
  public:
    MergedSerialDigitWrapper(Hardware* hardware, 
            DimmablePattern* dimmablePatterns,
            bool commonCathode, bool transistorsOnDigits,
            bool transistorsOnSegments, uint8_t numDigits, uint8_t numSegments,
            uint8_t numSubFields,
            uint8_t latchPin, uint8_t dataPin, uint8_t clockPin):
        mLedMatrix(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments,
            latchPin, dataPin, clockPin),
        mDriver(&mLedMatrix, dimmablePatterns, numDigits, numSubFields)
    {}

    virtual Driver* getDriver() override { return &mDriver; }

  private:
    LedMatrixMergedSerial mLedMatrix;
    MergedDigitDriver mDriver;
};

class MergedSpiDigitWrapper: public Wrapper {
  public:
    MergedSpiDigitWrapper(Hardware* hardware, 
            DimmablePattern* dimmablePatterns,
            bool commonCathode, bool transistorsOnDigits,
            bool transistorsOnSegments, uint8_t numDigits, uint8_t numSegments,
            uint8_t numSubFields,
            uint8_t latchPin, uint8_t dataPin, uint8_t clockPin):
        mLedMatrix(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments,
            latchPin, dataPin, clockPin),
        mDriver(&mLedMatrix, dimmablePatterns, numDigits, numSubFields)
    {}

    virtual Driver* getDriver() override { return &mDriver; }

  private:
    LedMatrixMergedSpi mLedMatrix;
    MergedDigitDriver mDriver;
};

}

#endif
