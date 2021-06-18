#ifndef ACE_SEGMENT_TESTABLE_WIRE_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_WIRE_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

/**
 * Version of TwoWireInterface with the same API but writes to the EventLog
 * so that it can be validated in unit tests.
 */
class TestableWireInterface {
  public:
    void begin() const {
      mEventLog.addWireBegin();
    }

    void end() const {
      mEventLog.addWireEnd();
    }

    void beginTransmission(uint8_t addr) const {
      mEventLog.addWireBeginTransmission(addr);
    }

    void write(uint16_t data) const {
      mEventLog.addWireWrite(data);
    }

    void endTransmission() const {
      mEventLog.addWireEndTransmission();
    }

  public:
    mutable EventLog mEventLog;
};

} // testing
} // ace_segment

#endif
