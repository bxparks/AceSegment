#ifndef ACE_SEGMENT_LED_MATRIX_DIRECT_H
#define ACE_SEGMENT_LED_MATRIX_DIRECT_H

#include "Hardware.h"
#include "LedMatrix.h"

namespace ace_segment {

class LedMatrixDirect: public LedMatrix {
  public:
    LedMatrixDirect(Hardware* hardware, uint8_t numGroups, uint8_t numElements):
        LedMatrix(hardware, numGroups, numElements)
    {}

    void setGroupPins(uint8_t* groupPins) {
      mGroupPins = groupPins;
    }

    void setElementPins(uint8_t* elementPins) {
      mElementPins = elementPins;
    }

    virtual void configure() override {
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

    virtual void enableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOn);
    }

    virtual void disableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOff);
    }

    virtual void drawElements(uint8_t pattern) override {
      uint8_t elementMask = 0x1;
      for (uint8_t element = 0; element < mNumElements; element++) {
        uint8_t output =
            (pattern & elementMask) ? mElementOn : mElementOff;
        writeElementPin(element, output);
        elementMask <<= 1;
      }
    }

  private:
    /** Write to group pin identified by 'group'. VisibleForTesting. */
    void writeGroupPin(uint8_t group, uint8_t output) {
      uint8_t groupPin = mGroupPins[group];
      mHardware->digitalWrite(groupPin, output);
    }

    /** Write to the element pin identified by 'element'. VisibleForTesting. */
    void writeElementPin(uint8_t element, uint8_t output) {
      uint8_t elementPin = mElementPins[element];
      mHardware->digitalWrite(elementPin, output);
    }

    uint8_t* mGroupPins;
    uint8_t* mElementPins;
};

}

#endif
