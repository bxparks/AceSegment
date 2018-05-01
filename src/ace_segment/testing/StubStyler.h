#ifndef ACE_SEGMENT_STUB_STYLER_H
#define ACE_SEGMENT_STUB_STYLER_H

#include "../Styler.h"

namespace ace_segment {

/** A stub Styler for testing purposes that does nothing. */
class StubStyler: public Styler {
  public:
    virtual void calcForFrame() override {}

    virtual void apply(uint8_t* pattern, uint8_t* brightness) override {}

    virtual bool requiresBrightness() override { return false; }
};

}

#endif

