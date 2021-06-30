#ifndef ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableTmiInterface {
  public:
    void begin() const {
      sEventLog.addTmiBegin();
    }

    void end() const {
      sEventLog.addTmiEnd();
    }

    void startCondition() const {
      sEventLog.addTmiStartCondition();
    }

    void stopCondition() const {
      sEventLog.addTmiStopCondition();
    }

    void sendByte(uint8_t data) const {
      sEventLog.addTmiSendByte(data);
    }

  public:
    static EventLog sEventLog;
};

} // testing
} // ace_segment

#endif
