#line 2 "BaseHardwareTest.cpp"

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

#include "BaseHardwareTest.h"

namespace ace_segment {
namespace testing {

void BaseHardwareTest::assertEvents(int8_t n, ...) {
  assertEqual(n, mHardware->getNumRecords());
  va_list args;
  va_start(args, n);
  for (int i = 0; i < n; i++) {
    uint8_t type = va_arg(args, int);
    Event& event = mHardware->getEvent(i);
    assertEqual(type, event.type);
    switch (type) {
      case Event::kTypeDigitalWrite: {
          uint8_t pin = va_arg(args, int);
          uint8_t value = va_arg(args, int);
          assertEqual(pin, event.arg1);
          assertEqual(value, event.arg2);
        }
        break;
      case Event::kTypePinMode: {
          uint8_t pin = va_arg(args, int);
          uint8_t mode = va_arg(args, int);
          assertEqual(pin, event.arg1);
          assertEqual(mode, event.arg2);
        }
        break;
      case Event::kTypeShiftOut: {
          uint8_t dataPin = va_arg(args, int);
          uint8_t clockPin = va_arg(args, int);
          uint8_t bitOrder = va_arg(args, int);
          uint8_t value = va_arg(args, int);
          assertEqual(dataPin, event.arg1);
          assertEqual(clockPin, event.arg2);
          assertEqual(bitOrder, event.arg3);
          assertEqual(value, event.arg4);
        }
        break;
      case Event::kTypeSpiBegin:
        break;
      case Event::kTypeSpiEnd:
        break;
      case Event::kTypeSpiTransfer: {
          uint8_t value = va_arg(args, int);
          assertEqual(value, event.arg1);
        }
        break;
      default:
        // Unknown event type
        fail();
    }
  }
  va_end(args);
}

} // namespace testing
} // namespace ace_segment
