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
    DriverBuilder(Hardware* hardware):
      mHardware(hardware)
    {}

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

    DriverBuilder& useTransistorDrivers() {
      mUseTransistorDrivers = true;
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
      mLedMatrixType = kTypeLedMatrixDirect;
      mSegmentPins = segmentPins;
      return *this;
    }

    DriverBuilder& setSegmentSerialPins(uint8_t latchPin, uint8_t dataPin,
        uint8_t clockPin) {
      mLedMatrixType = kTypeLedMatrixSerial;

      // We support only resistors on segments in this configuration.
      mResistorsOnSegments = true;
      mSegmentPins = nullptr;

      mLatchPin = latchPin;
      mDataPin = dataPin;
      mClockPin = clockPin;
      return *this;
    }

    DriverBuilder& setSegmentSpiPins(uint8_t latchPin, uint8_t dataPin,
        uint8_t clockPin) {
      mLedMatrixType = kTypeLedMatrixSpi;

      // We support only resistors on segments in this configuration.
      mResistorsOnSegments = true;
      mSegmentPins = nullptr;

      mLatchPin = latchPin;
      mDataPin = dataPin;
      mClockPin = clockPin;
      return *this;
    }

    DriverBuilder& setDimmablePatterns(DimmablePattern* dimmablePatterns) {
      mDimmablePatterns = dimmablePatterns;
      return *this;
    }

    /**
     * Use a driver that provides pulse width modulation.
     *
     * @param numSubFields number subfields per field, 16 seems to be a good
     * reasonable. A minimum of 1 is imposed if set to 0.
     */
    DriverBuilder& useModulatingDriver(uint8_t numSubFields) {
      mUseModulatingDriver = true;
      mNumSubFields = (numSubFields > 0) ? numSubFields : 1;
      return *this;
    }

    Driver* build();

  private:
    static const uint8_t kTypeLedMatrixDirect = 0;
    static const uint8_t kTypeLedMatrixSerial = 1;
    static const uint8_t kTypeLedMatrixSpi = 2;

    LedMatrix* buildLedMatrix();

    // parameters for LedMatrix
    Hardware* const mHardware;
    uint8_t mNumDigits = 2;
    uint8_t mNumSegments = 8;
    bool mResistorsOnSegments = true;
    bool mCommonCathode = true;
    bool mUseTransistorDrivers = false;
    uint8_t mLedMatrixType = kTypeLedMatrixDirect;
    const uint8_t* mDigitPins = nullptr;
    const uint8_t* mSegmentPins = nullptr;
    uint8_t mLatchPin = 0;
    uint8_t mDataPin = 0;
    uint8_t mClockPin = 0;

    // parameters for Driver
    DimmablePattern* mDimmablePatterns = nullptr;
    bool mUseModulatingDriver = false;
    uint8_t mNumSubFields = 16;
};

}

#endif
