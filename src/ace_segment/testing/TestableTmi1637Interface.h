#ifndef ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_TMI_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableTmi1637Interface {
  public:
    void begin() const {
      gEventLog.addTmi1637Begin();
    }

    void end() const {
      gEventLog.addTmi1637End();
    }

    void startCondition() const {
      gEventLog.addTmi1637StartCondition();
    }

    void stopCondition() const {
      gEventLog.addTmi1637StopCondition();
    }

    uint8_t write(uint8_t data) const {
      gEventLog.addTmi1637SendByte(data);
      return 0;
    }
};

} // testing
} // ace_segment

#endif
