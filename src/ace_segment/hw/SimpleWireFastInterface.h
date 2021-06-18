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

#ifndef ACE_SEGMENT_SIMPLE_WIRE_FAST_INTERFACE_H
#define ACE_SEGMENT_SIMPLE_WIRE_FAST_INTERFACE_H

// This header file requires the digitalWriteFast library on AVR, or the
// EpoxyMockDigitalWriteFast library on EpoxyDuino.
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)

#include <stdint.h>
#include <Arduino.h> // delayMicroseconds()

namespace ace_segment {

/**
 * A version of SimpleWireInterface that uses one of the <digitalWriteFast.h>
 * libraries. The biggest benefit of using digitalWriteFast is the reduction of
 * flash memory size, 500-700 bytes on AVR.
 *
 * On AVR processors, `delayMicroseconds()` is not accurate below 3
 * microseconds. I am not sure about the accuracy on other microcontrollers, but
 * it is probably prudent to keep T_DELAY_MICROS greater than or equal to 3.
 *
 * @tparam T_DATA_PIN SDA pin
 * @tparam T_CLOCK_PIN SCL pin
 * @tparam T_DELAY_MICROS delay after each bit transition, should be greater
 *    or equal to 3 microseconds.
 */
template <
    uint8_t T_DATA_PIN,
    uint8_t T_CLOCK_PIN,
    uint8_t T_DELAY_MICROS
>
class SimpleWireFastInterface {
  public:
    /** Constructor. */
    SimpleWireFastInterface() = default;

    /**
     * Initialize the clock and data pins.
     *
     * These are open-drain lines, with pull-up resistors. We must not drive
     * them HIGH actively since that could damage the transitor at the other
     * end of the line pulling LOW. Instead, we go into INPUT mode to let the
     * line to HIGH through the pullup resistor, then go to OUTPUT mode only
     * to pull down.
     */
    void begin() const {
      digitalWriteFast(T_CLOCK_PIN, LOW);
      digitalWriteFast(T_DATA_PIN, LOW);

      // Begin with both lines in INPUT mode to passively go HIGH.
      clockHigh();
      dataHigh();
    }

    /** Set clock and data pins to INPUT mode. */
    void end() const {
      clockHigh();
      dataHigh();
    }

    /**
     * Send start condition.
     * @param addr I2C address of slave device
     */
    void beginTransmission(uint8_t addr) const {
      clockHigh();
      dataHigh();

      dataLow();
      clockLow();

      // Send I2C addr (7 bits) and R/W bit set to "write" (0x00).
      uint8_t effectiveAddr = (addr << 1) | 0x00;
      write(effectiveAddr);
    }

    /** Send stop condition. */
    void endTransmission() const {
      // clock will always be LOW when this is called
      dataLow();
      clockHigh();
      dataHigh();
    }

    /**
     * Send the data byte on the data bus, with MSB first as specified by I2C.
     *
     * This loop generates slightly asymmetric logic signals because clockLow()
     * lasts for 2*bitDelay(), but clockHigh() lasts for only 1*bitDelay(). This
     * does not seem to cause any problems with the LED modules that I have
     * tested.
     *
     * @return 0 means ACK, 1 means NACK.
     */
    uint8_t write(uint8_t data) const {
      for (uint8_t i = 0;  i < 8; ++i) {
        if (data & 0x80) {
          dataHigh();
        } else {
          dataLow();
        }
        clockHigh();
        clockLow();
        data <<= 1;
      }

      return readAck();
    }

  private:
    /**
     * Read the ACK/NACK bit from the device upon the falling edge of the 8th
     * CLK, which happens in the write() loop above.
     */
    uint8_t readAck() const {
      // Go into INPUT mode, reusing dataHigh(), saving 10 flash bytes on AVR.
      dataHigh();
      uint8_t ack = digitalReadFast(T_DATA_PIN);

      // Device releases SDA upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

    // The following methods use compile-time constants from the template
    // parameters. The compiler will optimize away the 'this' pointer so that
    // these methods become identical to calling static functions.

    void bitDelay() const { delayMicroseconds(T_DELAY_MICROS); }

    void clockHigh() const { pinModeFast(T_CLOCK_PIN, INPUT); bitDelay(); }

    void clockLow() const { pinModeFast(T_CLOCK_PIN, OUTPUT); bitDelay(); }

    void dataHigh() const { pinModeFast(T_DATA_PIN, INPUT); bitDelay(); }

    void dataLow() const { pinModeFast(T_DATA_PIN, OUTPUT); bitDelay(); }
};

}

#endif // defined(ARDUINO_ARCH_AVR)

#endif
