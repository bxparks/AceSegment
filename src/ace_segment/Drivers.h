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
#include "SplitDigitDriver.h"
#include "SplitSegmentDriver.h"
#include "MergedDigitDriver.h"

namespace ace_segment {

class SplitDirectDigitDriver:
    private LedMatrixSplitDirect,
    public SplitDigitDriver {

  public:
    SplitDirectDigitDriver(Hardware* hardware,
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments, uint8_t numSubFields,
            const uint8_t* digitPins, const uint8_t* segmentPins):
        LedMatrixSplitDirect(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments, digitPins,
            segmentPins),
        SplitDigitDriver(this, dimmablePatterns, numDigits, numSubFields)
    {}
};

class SplitDirectSegmentDriver:
    private LedMatrixSplitDirect,
    public SplitSegmentDriver {

  public:
    SplitDirectSegmentDriver(Hardware* hardware,
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments,
            const uint8_t* digitPins, const uint8_t* segmentPins):
        LedMatrixSplitDirect(hardware, !commonCathode, transistorsOnSegments,
            transistorsOnDigits, numSegments, numDigits, segmentPins,
            digitPins),
        SplitSegmentDriver(this, dimmablePatterns, numDigits)
    {}
};

class SplitSerialDigitDriver:
    private LedMatrixSplitSerial,
    public SplitDigitDriver {

  public:
    SplitSerialDigitDriver(Hardware* hardware,
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments, uint8_t numSubFields,
            const uint8_t* digitPins, const uint8_t latchPin,
            uint8_t dataPin, uint8_t clockPin):
        LedMatrixSplitSerial(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments, digitPins,
            latchPin, dataPin, clockPin),
        SplitDigitDriver(this, dimmablePatterns, numDigits, numSubFields)
    {}
};

class SplitSpiDigitDriver:
    private LedMatrixSplitSpi,
    public SplitDigitDriver {

  public:
    SplitSpiDigitDriver(Hardware* hardware,
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments, uint8_t numSubFields,
            const uint8_t* digitPins, uint8_t latchPin,
            uint8_t dataPin, uint8_t clockPin):
        LedMatrixSplitSpi(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments, digitPins,
            latchPin, dataPin, clockPin),
        SplitDigitDriver(this, dimmablePatterns, numDigits, numSubFields)
    {}
};

class MergedSerialDigitDriver:
    private LedMatrixMergedSerial,
    public MergedDigitDriver {

  public:
    MergedSerialDigitDriver(Hardware* hardware,
            DimmablePattern* dimmablePatterns,
            bool commonCathode, bool transistorsOnDigits,
            bool transistorsOnSegments, uint8_t numDigits, uint8_t numSegments,
            uint8_t numSubFields,
            uint8_t latchPin, uint8_t dataPin, uint8_t clockPin):
        LedMatrixMergedSerial(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments,
            latchPin, dataPin, clockPin),
        MergedDigitDriver(this, dimmablePatterns, numDigits, numSubFields)
    {}
};

class MergedSpiDigitDriver:
    private LedMatrixMergedSpi,
    public MergedDigitDriver{

  public:
    MergedSpiDigitDriver(Hardware* hardware,
            DimmablePattern* dimmablePatterns,
            bool commonCathode, bool transistorsOnDigits,
            bool transistorsOnSegments, uint8_t numDigits, uint8_t numSegments,
            uint8_t numSubFields,
            uint8_t latchPin, uint8_t dataPin, uint8_t clockPin):
        LedMatrixMergedSpi(hardware, commonCathode, transistorsOnDigits,
            transistorsOnSegments, numDigits, numSegments,
            latchPin, dataPin, clockPin),
        MergedDigitDriver(this, dimmablePatterns, numDigits, numSubFields)
    {}
};

}

#endif
