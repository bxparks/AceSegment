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

#include "Hardware.h"
#include "LedMatrix.h"
#include "LedMatrixDirect.h"
#include "LedMatrixSerial.h"
#include "LedMatrixSpi.h"
#include "Driver.h"
#include "DigitDriver.h"
#include "ModulatingDigitDriver.h"
#include "SegmentDriver.h"
#include "DriverBuilder.h"

namespace ace_segment {

/** The LedMatrix object returned must have its configure() called. */
LedMatrix* DriverBuilder::buildLedMatrix() {
  if (mLedMatrixType == kTypeLedMatrixDirect) {
    if (mResistorsOnSegments) {
      Serial.println(F("DriverBuilder(): creating LedMatrixDirect w/ resistorsOnSegments"));
      LedMatrixDirect* ledMatrix =
          new LedMatrixDirect(mHardware, mNumDigits, mNumSegments);
      ledMatrix->setGroupPins(mDigitPins);
      ledMatrix->setElementPins(mSegmentPins);
      if (mCommonCathode) {
        ledMatrix->setCathodeOnGroup();
      } else {
        ledMatrix->setAnodeOnGroup();
      }
      return ledMatrix;
    } else {
      Serial.println(F("DriverBuilder(): creating LedMatrixDirect w/ resistorsOnDigits"));
      LedMatrixDirect* ledMatrix =
          new LedMatrixDirect(mHardware, mNumSegments, mNumDigits);
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
    LedMatrixSerial* ledMatrix;
    if (mLedMatrixType == kTypeLedMatrixSerial) {
      Serial.println(F("DriverBuilder(): creating LedMatrixSerial w/ resistorsOnSegments"));
      ledMatrix = new LedMatrixSerial(mHardware, mNumDigits, mNumSegments);
    } else {
      Serial.println(F("DriverBuilder(): creating LedMatrixSpi w/ resistorsOnSegments"));
      ledMatrix = new LedMatrixSpi(mHardware, mNumDigits, mNumSegments);
    }
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
      Serial.println(F("DriverBuilder(): creating ModulatingDigitDriver"));
      return new ModulatingDigitDriver(ledMatrix, mDimmingDigits,
          mNumDigits, mNumSubFields);
    } else {
      Serial.println(F("DriverBuilder(): creating DigitDriver"));
      return new DigitDriver(ledMatrix, mDimmingDigits, mNumDigits);
    }
  } else {
    Serial.println(F("DriverBuilder(): creating SegmentDriver"));
    return new SegmentDriver(ledMatrix, mDimmingDigits, mNumDigits);
  }
}

}
