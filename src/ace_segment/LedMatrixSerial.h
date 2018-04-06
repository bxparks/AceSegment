#ifndef ACE_SEGMENT_LED_MATRIX_SERIAL_H
#define ACE_SEGMENT_LED_MATRIX_SERIAL_H

#include "Hardware.h"

namespace ace_segment {

class LedMatrixSerial: public LedMatrix {
  public:
    LedMatrixSerial(Hardware* hardware, uint8_t numGroups, uint8_t numElements):
        LedMatrix(hardware, numGroups, numElements)
    {}

    void setGroupPins(uint8_t* groupPins) {
      mGroupPins = groupPins;
    }

    void setElementPins(uint8_t latchPin, uint8_t dataPin, uint8_t clockPin) {
      mLatchPin = latchPin;
      mDataPin = dataPin;
      mClockPin = clockPin;
    }

    virtual void configure() override {
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

    virtual void enableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOn);
    }

    virtual void disableGroup(uint8_t group) override {
      writeGroupPin(group, mGroupOff);
    }

    virtual void drawElements(uint8_t pattern) override {
      mHardware->digitalWrite(mLatchPin, LOW);
      uint8_t actualPattern = (mElementOn == HIGH) ? pattern : ~pattern;
      mHardware->shiftOut(mDataPin, mClockPin, MSBFIRST, actualPattern);
      mHardware->digitalWrite(mLatchPin, HIGH);
    }

  private:
    /** Write to group pin identified by 'group'. VisibleForTesting. */
    void writeGroupPin(uint8_t group, uint8_t output) {
      uint8_t groupPin = mGroupPins[group];
      mHardware->digitalWrite(groupPin, output);
    }

    uint8_t* mGroupPins;
    uint8_t mLatchPin;
    uint8_t mDataPin;
    uint8_t mClockPin;
};

}
#endif
