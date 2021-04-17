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

#ifndef ACE_SEGMENT_FAST_SW_SPI_INTERFACE_H
#define ACE_SEGMENT_FAST_SW_SPI_INTERFACE_H

// This header file requires the digitalWriteFast library on AVR, or the
// EpoxyMockDigitalWriteFast library on EpoxyDuino.
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)

#include <stdint.h>
#include <Arduino.h> // OUTPUT

namespace ace_segment {

/**
 * Software SPI using pinModeFast(), digitalWriteFast() and shiftOutFast()
 * from https://github.com/NicksonYap/digitalWriteFast.
 *
 * @tparam latchPin the latch pin (CS)
 * @tparam dataPin the data pin (MOSI)
 * @tparam clockPin the clock pin (CLK)
 */
template <uint8_t latchPin, uint8_t dataPin, uint8_t clockPin>
class FastSwSpiInterface {
  public:
    FastSwSpiInterface() = default;

    void begin() const {
      pinModeFast(latchPin, OUTPUT);
      pinModeFast(dataPin, OUTPUT);
      pinModeFast(clockPin, OUTPUT);
    }

    void end() const {
      pinModeFast(latchPin, INPUT);
      pinModeFast(dataPin, INPUT);
      pinModeFast(clockPin, INPUT);
    }

    void transfer(uint8_t value) const {
      digitalWriteFast(latchPin, LOW);
      shiftOutFast(value);
      digitalWriteFast(latchPin, HIGH);
    }

    void transfer16(uint16_t value) const {
      uint8_t msb = (value & 0xff00) >> 8;
      uint8_t lsb = (value & 0xff);
      digitalWriteFast(latchPin, LOW);
      shiftOutFast(msb);
      shiftOutFast(lsb);
      digitalWriteFast(latchPin, HIGH);
    }

  static void shiftOutFast(uint8_t output) {
    uint8_t mask = 0x80; // start with the MSB
    for (uint8_t i = 0; i < 8; i++)  {
      digitalWriteFast(clockPin, LOW);
      if (output & mask) {
        digitalWriteFast(dataPin, HIGH);
      } else {
        digitalWriteFast(dataPin, LOW);
      }
      digitalWriteFast(clockPin, HIGH);
      mask >>= 1;
    }
  }

};

} // ace_segment

#endif // defined(ARDUINO_ARCH_AVR)

#endif
