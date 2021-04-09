/*
 * MIT License
 * Copyright (c) 2021 Brian T. Park
 */


#ifndef ACE_SEGMENT_TESTABLE_LED_MATRIX_H
#define ACE_SEGMENT_TESTABLE_LED_MATRIX_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableLedMatrix {
  public:
    void draw(uint8_t group, uint8_t elementPattern) const {
      mEventLog.addLedMatrixDraw(group, elementPattern);
    }

    void enableGroup(uint8_t group) const {
      mEventLog.addLedMatrixEnableGroup(group);
    }

    void disableGroup(uint8_t group) const {
      mEventLog.addLedMatrixDisableGroup(group);
    }

    void clear() const {
      mEventLog.addLedMatrixClear();
    }

  public:
    mutable EventLog mEventLog;
};

}
}

#endif
