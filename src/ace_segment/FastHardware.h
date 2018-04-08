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
      DigitalWriter writer = kDigitalWriters[index];
      writer();
    }

    virtual void shiftOut(uint8_t dataPin, uint8_t clockPin,
        uint8_t bitOrder, uint8_t val) override;

    virtual DigitalWriter getDigitalWriter(uint8_t pin, uint8_t value) {
      uint8_t index = 2 * pin + value;
      DigitalWriter writer = kDigitalWriters[index];
      return writer;
    }

  private:
    // digitalWriteFast() requires both argments to be compile-time constants,
    // so we create an array of pointer to functions with the compile-time
    // constants built for D00 to D19.
    static void digitalWriteFastLow00() { digitalWriteFast(0, LOW); }
    static void digitalWriteFastHigh00() { digitalWriteFast(0, HIGH); }

    static void digitalWriteFastLow01() { digitalWriteFast(1, LOW); }
    static void digitalWriteFastHigh01() { digitalWriteFast(1, HIGH); }

    static void digitalWriteFastLow02() { digitalWriteFast(2, LOW); }
    static void digitalWriteFastHigh02() { digitalWriteFast(2, HIGH); }

    static void digitalWriteFastLow03() { digitalWriteFast(3, LOW); }
    static void digitalWriteFastHigh03() { digitalWriteFast(3, HIGH); }

    static void digitalWriteFastLow04() { digitalWriteFast(4, LOW); }
    static void digitalWriteFastHigh04() { digitalWriteFast(4, HIGH); }

    static void digitalWriteFastLow05() { digitalWriteFast(5, LOW); }
    static void digitalWriteFastHigh05() { digitalWriteFast(5, HIGH); }

    static void digitalWriteFastLow06() { digitalWriteFast(6, LOW); }
    static void digitalWriteFastHigh06() { digitalWriteFast(6, HIGH); }

    static void digitalWriteFastLow07() { digitalWriteFast(7, LOW); }
    static void digitalWriteFastHigh07() { digitalWriteFast(7, HIGH); }

    static void digitalWriteFastLow08() { digitalWriteFast(8, LOW); }
    static void digitalWriteFastHigh08() { digitalWriteFast(8, HIGH); }

    static void digitalWriteFastLow09() { digitalWriteFast(9, LOW); }
    static void digitalWriteFastHigh09() { digitalWriteFast(9, HIGH); }

    static void digitalWriteFastLow10() { digitalWriteFast(10, LOW); }
    static void digitalWriteFastHigh10() { digitalWriteFast(10, HIGH); }

    static void digitalWriteFastLow11() { digitalWriteFast(11, LOW); }
    static void digitalWriteFastHigh11() { digitalWriteFast(11, HIGH); }

    static void digitalWriteFastLow12() { digitalWriteFast(12, LOW); }
    static void digitalWriteFastHigh12() { digitalWriteFast(12, HIGH); }

    static void digitalWriteFastLow13() { digitalWriteFast(13, LOW); }
    static void digitalWriteFastHigh13() { digitalWriteFast(13, HIGH); }

    static void digitalWriteFastLow14() { digitalWriteFast(14, LOW); }
    static void digitalWriteFastHigh14() { digitalWriteFast(14, HIGH); }

    static void digitalWriteFastLow15() { digitalWriteFast(15, LOW); }
    static void digitalWriteFastHigh15() { digitalWriteFast(15, HIGH); }

    static void digitalWriteFastLow16() { digitalWriteFast(16, LOW); }
    static void digitalWriteFastHigh16() { digitalWriteFast(16, HIGH); }

    static void digitalWriteFastLow17() { digitalWriteFast(17, LOW); }
    static void digitalWriteFastHigh17() { digitalWriteFast(17, HIGH); }

    static void digitalWriteFastLow18() { digitalWriteFast(18, LOW); }
    static void digitalWriteFastHigh18() { digitalWriteFast(18, HIGH); }

    static void digitalWriteFastLow19() { digitalWriteFast(19, LOW); }
    static void digitalWriteFastHigh19() { digitalWriteFast(19, HIGH); }

    static const DigitalWriter kDigitalWriters[];
    static const size_t kNumWriters;

    // disable copy-constructor and assignment operator
    FastHardware(const FastHardware&) = delete;
    FastHardware& operator=(const FastHardware&) = delete;
};

}

#endif
