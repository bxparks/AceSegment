#ifndef ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableTmiInterface {
  public:
    void begin() const {
      gEventLog.addTmiBegin();
    }

    void end() const {
      gEventLog.addTmiEnd();
    }

    void startCondition() const {
      gEventLog.addTmiStartCondition();
    }

    void stopCondition() const {
      gEventLog.addTmiStopCondition();
    }

    uint8_t write(uint8_t data) const {
      gEventLog.addTmiSendByte(data);
      return 0;
    }
};

} // testing
} // ace_segment

#endif
