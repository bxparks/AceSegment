#ifndef ACE_SEGMENT_TESTABLE_SPI_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_SPI_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableSpiInterface {
  public:
    void begin() const {
      sEventLog.addSpiBegin();
    }

    void end() const {
      sEventLog.addSpiEnd();
    }

    void send8(uint8_t value) const {
      sEventLog.addSpiSend8(value);
    }

    void send16(uint16_t value) const {
      sEventLog.addSpiSend16(value);
    }

  public:
    // The SpiInterface is copied by value into LedMarixSingleHc595 and
    // LedMatrixDoubleHc595. We need to make sure there is only one copy of
    // the EventLog, so make it static.
    static EventLog sEventLog;
};

} // testing
} // ace_segment

#endif
