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

#ifndef ACE_SEGMENT_SOFT_TMI_FAST_INTERFACE_H
#define ACE_SEGMENT_SOFT_TMI_FAST_INTERFACE_H

// This header file requires the digitalWriteFast library on AVR, or the
// EpoxyMockDigitalWriteFast library on EpoxyDuino.
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)

#include <stdint.h>
#include <Arduino.h> // delayMicroseconds()

namespace ace_segment {

/**
 * Exactly the same as SoftTmiInterface except that this uses the
 * `digitalWriteFast` library on AVR processors. Normally, the digitalWriteFast
 * library is used to get faster speeds over `digitalWrite()` and `pinMode()`
 * functions. But speed of the `digitalWrite()` functions is not the limiting
 * factor in this library because every bit flip is followed by a
 * `delayMicroseconds()` which is far longer than the CPU cycle savings from
 * `digitalWritFast()`.
 *
 * The reason that you may want to use `digitalWriteFast` library is because it
 * consumes far less flash memory than normal `digitalWrite()`. The benchmarks
 * in MemoryBenchmark shows that using this `SoftTmiFastInterface` instead of
 * `SoftTmiInterface` saves 650-770 bytes of flash on an AVR processor.
 *
 * Word of caution: There is a use-case where the normal `SoftTmiInterface`
 * might consume less flash memory. If your application uses more than one
 * TM1637 LED Module, you will need to create multiple instances of the
 * `Tm1637Module`. But the pin numbers of this class must be a compile-time
 * constants, so different pins means that a different template class is
 * generated. Since the `Tm1637Module` class takes a `SoftTmiFastInterface` as
 * a template argument, each LED Module generate a new template instance of the
 * `Tm1637Module` class.
 *
 * When there are more than some number of TM1636 LED modules, it may actually
 * be more efficient to use the non-fast `SoftTmiInterface`, because you will
 * generate only a single template instantiation. I have not currently done any
 * experiments to see where the break-even point would be.
 *
 * On AVR processors, `delayMicroseconds()` is not accurate below 3
 * microseconds. Some microcontrollers may support better accuracy and may
 * work well with values as low as 1 microsecond.
 *
 * @tparam T_DIO_PIN pin attached to the data line
 * @tparam T_CLK_PIN pin attached to the clock line
 * @tparam T_DELAY_MICROS delay after each bit transition of DIO or CLK.. Should
 *    be greater or equal to 3 microseconds on AVR processors, but may work as
 *    low as 1 microsecond on other microcontrollers.
 */
template <
    uint8_t T_DIO_PIN,
    uint8_t T_CLK_PIN,
    uint8_t T_DELAY_MICROS
>
class SoftTmiFastInterface {
  public:
    explicit SoftTmiFastInterface() = default;

    /** Initialize the dio and clk pins.
     *
     * These are open-drain lines, with pull-up resistors. We must not drive
     * them HIGH actively since that could damage the transitor at the other
     * end of the line pulling LOW. Instead, we go into INPUT mode to let the
     * line to HIGH through the pullup resistor, then go to OUTPUT mode only
     * to pull down.
     */
    void begin() const {
      digitalWriteFast(T_CLK_PIN, LOW);
      digitalWriteFast(T_DIO_PIN, LOW);

      // Begin with both lines at HIGH.
      clockHigh();
      dataHigh();
    }

    /** Set dio and clk pins to INPUT mode. */
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
      // clock will always be LOW when this is called
      dataLow();
      clockHigh();
      dataHigh();
    }

    /**
     * Send the data byte on the data bus, with LSB first instead of the usual
     * MSB first for I2C.
     *
     * This loop generates slightly asymmetric logic signals because clockLow()
     * lasts for 2*bitDelay(), but clockHigh() lasts for only 1*bitDelay(). This
     * does not seem to cause any problems with the LED modules that I have
     * tested.
     *
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

      return readAck();
    }

  private:
    /**
     * Read the ACK/NACK bit from the device upon the falling edge of the 8th
     * CLK, which happens in the sendByte() loop above.
     */
    uint8_t readAck() const {
      // Go into INPUT mode, reusing dataHigh(), saving 6 flash bytes on AVR.
      dataHigh();
      uint8_t ack = digitalReadFast(T_DIO_PIN);

      // Device releases DIO upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

    // The following methods use compile-time constants from the template
    // parameters. The compiler will optimize away the 'this' pointer so that
    // these methods become identical to calling static functions.

    void bitDelay() const { delayMicroseconds(T_DELAY_MICROS); }

    void clockHigh() const { pinModeFast(T_CLK_PIN, INPUT); bitDelay(); }

    void clockLow() const { pinModeFast(T_CLK_PIN, OUTPUT); bitDelay(); }

    void dataHigh() const { pinModeFast(T_DIO_PIN, INPUT); bitDelay(); }

    void dataLow() const { pinModeFast(T_DIO_PIN, OUTPUT); bitDelay(); }
};

} // ace_segment

#endif // defined(ARDUINO_ARCH_AVR)

#endif
