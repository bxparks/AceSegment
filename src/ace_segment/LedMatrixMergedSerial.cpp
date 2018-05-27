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
#include "LedMatrixMergedSerial.h"

namespace ace_segment {

void LedMatrixMergedSerial::configure() {
  LedMatrix::configure();

  // Actual values of latchPin, dataPin and clockPin don't matter because the
  // functions are triggered on a rising edge, not on the level.
  mHardware->pinMode(mLatchPin, OUTPUT);
  mHardware->pinMode(mDataPin, OUTPUT);
  mHardware->pinMode(mClockPin, OUTPUT);
}

void LedMatrixMergedSerial::finish() {
  mHardware->pinMode(mLatchPin, INPUT);
  mHardware->pinMode(mDataPin, INPUT);
  mHardware->pinMode(mClockPin, INPUT);
}

void LedMatrixMergedSerial::draw(uint8_t groupPattern, uint8_t elementPattern) {
  mHardware->digitalWrite(mLatchPin, LOW);
  uint8_t actualElementPattern = (mElementOn == HIGH)
      ? elementPattern : ~elementPattern;
  uint8_t actualGroupPattern = (mGroupOn == HIGH)
      ? groupPattern : ~groupPattern;
  mHardware->shiftOut(mDataPin, mClockPin, MSBFIRST, actualGroupPattern);
  mHardware->shiftOut(mDataPin, mClockPin, MSBFIRST, actualElementPattern);
  mHardware->digitalWrite(mLatchPin, HIGH);
}

}
