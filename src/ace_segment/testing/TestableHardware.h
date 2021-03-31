/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ACE_SEGMENT_TESTABLE_HARDWARE_H
#define ACE_SEGMENT_TESTABLE_HARDWARE_H

#include "../Hardware.h"

namespace ace_segment {
namespace testing {

/** A record of one Hardware event. */
class Event {
  public:
    static const uint8_t kTypeDigitalWrite = 0;
    static const uint8_t kTypePinMode = 1;
    static const uint8_t kTypeSpiBegin = 3;
    static const uint8_t kTypeSpiTransfer = 4;
    static const uint8_t kTypeSpiEnd = 5;
    static const uint8_t kTypeSpiTransfer16 = 6;

    uint8_t type; // arg0
    uint8_t arg1;
    uint8_t arg2;
    uint8_t arg3;
    uint8_t arg4;
    uint16_t arg5; // used by spiTransfer16()
};

class TestableHardware {
  public:
    explicit TestableHardware():
      mNumRecords(0)
    {}

    unsigned long micros() const { return mMicros; }

    unsigned long millis() const { return mMillis; }

    void pinMode(uint8_t pin, uint8_t mode) const {
      if (mNumRecords < kMaxRecords) {
        Event& event = mEvents[mNumRecords];
        event.type = Event::kTypePinMode;
        event.arg1 = pin;
        event.arg2 = mode;
        mNumRecords++;
      }
    }

    void digitalWrite(uint8_t pin, uint8_t value) const {
      if (mNumRecords < kMaxRecords) {
        Event& event = mEvents[mNumRecords];
        event.type = Event::kTypeDigitalWrite;
        event.arg1 = pin;
        event.arg2 = value;
        mNumRecords++;
      }
    }

    // methods to set what this object returns

    void setMicros(unsigned long micros) { mMicros = micros; }

    void setMillis(unsigned long millis) { mMillis = millis; }

    void clear() { mNumRecords = 0; }

    uint8_t getNumRecords() { return mNumRecords; }

    Event& getEvent(int i) { return mEvents[i]; }

  private:
    static const int kMaxRecords = 32;

    // Disable copy-constructor and assignment operator
    TestableHardware(const TestableHardware&) = delete;
    TestableHardware& operator=(const TestableHardware&) = delete;

    unsigned long mMillis;
    unsigned long mMicros;

    mutable Event mEvents[kMaxRecords];
    mutable uint8_t mNumRecords;
};

} // namespace testing
} // namespace ace_segment

#endif
