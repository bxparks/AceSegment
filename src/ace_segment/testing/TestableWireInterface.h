#ifndef ACE_SEGMENT_TESTABLE_WIRE_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_WIRE_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableWireInterface {
  public:
    void begin() const {
      mEventLog.addWireBegin();
    }

    void end() const {
      mEventLog.addWireEnd();
    }

    void startCondition() const {
      mEventLog.addWireStartCondition();
    }

    void stopCondition() const {
      mEventLog.addWireStopCondition();
    }

    void sendByte(uint8_t data) const {
      mEventLog.addWireSendByte(data);
    }

  public:
    mutable EventLog mEventLog;
};

} // testing
} // ace_segment

#endif
