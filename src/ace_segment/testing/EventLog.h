/*
MIT License

Copyright (c) 2021 Brian T. Park

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

#ifndef ACE_SEGMENT_TEST_EVENT_LOG_H
#define ACE_SEGMENT_TEST_EVENT_LOG_H

#include <stdint.h>
#include <stdarg.h>

namespace ace_segment {
namespace testing {

enum class EventType : uint8_t {
  kDigitalWrite,
  kPinMode,
  // SpiInterface
  kSpiBegin,
  kSpiEnd,
  kSpiSend8,
  kSpiSend16,
  // Tmi1637Interface
  kTmi1637Begin,
  kTmi1637End,
  kTmi1637StartCondition,
  kTmi1637StopCondition,
  kTmi1637SendByte,
  // Tmi1638Interface
  kTmi1638Begin,
  kTmi1638End,
  kTmi1638BeginTransaction,
  kTmi1638EndTransaction,
  kTmi1638Write,
  // WireInterface
  kWireBegin,
  kWireEnd,
  kWireBeginTransmission,
  kWireEndTransmission,
  kWireWrite,
  // LedMatrix
  kLedMatrixDraw,
  kLedMatrixEnableGroup,
  kLedMatrixDisableGroup,
  kLedMatrixClear,
};

/** A record of one Hardware event. */
struct Event {
  EventType type; // arg0
  uint8_t arg1;
  uint8_t arg2;
  uint8_t arg3;
  uint16_t arg5; // used by send16()
};

class EventLog {
  public:
    EventLog() {}

