#ifndef ACE_SEGMENT_BLINK_STYLER_H
#define ACE_SEGMENT_BLINK_STYLER_H

#include <stdint.h>
#include "Styler.h"

namespace ace_segment {

class BlinkStyler: public Styler{
  public:
    BlinkStyler(uint8_t framesPerSecond, uint16_t durationMillis):
      mFramesPerBlink((uint32_t) framesPerSecond * durationMillis / 1000)
    {}

    virtual void calcForFrame() override {
      uint16_t middleOfBlink = mFramesPerBlink / 2;
      mBlinkState = (mCurrentFrame < middleOfBlink) ? kOn : kOff;
      Util::incrementMod(mCurrentFrame, mFramesPerBlink);
    }

    virtual void apply(uint8_t* pattern, uint8_t* brightness) override {
      if (mBlinkState == kOff) {
        *brightness = 0;
      }
    }

    virtual bool requiresBrightness() override { return false; }

  private:
    static const uint8_t kOff = 0;
    static const uint8_t kOn = 1;

    const uint16_t mFramesPerBlink;
    uint16_t mCurrentFrame = 0;
    uint8_t mBlinkState;
};

}

#endif
