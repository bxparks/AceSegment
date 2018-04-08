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

#ifndef ACE_SEGMENT_FAST_HARDWARE_H
#define ACE_SEGMENT_FAST_HARDWARE_H

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include "Hardware.h"

// Helper macro from https://isocpp.org/wiki/faq/pointers-to-members
#ifndef CALL_MEMBER_FN
#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))
#endif

namespace ace_segment {

/**
 * An experimental version of Hardware that uses the digitalWriteFast library
 * (https://github.com/NicksonYap/digitalWriteFast). Only digitalWriteFast()
 * method is used because that's the most time critical, pinModeFast() is not
 * used. Seems to make only about a 16us difference in LedMatrixSerial and
 * about 4-8us different in LedMatrixSpi.
 */
class FastHardware: public Hardware {
  public:
    /** Constructor. */
    FastHardware() {}

    virtual void digitalWrite(uint8_t pin, uint8_t value) override {
      if (pin >= kNumWriters) return;
      uint8_t index = 2 * pin + value;
      FastWriter writer = kFastWriters[index];
      CALL_MEMBER_FN(*this, writer)();
    }

    virtual void shiftOut(uint8_t dataPin, uint8_t clockPin,
        uint8_t bitOrder, uint8_t val) override;

  private:
    typedef void (FastHardware::*FastWriter)();

    // digitalWriteFast() requires both argments to be compile-time constants,
    // so we create an array of member functions with the compile-time
    // constants built for D00 to D19.
    void digitalWriteFastLow00() { digitalWriteFast(0, LOW); }
    void digitalWriteFastHigh00() { digitalWriteFast(0, HIGH); }

    void digitalWriteFastLow01() { digitalWriteFast(1, LOW); }
    void digitalWriteFastHigh01() { digitalWriteFast(1, HIGH); }

    void digitalWriteFastLow02() { digitalWriteFast(2, LOW); }
    void digitalWriteFastHigh02() { digitalWriteFast(2, HIGH); }

    void digitalWriteFastLow03() { digitalWriteFast(3, LOW); }
    void digitalWriteFastHigh03() { digitalWriteFast(3, HIGH); }

    void digitalWriteFastLow04() { digitalWriteFast(4, LOW); }
    void digitalWriteFastHigh04() { digitalWriteFast(4, HIGH); }

    void digitalWriteFastLow05() { digitalWriteFast(5, LOW); }
    void digitalWriteFastHigh05() { digitalWriteFast(5, HIGH); }

    void digitalWriteFastLow06() { digitalWriteFast(6, LOW); }
    void digitalWriteFastHigh06() { digitalWriteFast(6, HIGH); }

    void digitalWriteFastLow07() { digitalWriteFast(7, LOW); }
    void digitalWriteFastHigh07() { digitalWriteFast(7, HIGH); }

    void digitalWriteFastLow08() { digitalWriteFast(8, LOW); }
    void digitalWriteFastHigh08() { digitalWriteFast(8, HIGH); }

    void digitalWriteFastLow09() { digitalWriteFast(9, LOW); }
    void digitalWriteFastHigh09() { digitalWriteFast(9, HIGH); }

    void digitalWriteFastLow10() { digitalWriteFast(10, LOW); }
    void digitalWriteFastHigh10() { digitalWriteFast(10, HIGH); }

    void digitalWriteFastLow11() { digitalWriteFast(11, LOW); }
    void digitalWriteFastHigh11() { digitalWriteFast(11, HIGH); }

    void digitalWriteFastLow12() { digitalWriteFast(12, LOW); }
    void digitalWriteFastHigh12() { digitalWriteFast(12, HIGH); }

    void digitalWriteFastLow13() { digitalWriteFast(13, LOW); }
    void digitalWriteFastHigh13() { digitalWriteFast(13, HIGH); }

    void digitalWriteFastLow14() { digitalWriteFast(14, LOW); }
    void digitalWriteFastHigh14() { digitalWriteFast(14, HIGH); }

    void digitalWriteFastLow15() { digitalWriteFast(15, LOW); }
    void digitalWriteFastHigh15() { digitalWriteFast(15, HIGH); }

    void digitalWriteFastLow16() { digitalWriteFast(16, LOW); }
    void digitalWriteFastHigh16() { digitalWriteFast(16, HIGH); }

    void digitalWriteFastLow17() { digitalWriteFast(17, LOW); }
    void digitalWriteFastHigh17() { digitalWriteFast(17, HIGH); }

    void digitalWriteFastLow18() { digitalWriteFast(18, LOW); }
    void digitalWriteFastHigh18() { digitalWriteFast(18, HIGH); }

    void digitalWriteFastLow19() { digitalWriteFast(19, LOW); }
    void digitalWriteFastHigh19() { digitalWriteFast(19, HIGH); }

    static const FastWriter kFastWriters[];
    static const size_t kNumWriters;

  private:

    FastWriter getFastWriter(uint8_t pin, uint8_t value) {
      uint8_t index = 2 * pin + value;
      FastWriter writer = kFastWriters[index];
      return writer;
    }

    // disable copy-constructor and assignment operator
    FastHardware(const FastHardware&) = delete;
    FastHardware& operator=(const FastHardware&) = delete;
};

}

#endif
