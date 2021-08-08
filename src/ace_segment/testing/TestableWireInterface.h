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
      gEventLog.addWireBegin();
    }

    void end() const {
      gEventLog.addWireEnd();
    }

    void beginTransmission(uint8_t addr) const {
      gEventLog.addWireBeginTransmission(addr);
    }

    void write(uint16_t data) const {
      gEventLog.addWireWrite(data);
    }

    void endTransmission(bool sendStop = true) const {
      (void) sendStop; // disable compiler warning
      gEventLog.addWireEndTransmission();
    }
};

} // testing
} // ace_segment

#endif
