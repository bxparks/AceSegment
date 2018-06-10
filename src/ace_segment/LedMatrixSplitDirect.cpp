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
#include "LedMatrixSplitDirect.h"

namespace ace_segment {

void LedMatrixSplitDirect::configure() {
  LedMatrix::configure();

  for (uint8_t group = 0; group < mNumGroups; group++) {
    uint8_t digitalPin = mGroupPins[group];
    mHardware->pinMode(digitalPin, OUTPUT);
    mHardware->digitalWrite(digitalPin, mGroupOff);
  }
  for (uint8_t element = 0; element < mNumElements; element++) {
    uint8_t elementPin = mElementPins[element];
    mHardware->pinMode(elementPin, OUTPUT);
    mHardware->digitalWrite(elementPin, mElementOff);
  }
}

void LedMatrixSplitDirect::finish() {
  for (uint8_t group = 0; group < mNumGroups; group++) {
    uint8_t digitalPin = mGroupPins[group];
    mHardware->pinMode(digitalPin, INPUT);
  }
  for (uint8_t element = 0; element < mNumElements; element++) {
    uint8_t elementPin = mElementPins[element];
    mHardware->pinMode(elementPin, INPUT);
  }
}

void LedMatrixSplitDirect::enableGroup(uint8_t group) {
  writeGroupPin(group, mGroupOn);
}

void LedMatrixSplitDirect::disableGroup(uint8_t group) {
  writeGroupPin(group, mGroupOff);
}

void LedMatrixSplitDirect::drawElements(uint8_t pattern) {
  uint8_t elementMask = 0x1;
  for (uint8_t element = 0; element < mNumElements; element++) {
    uint8_t output =
        (pattern & elementMask) ? mElementOn : mElementOff;
    writeElementPin(element, output);
    elementMask <<= 1;
  }
}

}
