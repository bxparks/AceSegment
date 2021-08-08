#ifndef ACE_SEGMENT_TESTABLE_SPI_INTERFACE_H
#define ACE_SEGMENT_TESTABLE_SPI_INTERFACE_H

#include "EventLog.h" // EventLog

namespace ace_segment {
namespace testing {

class TestableSpiInterface {
  public:
    void begin() const {
      gEventLog.addSpiBegin();
    }

    void end() const {
      gEventLog.addSpiEnd();
    }

    void send8(uint8_t value) const {
      gEventLog.addSpiSend8(value);
    }

    void send16(uint16_t value) const {
      gEventLog.addSpiSend16(value);
    }

    void send16(uint8_t msb, uint8_t lsb) const {
      uint16_t value = ((uint16_t) msb) << 8 | (uint16_t) lsb;
      send16(value);
    }
};

} // testing
} // ace_segment

#endif
