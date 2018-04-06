#ifndef ACE_SEGMENT_LED_MATRIX_H
#define ACE_SEGMENT_LED_MATRIX_H

#include <Arduino.h> // LOW and HIGH

namespace ace_segment {

class Hardware;

class LedMatrix {
  public:
    LedMatrix(Hardware* hardware, uint8_t numGroups, uint8_t numElements):
        mHardware(hardware),
        mNumGroups(numGroups),
        mNumElements(numElements)
    {}

    /** LED negative terminals are on the group line. Required. */
    void setCathodeOnGroup() {
      mGroupOn = LOW;
      mGroupOff = HIGH;
      mElementOn = HIGH;
      mElementOff = LOW;
    }

    /** LED positive terminals are on the group line. Required. */
    void setAnodeOnGroup() {
      mGroupOn = HIGH;
      mGroupOff = LOW;
      mElementOn = LOW;
      mElementOff = HIGH;
    }

    virtual void configure() {}

    virtual void enableGroup(uint8_t group) = 0;

    virtual void disableGroup(uint8_t group) = 0;

    virtual void drawElements(uint8_t pattern) = 0;

  protected:
    Hardware* const mHardware;
    const uint8_t mNumGroups;
    const uint8_t mNumElements;

    uint8_t mGroupOn;
    uint8_t mGroupOff;
    uint8_t mElementOn;
    uint8_t mElementOff;
};

}

#endif
