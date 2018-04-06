#include "Hardware.h"
#include "LedMatrix.h"
#include "LedMatrixDirect.h"
#include "LedMatrixSerial.h"
#include "Driver.h"
#include "DigitDriver.h"
#include "ModulatingDigitDriver.h"
#include "SegmentDriver.h"
#include "DriverBuilder.h"

namespace ace_segment {

/** The LedMatrix object returned must have its configure() called. */
LedMatrix* DriverBuilder::buildLedMatrix() {
  if (mUseDirect) {
    if (mResistorsOnSegments) {
      LedMatrixDirect* ledMatrix = new LedMatrixDirect(
          mHardware, mNumDigits, mNumSegments);
      ledMatrix->setGroupPins(mDigitPins);
      ledMatrix->setElementPins(mSegmentPins);
      if (mCommonCathode) {
        ledMatrix->setCathodeOnGroup();
      } else {
        ledMatrix->setAnodeOnGroup();
      }
      return ledMatrix;
    } else {
      LedMatrixDirect* ledMatrix = new LedMatrixDirect(
          mHardware, mNumSegments, mNumDigits);
      ledMatrix->setGroupPins(mSegmentPins);
      ledMatrix->setElementPins(mDigitPins);
      if (mCommonCathode) {
        ledMatrix->setAnodeOnGroup();
      } else {
        ledMatrix->setCathodeOnGroup();
      }
      return ledMatrix;
    }
  } else {
    // We support only resistors on segments for SerialToParallel
    LedMatrixSerial* ledMatrix = new LedMatrixSerial(
        mHardware, mNumDigits, mNumSegments);
    ledMatrix->setGroupPins(mDigitPins);
    ledMatrix->setElementPins(mLatchPin, mDataPin, mClockPin);
    if (mCommonCathode) {
      ledMatrix->setCathodeOnGroup();
    } else {
      ledMatrix->setAnodeOnGroup();
    }
    return ledMatrix;
  }
}

Driver* DriverBuilder::build() {
  LedMatrix* ledMatrix = buildLedMatrix();
  if (mResistorsOnSegments) {
    if (mUseModulatingDriver) {
      return new ModulatingDigitDriver(ledMatrix, mDimmingDigits,
          mNumDigits, mNumSubFields);
    } else {
      return new DigitDriver(ledMatrix, mDimmingDigits, mNumDigits);
    }
  } else {
    return new SegmentDriver(ledMatrix, mDimmingDigits, mNumDigits);
  }
}

}
