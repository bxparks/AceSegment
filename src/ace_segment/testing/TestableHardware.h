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

/** A record of writePin() events. */
class PinRecord {
  public:
    uint8_t pin;
    uint8_t value;
};

class TestableHardware: public Hardware {
  public:
    explicit TestableHardware():
      mNumRecords(0)
    {}

    virtual unsigned long getMicros() override { return mMicros; }

    virtual unsigned long getMillis() override { return mMillis; }

    virtual void setPinMode(uint8_t pin, uint8_t mode) override {
      mPinMode = mode;
    }

    virtual void writePin(uint8_t pin, uint8_t value) override {
      if (mNumRecords < kMaxRecords) {
        PinRecord& record = mPinRecords[mNumRecords];
        record.pin = pin;
        record.value = value;
        mNumRecords++;
      }
    }

    // test routines

    void setMicros(unsigned long micros) { mMicros = micros; }

    void setMillis(unsigned long millis) { mMillis = millis; }

    uint8_t getPinMode(uint8_t pin) { return mPinMode; }

    void clear() { mNumRecords = 0; }

    int getNumRecords() { return mNumRecords; }

    PinRecord& getPinRecord(int i) { return mPinRecords[i]; }

  private:
    // Max = #segments + 2 (digits) = 10, but let's make it 16 just in case.
    static const int kMaxRecords = 16;

    // Disable copy-constructor and assignment operator
    TestableHardware(const TestableHardware&) = delete;
    TestableHardware& operator=(const TestableHardware&) = delete;

    unsigned long mMillis;
    unsigned long mMicros;
    uint8_t mPinMode;

    PinRecord mPinRecords[kMaxRecords];
    int mNumRecords;
};

}

}

#endif
