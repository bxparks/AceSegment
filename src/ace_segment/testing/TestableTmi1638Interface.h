#ifndef ACE_SEGMENT_TESTABLE_TMI_1638_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_TMI_1638_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableTmi1638Interface {
  public:
    void begin() const {
      gEventLog.addTmi1638Begin();
    }

    void end() const {
      gEventLog.addTmi1638End();
    }

    void beginTransaction() const {
      gEventLog.addTmi1638BeginTransaction();
    }

    void endTransaction() const {
      gEventLog.addTmi1638EndTransaction();
    }

    uint8_t write(uint8_t data) const {
      gEventLog.addTmi1638Write(data);
      return 0;
    }
};

} // testing
} // ace_segment

#endif
