#ifndef ACE_SEGMENT_PULSE_STYLER_H
#define ACE_SEGMENT_PULSE_STYLER_H

#include <stdint.h>
#include "Styler.h"

namespace ace_segment {

class PulseStyler: public Styler {
  public:
    PulseStyler(uint8_t framesPerSecond, uint16_t durationMillis):
      mFramesPerPulse((uint32_t) framesPerSecond * durationMillis / 1000),
      mFramesPerPulseInverse((uint32_t) 65536 * 1000
          / framesPerSecond / durationMillis)
    {}

    virtual void calcForFrame() override {
      uint16_t middleOfPulse = mFramesPerPulse / 2;

      uint16_t fraction;
      if (mCurrentFrame < middleOfPulse) {
        fraction = (uint32_t) mFramesPerPulseInverse * mCurrentFrame / (256/2);
      } else if (mCurrentFrame < mFramesPerPulse) {
        uint16_t reverse = (mFramesPerPulse - mCurrentFrame - 1);
        fraction = (uint32_t) mFramesPerPulseInverse * reverse / (256/2);
      } else {
        fraction = 0;
      }
      if (fraction > 255) fraction = 255;

      mPulseFraction = fraction;
      Util::incrementMod(mCurrentFrame, mFramesPerPulse);
    }

    virtual void apply(uint8_t* pattern, uint8_t* brightness) override {
      *brightness = ((uint16_t) mPulseFraction * (*brightness)) / 256;
    }

    virtual bool requiresBrightness() override { return true; }

  private:
    const uint16_t mFramesPerPulse;
    const uint16_t mFramesPerPulseInverse; // 65536/mFramesPerPulse
    uint16_t mCurrentFrame = 0;
    uint8_t mPulseFraction;
};

}

#endif
