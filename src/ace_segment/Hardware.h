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

#ifndef ACE_SEGMENT_HARDWARE_H
#define ACE_SEGMENT_HARDWARE_H

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>

namespace ace_segment {

class Hardware {
  public:
    typedef void (*DigitalWriter)();

    /** Constructor. */
    Hardware() {}

    /** Write value to pin. */
    virtual void digitalWrite(uint8_t pin, uint8_t value) {
      ::digitalWrite(pin, value);
    }

    /** Set pin mode. */
    virtual void pinMode(uint8_t pin, uint8_t mode) {
      ::pinMode(pin, mode);
    }

    /** Shift out. */
    virtual void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder,
        uint8_t value) {
      ::shiftOut(dataPin, clockPin, bitOrder, value);
    }

    /** Get the current micros  */
    virtual unsigned long micros() {
      return ::micros();
    }

    /** Get the current millis  */
    virtual unsigned long millis() {
      return ::millis();
    }

    /** Send byte through SPI. */
    virtual void spiTransfer(uint8_t value) {
      SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
      SPI.transfer(value);
      SPI.endTransaction();
    }

    virtual DigitalWriter getDigitalWriter(uint8_t pin, uint8_t value) {
      uint8_t index = 2 * pin + value;
      DigitalWriter writer = kDigitalWriters[index];
      return writer;
    }

  private:
    // Create an array of functions for digitalWrite() with the constants built
    // for D00 to D19. Doesn't do much for normal digitalWrite() but should
    // speed things up for digitalWriteFast().
    static void digitalWriteLow00() { ::digitalWrite(0, LOW); }
    static void digitalWriteHigh00() { ::digitalWrite(0, HIGH); }

    static void digitalWriteLow01() { ::digitalWrite(1, LOW); }
    static void digitalWriteHigh01() { ::digitalWrite(1, HIGH); }

    static void digitalWriteLow02() { ::digitalWrite(2, LOW); }
    static void digitalWriteHigh02() { ::digitalWrite(2, HIGH); }

    static void digitalWriteLow03() { ::digitalWrite(3, LOW); }
    static void digitalWriteHigh03() { ::digitalWrite(3, HIGH); }

    static void digitalWriteLow04() { ::digitalWrite(4, LOW); }
    static void digitalWriteHigh04() { ::digitalWrite(4, HIGH); }

    static void digitalWriteLow05() { ::digitalWrite(5, LOW); }
    static void digitalWriteHigh05() { ::digitalWrite(5, HIGH); }

    static void digitalWriteLow06() { ::digitalWrite(6, LOW); }
    static void digitalWriteHigh06() { ::digitalWrite(6, HIGH); }

    static void digitalWriteLow07() { ::digitalWrite(7, LOW); }
    static void digitalWriteHigh07() { ::digitalWrite(7, HIGH); }

    static void digitalWriteLow08() { ::digitalWrite(8, LOW); }
    static void digitalWriteHigh08() { ::digitalWrite(8, HIGH); }

    static void digitalWriteLow09() { ::digitalWrite(9, LOW); }
    static void digitalWriteHigh09() { ::digitalWrite(9, HIGH); }

    static void digitalWriteLow10() { ::digitalWrite(10, LOW); }
    static void digitalWriteHigh10() { ::digitalWrite(10, HIGH); }

    static void digitalWriteLow11() { ::digitalWrite(11, LOW); }
    static void digitalWriteHigh11() { ::digitalWrite(11, HIGH); }

    static void digitalWriteLow12() { ::digitalWrite(12, LOW); }
    static void digitalWriteHigh12() { ::digitalWrite(12, HIGH); }

    static void digitalWriteLow13() { ::digitalWrite(13, LOW); }
    static void digitalWriteHigh13() { ::digitalWrite(13, HIGH); }

    static void digitalWriteLow14() { ::digitalWrite(14, LOW); }
    static void digitalWriteHigh14() { ::digitalWrite(14, HIGH); }

    static void digitalWriteLow15() { ::digitalWrite(15, LOW); }
    static void digitalWriteHigh15() { ::digitalWrite(15, HIGH); }

    static void digitalWriteLow16() { ::digitalWrite(16, LOW); }
    static void digitalWriteHigh16() { ::digitalWrite(16, HIGH); }

    static void digitalWriteLow17() { ::digitalWrite(17, LOW); }
    static void digitalWriteHigh17() { ::digitalWrite(17, HIGH); }

    static void digitalWriteLow18() { ::digitalWrite(18, LOW); }
    static void digitalWriteHigh18() { ::digitalWrite(18, HIGH); }

    static void digitalWriteLow19() { ::digitalWrite(19, LOW); }
    static void digitalWriteHigh19() { ::digitalWrite(19, HIGH); }

    static const DigitalWriter kDigitalWriters[];
    static const size_t kNumWriters;

    // disable copy-constructor and assignment operator
    Hardware(const Hardware&) = delete;
    Hardware& operator=(const Hardware&) = delete;
};

}

#endif
