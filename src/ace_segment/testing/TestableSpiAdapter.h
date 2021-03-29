#ifndef ACE_SEGMENT_TESTABLE_SPI_ADAPTER_H
#define ACE_SEGMENT_TESTABLE_SPI_ADAPTER_H

#include "../Hardware.h"
#include "../TestableHardware.h" // Event

namespace ace_segment {
namespace testing {

class TestableSpiAdapter : public SpiAdapter {
  public:
    explicit TestableSpiAdapter():
      mNumRecords(0)
    {}

    void spiBegin() const override {
      if (mNumRecords < kMaxRecords) {
        Event& event = mEvents[mNumRecords];
        event.type = Event::kTypeSpiBegin;
        mNumRecords++;
      }
    }

    void spiEnd() const override {
      if (mNumRecords < kMaxRecords) {
        Event& event = mEvents[mNumRecords];
        event.type = Event::kTypeSpiEnd;
        mNumRecords++;
      }
    }

    void spiTransfer(uint8_t value) const override {
      if (mNumRecords < kMaxRecords) {
        Event& event = mEvents[mNumRecords];
        event.type = Event::kTypeSpiTransfer;
        event.arg1 = value;
        mNumRecords++;
      }
    }

    void spiTransfer16(uint16_t value) const override {
      if (mNumRecords < kMaxRecords) {
        Event& event = mEvents[mNumRecords];
        event.type = Event::kTypeSpiTransfer16;
        event.arg5 = value;
        mNumRecords++;
      }
    }

    void clear() { mNumRecords = 0; }

    uint8_t getNumRecords() { return mNumRecords; }

    Event& getEvent(int i) { return mEvents[i]; }

  private:
    static const int kMaxRecords = 32;

    // Disable copy-constructor and assignment operator
    TestableSpiAdapter(const TestableSpiAdapter&) = delete;
    TestableSpiAdapter& operator=(const TestableSpiAdapter&) = delete;

    mutable Event mEvents[kMaxRecords];
    mutable uint8_t mNumRecords;
};

#endif
