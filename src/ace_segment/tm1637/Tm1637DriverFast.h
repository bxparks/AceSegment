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

#ifndef ACE_SEGMENT_TM1637_DRIVER_FAST_H
#define ACE_SEGMENT_TM1637_DRIVER_FAST_H

// This header file requires the digitalWriteFast library on AVR, or the
// EpoxyMockDigitalWriteFast library on EpoxyDuino.
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)

#include <stdint.h>
#include <Arduino.h>

namespace ace_segment {

/**
 * Exactly the same as Tm1637Driver except that this uses the `digitalWriteFast`
 * library on AVR processors. Normally, the digitalWriteFast library is used to
 * get faster speeds over `digitalWrite()` and `pinMode()` functions. But speed
 * of the `digitalWrite()` functions is not the limiting factor in this library
 * because every bit flip is followed by a `delayMicroseconds()` which is far
 * longer than the CPU cycle savings from `digitalWritFast()`.
 *
 * The reason that you may want to use `digitalWriteFast` library is because it
 * consumes far less flash memory than normal `digitalWrite()`. The benchmarks
 * in MemoryBenchmark shows that using this `Tm1637DriverFast` instead of
 * `Tm1637Driver` saves 650-770 bytes of flash on an AVR processor.
 *
 * Word of caution though, there is a use-case where you may want to still use
 * the normal `Tm1637Driver`. If your application uses more than one TM1637 LED
 * Module, you will need to create multiple instances of the `Tm1637Display`.
 * But note that the pin numbers of this class must be a compile-time constants,
 * so different pins means that a different template class is generated. Since
 * `Tm1637Display` class takes a `Tm1637DriverFast` as a template argument, each
 * LED Module generate a new template instance of the `Tm1637Display` class.
 *
 * In the case of multiple LED modules, it may actually be more efficient to use
 * the non-fast `Tm1637Driver`, because you will generate only a single template
 * instantiation. There will be multiple instances of that class, but only a
 * single class.
 *
 * This class is stateless. It is thread-safe.
 */
template <
    uint8_t CLOCK_PIN,
    uint8_t DIO_PIN,
    uint16_t DELAY_MICROS
>
class Tm1637DriverFast {
  public:
    explicit Tm1637DriverFast() = default;

    /** Initialize the GPIO pins. */
    void begin() const {
      // These are open-drain lines, with a pull-up resistor. We must not drive
      // them HIGH actively since that could damage the transitor at the other
      // end of the line pulling LOW. Instead, we go into INPUT mode to let the
      // line to HIGH through the pullup resistor, then go to OUTPUT mode only
      // to pull down.
      digitalWriteFast(CLOCK_PIN, LOW);
      digitalWriteFast(DIO_PIN, LOW);

      // Begin with both lines at HIGH.
      clockHigh();
      dataHigh();
    }

    /** Set pins to INPUT mode. */
    void end() const {
      clockHigh();
      dataHigh();
    }

    /** Generate the I2C start condition. */
    void startCondition() const {
      clockHigh();
      dataHigh();

      dataLow();
      clockLow();
    }

    /** Generate the I2C stop condition. */
    void stopCondition() const {
      dataLow();
      clockHigh();
      dataHigh();
    }

    /**
     * Send the data byte on the data bus.
     * @return 0 means ACK, 1 means NACK.
     */
    uint8_t sendByte(uint8_t data) const {
      for (uint8_t i = 0;  i < 8; ++i) {
        if (data & 0x1) {
          dataHigh();
        } else {
          dataLow();
        }
        clockHigh();
        clockLow();
        data >>= 1;
      }

      // Device places the ACK/NACK bit upon the falling edge of the 8th CLK,
      // which happens in the loop above.
      pinModeFast(DIO_PIN, INPUT);
      bitDelay();
      uint8_t ack = digitalReadFast(DIO_PIN);

      // Device releases DIO upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

  private:
    void bitDelay() const { delayMicroseconds(DELAY_MICROS); }

    void clockHigh() const { pinModeFast(CLOCK_PIN, INPUT); bitDelay(); }

    void clockLow() const { pinModeFast(CLOCK_PIN, OUTPUT); bitDelay(); }

    void dataHigh() const { pinModeFast(DIO_PIN, INPUT); bitDelay(); }

    void dataLow() const { pinModeFast(DIO_PIN, OUTPUT); bitDelay(); }
};

} // ace_segment

#endif // defined(ARDUINO_ARCH_AVR)

#endif
