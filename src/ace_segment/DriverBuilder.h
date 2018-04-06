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
      mUseDirect(true),
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

    DriverBuilder& setDigitPins(uint8_t* digitPins) {
      mDigitPins = digitPins;
      return *this;
    }

    DriverBuilder& setSegmentDirectPins(uint8_t* segmentPins) {
      mUseDirect = true;
      mSegmentPins = segmentPins;
      return *this;
    }

    DriverBuilder& setSegmentSerialPins(uint8_t latchPin, uint8_t dataPin,
        uint8_t clockPin) {
      mUseDirect = false;
      // We support only resistors on segments in this configuration.
      mResistorsOnSegments = true;
      mSegmentPins = nullptr;

      mLatchPin = latchPin;
      mDataPin = dataPin;
      mClockPin = clockPin;
      return *this;
    }

    /** The LedMatrix object returned must have its configure() called. */
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
    bool mUseDirect;
    uint8_t* mDigitPins;
    uint8_t* mSegmentPins;
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