    void addDigitalWrite(uint8_t pin, uint8_t value) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kDigitalWrite;
      event.arg1 = pin;
      event.arg2 = value;
      mNumRecords++;
    }

    void addPinMode(uint8_t pin, uint8_t mode) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kPinMode;
      event.arg1 = pin;
      event.arg2 = mode;
      mNumRecords++;
    }

    //-------------------------------------------------------------------------

    void addSpiBegin() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kSpiBegin;
      mNumRecords++;
    }

    void addSpiEnd() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kSpiEnd;
      mNumRecords++;
    }

    void addSpiSend8(uint8_t value) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kSpiSend8;
      event.arg1 = value;
      mNumRecords++;
    }

    void addSpiSend16(uint16_t value) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kSpiSend16;
      event.arg5 = value;
      mNumRecords++;
    }

    //-------------------------------------------------------------------------

    void addTmi1637Begin() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1637Begin;
      mNumRecords++;
    }

    void addTmi1637End() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1637End;
      mNumRecords++;
    }

    void addTmi1637StartCondition() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1637StartCondition;
      mNumRecords++;
    }

    void addTmi1637StopCondition() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1637StopCondition;
      mNumRecords++;
    }

    void addTmi1637SendByte(uint8_t data) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1637SendByte;
      event.arg1 = data;
      mNumRecords++;
    }

    //-------------------------------------------------------------------------

    void addTmi1638Begin() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1638Begin;
      mNumRecords++;
    }

    void addTmi1638End() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1638End;
      mNumRecords++;
    }

    void addTmi1638BeginTransaction() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1638BeginTransaction;
      mNumRecords++;
    }

    void addTmi1638EndTransaction() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1638EndTransaction;
      mNumRecords++;
    }

    void addTmi1638Write(uint8_t data) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kTmi1638Write;
      event.arg1 = data;
      mNumRecords++;
    }

    //-------------------------------------------------------------------------

    void addWireBegin() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kWireBegin;
      mNumRecords++;
    }

    void addWireEnd() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kWireEnd;
      mNumRecords++;
    }

    void addWireBeginTransmission(uint8_t addr) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kWireBeginTransmission;
      event.arg1 = addr;
      mNumRecords++;
    }

    void addWireEndTransmission() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kWireEndTransmission;
      mNumRecords++;
    }

    void addWireWrite(uint8_t data) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kWireWrite;
      event.arg1 = data;
      mNumRecords++;
    }

    //-------------------------------------------------------------------------

    void addLedMatrixDraw(uint8_t group, uint8_t elementPattern) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kLedMatrixDraw;
      event.arg1 = group;
      event.arg2 = elementPattern;
      mNumRecords++;
    }

    void addLedMatrixEnableGroup(uint8_t group) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kLedMatrixEnableGroup;
      event.arg1 = group;
      mNumRecords++;
    }

    void addLedMatrixDisableGroup(uint8_t group) {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kLedMatrixDisableGroup;
      event.arg1 = group;
      mNumRecords++;
    }

    void addLedMatrixClear() {
      if (mNumRecords >= kMaxRecords) return;

      Event& event = mEvents[mNumRecords];
      event.type = EventType::kLedMatrixClear;
      mNumRecords++;
    }

    //-------------------------------------------------------------------------

    uint8_t getNumRecords() { return mNumRecords; }

    Event& getEvent(int i) { return mEvents[i]; }

    void clear() { mNumRecords = 0; }

    /**
     * Return false if the event log does not match the given arguments.
     *
     * @param n number of variable parameters (must be `int` type to be used
     * in va_start() macro).
     */
    bool assertEvents(int n, ...) {
      if (n != mNumRecords) return false;

      va_list args;
      va_start(args, n);
      for (uint8_t i = 0; i < n; i++) {
        EventType type = (EventType) va_arg(args, int);
        Event& event = mEvents[i];
        if (type != event.type) return false;

        switch (type) {
          case EventType::kDigitalWrite: {
              uint8_t pin = va_arg(args, int);
              uint8_t value = va_arg(args, int);
              if (pin != event.arg1) return false;
              if (value != event.arg2) return false;
            }
            break;

          case EventType::kPinMode: {
              uint8_t pin = va_arg(args, int);
              uint8_t mode = va_arg(args, int);
              if (pin != event.arg1) return false;
              if (mode != event.arg2) return false;
            }
            break;

          //------------------------------------------------------------------

          case EventType::kSpiBegin:
            break;

          case EventType::kSpiEnd:
            break;

          case EventType::kSpiSend8: {
              uint8_t value = va_arg(args, int);
              if (value != event.arg1) return false;
            }
            break;

          case EventType::kSpiSend16: {
              uint16_t value = va_arg(args, int);
              if (value != event.arg5) return false;
            }
            break;

          //------------------------------------------------------------------

          case EventType::kTmi1637Begin:
            break;

          case EventType::kTmi1637End:
            break;

          case EventType::kTmi1637StopCondition:
            break;

          case EventType::kTmi1637StartCondition:
            break;

          case EventType::kTmi1637SendByte: {
              uint8_t value = va_arg(args, int);
              if (value != event.arg1) return false;
            }
            break;

          //------------------------------------------------------------------

          case EventType::kTmi1638Begin:
            break;

          case EventType::kTmi1638End:
            break;

          case EventType::kTmi1638BeginTransaction:
            break;

          case EventType::kTmi1638EndTransaction:
            break;

          case EventType::kTmi1638Write: {
              uint8_t value = va_arg(args, int);
              if (value != event.arg1) return false;
            }
            break;

          //------------------------------------------------------------------

          case EventType::kWireBegin:
            break;

          case EventType::kWireEnd:
            break;

          case EventType::kWireBeginTransmission: {
              uint8_t value = va_arg(args, int);
              if (value != event.arg1) return false;
            }
            break;

          case EventType::kWireEndTransmission:
            break;

          case EventType::kWireWrite: {
              uint8_t value = va_arg(args, int);
              if (value != event.arg1) return false;
            }
            break;

          //------------------------------------------------------------------

          case EventType::kLedMatrixDraw: {
              uint8_t group = va_arg(args, int);
              uint8_t elementPattern = va_arg(args, int);
              if (group != event.arg1) return false;
              if (elementPattern != event.arg2) return false;
            }
            break;

          case EventType::kLedMatrixEnableGroup: {
              uint8_t group = va_arg(args, int);
              if (group != event.arg1) return false;
            }
            break;

          case EventType::kLedMatrixDisableGroup: {
              uint8_t group = va_arg(args, int);
              if (group != event.arg1) return false;
            }
            break;

          case EventType::kLedMatrixClear:
            break;
        }
      }
      va_end(args);

      return true;
    }

  private:
    static const int kMaxRecords = 32;

    Event mEvents[kMaxRecords];
    uint8_t mNumRecords = 0;
};

// One global instance for testing.
extern EventLog gEventLog;

} // testing
} // ace_segment

#endif
