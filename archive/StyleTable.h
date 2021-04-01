#ifndef ACE_SEGMENT_STYLE_TABLE_H
#define ACE_SEGMENT_STYLE_TABLE_H

#include "Styler.h"

namespace ace_segment {

class StyleTable {
  public:
    /**
     * Maximum number of styles. Valid style indexes are [0, kNumStyles-1].
     * Style 0 is reserved for "no-style" and cannot be changed by the user, so
     * the number of user-accessible styles is (kNumStyles - 1).
     */
    // TODO: Make this a template parameter?
    static const uint8_t kNumStyles = 6;

    /** Constructor. */
    StyleTable() {
      for (uint8_t i = 0; i < kNumStyles; i++) {
        mStylers[i] = nullptr;
      }
    }

    /**
     * Set the Styler for the given styleIndex. Currently supports maximum of
     * kNumStyles which is 4.
     */
    void setStyler(uint8_t styleIndex, Styler* styler) {
      if (styleIndex > 0 && styleIndex < kNumStyles) {
        mStylers[styleIndex] = styler;
      }
    }

    /** Retrieve the Styler at styleIndex. Returns nullptr if not defined. */
    Styler* getStyler(uint8_t styleIndex) const {
      return (styleIndex < kNumStyles) ? mStylers[styleIndex] : nullptr;
    }

  private:
    // Array of Stylers. Index 0 is reserved and represents the "no-style"
    // setting which does nothing. It cannot be set by the the client code.
    Styler* mStylers[kNumStyles];
};

}

#endif
