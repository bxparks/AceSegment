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
#include "LedMatrixSerial.h"

namespace ace_segment {

void LedMatrixSerial::configure() {
  LedMatrix::configure();

  // TODO: Do I need to set the initial values of the 74HC595?
  mHardware->pinMode(mLatchPin, OUTPUT);
  mHardware->pinMode(mDataPin, OUTPUT);
  mHardware->pinMode(mClockPin, OUTPUT);

  for (uint8_t group = 0; group < mNumGroups; group++) {
    uint8_t pin = mGroupPins[group];
    mHardware->pinMode(pin, OUTPUT);
    mHardware->digitalWrite(pin, mGroupOff);
  }
}

void LedMatrixSerial::enableGroup(uint8_t group) {
  writeGroupPin(group, mGroupOn);
}

void LedMatrixSerial::disableGroup(uint8_t group) {
  writeGroupPin(group, mGroupOff);
}

void LedMatrixSerial::drawElements(uint8_t pattern) {
  mHardware->digitalWrite(mLatchPin, LOW);
  uint8_t actualPattern = (mElementOn == HIGH) ? pattern : ~pattern;
  mHardware->shiftOut(mDataPin, mClockPin, MSBFIRST, actualPattern);
  mHardware->digitalWrite(mLatchPin, HIGH);
}

void LedMatrixSerial::writeGroupPin(uint8_t group, uint8_t output) {
  uint8_t groupPin = mGroupPins[group];
  mHardware->digitalWrite(groupPin, output);
}

}
