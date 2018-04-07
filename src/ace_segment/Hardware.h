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
    // TODO: replace with SPI version
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

  private:
    // disable copy-constructor and assignment operator
    Hardware(const Hardware&) = delete;
    Hardware& operator=(const Hardware&) = delete;
};

}

#endif
