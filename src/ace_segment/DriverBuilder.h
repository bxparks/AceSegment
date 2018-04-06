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

#ifndef ACE_SEGMENT_DRIVER_BUILDER_H
#define ACE_SEGMENT_DRIVER_BUILDER_H

#include "Hardware.h"
#include "LedMatrix.h"
#include "Driver.h"

namespace ace_segment {

class Hardware;
class LedMatrix;
class Driver;

class DriverBuilder {
  public:
    DriverBuilder():
      mNumSegments(8),
      mResistorsOnSegments(true),
      mCommonCathode(true),
      mUseSerialToParallel(false),
      mUseSpi(false),
      mDigitPins(nullptr),
      mSegmentPins(nullptr),
      mUseModulatingDriver(false),
      mNumSubFields(1)
    {}

    DriverBuilder& setHardware(Hardware* hardware) {
      mHardware = hardware;
      return *this;
    }

    DriverBuilder& setNumDigits(uint8_t numDigits) {
      mNumDigits = numDigits;
      return *this;
    }

    DriverBuilder& setNumSegments(uint8_t numSegments) {
      mNumSegments = numSegments;
      return *this;
    }

    DriverBuilder& setCommonAnode() {
      mCommonCathode = false;
      return *this;
    }

    DriverBuilder& setCommonCathode() {
      mCommonCathode = true;
      return *this;
    }

    DriverBuilder& setResistorsOnDigits() {
      mResistorsOnSegments = false;
      return *this;
    }

    DriverBuilder& setResistorsOnSegments() {
      mResistorsOnSegments = true;
      return *this;
    }

    DriverBuilder& setDigitPins(const uint8_t* digitPins) {
      mDigitPins = digitPins;
      return *this;
    }

    DriverBuilder& setSegmentDirectPins(const uint8_t* segmentPins) {
      mUseSerialToParallel = false;
      mSegmentPins = segmentPins;
      return *this;
    }

    DriverBuilder& setSegmentSerialPins(uint8_t latchPin, uint8_t dataPin,
        uint8_t clockPin) {
      mUseSerialToParallel = true;
      // We support only resistors on segments in this configuration.
      mResistorsOnSegments = true;
      mSegmentPins = nullptr;

      mLatchPin = latchPin;
      mDataPin = dataPin;
      mClockPin = clockPin;
      return *this;
    }

    DriverBuilder& useSpi() {
      mUseSpi = true;
      return *this;
    }

    LedMatrix* buildLedMatrix();

    DriverBuilder& setDimmingDigits(DimmingDigit* dimmingDigits) {
      mDimmingDigits = dimmingDigits;
      return *this;
    }

    DriverBuilder& useModulatingDriver() {
      mUseModulatingDriver = true;
      return *this;
    }

    /** Used only if useModulatingDriver() is true. */
    DriverBuilder& setNumSubFields(uint8_t numSubFields) {
      mNumSubFields = numSubFields;
      return *this;
    }

    Driver* build();

  private:
    // parameters for LedMatrix
    Hardware* mHardware;
    uint8_t mNumDigits;
    uint8_t mNumSegments;
    bool mResistorsOnSegments;
    bool mCommonCathode;
    bool mUseSerialToParallel;
    bool mUseSpi;
    const uint8_t* mDigitPins;
    const uint8_t* mSegmentPins;
    uint8_t mLatchPin;
    uint8_t mDataPin;
    uint8_t mClockPin;

    // parameters for Driver
    DimmingDigit* mDimmingDigits;
    bool mUseModulatingDriver;
    uint8_t mNumSubFields;
};

}

#endif
