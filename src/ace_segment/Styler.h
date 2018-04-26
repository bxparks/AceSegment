#ifndef ACE_SEGMENT_STYLER_H
#define ACE_SEGMENT_STYLER_H

namespace ace_segment {

/**
 * Interface for classes which apply a style to the given bit pattern and
 * brightness.
 */
class Styler {
  public:
    /** Destructor. */
    virtual ~Styler() {}

    /** Called once per frame to update the internal variables of the object. */
    virtual void calcForFrame() = 0;

    /** Changes updates the brightness for current frame. */
    virtual void apply(uint8_t* pattern, uint8_t* brightness) = 0;

    /** Requires driver support for brightness control if true. */
    virtual bool requiresBrightness() = 0;
};

}

#endif
