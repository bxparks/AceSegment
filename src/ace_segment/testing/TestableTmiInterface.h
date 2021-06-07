#ifndef ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableTmiInterface {
  public:
    void begin() const {
      mEventLog.addTmiBegin();
    }

    void end() const {
      mEventLog.addTmiEnd();
    }

    void startCondition() const {
      mEventLog.addTmiStartCondition();
    }

    void stopCondition() const {
      mEventLog.addTmiStopCondition();
    }

    void sendByte(uint8_t data) const {
      mEventLog.addTmiSendByte(data);
    }

  public:
    mutable EventLog mEventLog;
};

} // testing
} // ace_segment

#endif
