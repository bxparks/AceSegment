#ifndef ACE_SEGMENT_TESTABLE_SPI_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_SPI_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableSpiInterface {
  public:
    void begin() const {
      mEventLog.addSpiBegin();
    }

    void end() const {
      mEventLog.addSpiEnd();
    }

    void send8(uint8_t value) const {
      mEventLog.addSpiSend8(value);
    }

    void send16(uint16_t value) const {
      mEventLog.addSpiSend16(value);
    }

  public:
    mutable EventLog mEventLog;
};

} // testing
} // ace_segment

#endif
