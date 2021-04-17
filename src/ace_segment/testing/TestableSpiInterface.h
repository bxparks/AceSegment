#ifndef ACE_SEGMENT_TESTABLE_SPI_ADAPTER_H
#define ACE_SEGMENT_TESTABLE_SPI_ADAPTER_H

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

    void transfer(uint8_t value) const {
      mEventLog.addSpiTransfer(value);
    }

    void transfer16(uint16_t value) const {
      mEventLog.addSpiTransfer16(value);
    }

  public:
    mutable EventLog mEventLog;
};

} // testing
} // ace_segment

#endif
