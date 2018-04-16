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
  LedMatrix* matrix;
  if (mLedMatrixType == kTypeLedMatrixDirect) {
    if (mResistorsOnSegments) {
      LedMatrixDirect* ledMatrix =
          new LedMatrixDirect(mHardware, mNumDigits, mNumSegments);
      ledMatrix->setGroupPins(mDigitPins);
      ledMatrix->setElementPins(mSegmentPins);
      if (mCommonCathode) {
        ledMatrix->setCathodeOnGroup();
      } else {
        ledMatrix->setAnodeOnGroup();
      }
      matrix = ledMatrix;
    } else {
      LedMatrixDirect* ledMatrix =
          new LedMatrixDirect(mHardware, mNumSegments, mNumDigits);
      ledMatrix->setGroupPins(mSegmentPins);
      ledMatrix->setElementPins(mDigitPins);
      // If the resistors are on the Digit pins, then the "anode" and "cathode"
      // pins become flipped electrically, because we're scanning the LED
      // matrix in the other direction.
      if (mCommonCathode) {
        ledMatrix->setAnodeOnGroup();
      } else {
        ledMatrix->setCathodeOnGroup();
      }
      matrix = ledMatrix;
    }
  } else {
    // We support only resistors on segments for SerialToParallel
    LedMatrixSerial* ledMatrix;
    if (mLedMatrixType == kTypeLedMatrixSerial) {
      ledMatrix = new LedMatrixSerial(mHardware, mNumDigits, mNumSegments);
    } else {
      ledMatrix = new LedMatrixSpi(mHardware, mNumDigits, mNumSegments);
    }
    ledMatrix->setGroupPins(mDigitPins);
    ledMatrix->setElementPins(mLatchPin, mDataPin, mClockPin);
    if (mCommonCathode) {
      ledMatrix->setCathodeOnGroup();
    } else {
      ledMatrix->setAnodeOnGroup();
    }
    matrix = ledMatrix;
  }

  if (mUseTransistors) {
    matrix->invertGroupLevels();
  }
  return matrix;
}

Driver* DriverBuilder::build() {
  LedMatrix* ledMatrix = buildLedMatrix();

  if (mResistorsOnSegments) {
    if (mUseModulatingDriver) {
      return new ModulatingDigitDriver(ledMatrix, mDimmablePatterns,
          mNumDigits, mNumSubFields, true /* ownsLedMatrix */);
    } else {
      return new DigitDriver(ledMatrix, mDimmablePatterns, mNumDigits,
          true /* ownsLedMatrix */);
    }
  } else {
    return new SegmentDriver(ledMatrix, mDimmablePatterns, mNumDigits,
        true /* ownsLedMatrix */);
  }
}

}
