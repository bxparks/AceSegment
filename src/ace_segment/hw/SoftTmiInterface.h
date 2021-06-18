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

#ifndef ACE_SEGMENT_SOFT_TMI_INTERFACE_H
#define ACE_SEGMENT_SOFT_TMI_INTERFACE_H

#include <Arduino.h>

namespace ace_segment {

/**
 * Class that knows how to communicate with a TM1637 chip. It uses a 2-wire
 * (Clock and a bidirectional DIO) protocol that is similar to I2C electrically.
 * Both the Clock and Data pins are open-drain which means a single transitor on
 * either the master or slave can pull the line LOW, but a pull-up resisotr is
 * required to set the line HIGH. Because these are open-drain, we must make
 * sure that the microcontroller does *not* actively drive these lines HIGH,
 * otherwise, the output pin of the MCU at 5V (HIGH) becomes directly connected
 * to the 0V (LOW) of the transistor on the device pulling it LOW, with no
 * current limiting resistor. Either of MCU or the device can become damaged. To
 * set the line HIGH or LOW, we set the output level to LOW, then use the
 * pinMode() function to either INPUT (to get a HIGH value) or OUTPUT (to get a
 * LOW value).
 *
 * The logical protocol of the TM1637 is similar to I2C in the following ways:
 *
 *    * The start and stop conditions are the same.
 *    * Data transfer happens on the rising edge of the CLK signal.
 *    * The slave sends back a one-bit ACK/NACK after the 8th bit of the CLK.
 *
 * The difference is:
 *
 *    * There is no I2C address byte, so only a single TM1637 device can be on
 *      the bus.
 *    * The first byte sent to the TM1637 is a command byte.
 *    * The bytes are sent LSB first instead of the usual MSB first on I2C.
 *
 * Since the protocol does not match I2C, we cannot use the hardware I2C
 * capabilities of the microcontroller, so we have to implement a software
 * version of this protocol.
 */
class SoftTmiInterface {
  public:
    /**
     * Constructor.
     *
     * On AVR processors, `delayMicroseconds()` is not accurate below 3
     * microseconds. Some microcontrollers may support better accuracy and may
     * work well with values as low as 1 microsecond.
     *
     * @tparam dioPin pin attached to the data line
     * @tparam clkPin pin attached to the clock line
     * @param delayMicros delay after each bit transition of DIO or CLK. Should
     *    be greater or equal to 3 microseconds on AVR processors, but may work
     *    as low as 1 microsecond on other microcontrollers.
     */
    explicit SoftTmiInterface(
        uint8_t dioPin,
        uint8_t clkPin,
        uint8_t delayMicros
    ) :
        mDioPin(dioPin),
        mClkPin(clkPin),
        mDelayMicros(delayMicros)
    {}

    /**
     * Initialize the dio and clk pins.
     *
     * These are open-drain lines, with pull-up resistors. We must not drive
     * them HIGH actively since that could damage the transitor at the other
     * end of the line pulling LOW. Instead, we go into INPUT mode to let the
     * line to HIGH through the pullup resistor, then go to OUTPUT mode only
     * to pull down.
     */
    void begin() const {
      digitalWrite(mClkPin, LOW);
      digitalWrite(mDioPin, LOW);

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
      // Go into INPUT mode, reusing dataHigh(), saving 10 flash bytes on AVR.
      dataHigh();
      uint8_t ack = digitalRead(mDioPin);

      // Device releases DIO upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

    void bitDelay() const { delayMicroseconds(mDelayMicros); }

    void clockHigh() const { pinMode(mClkPin, INPUT); bitDelay(); }

    void clockLow() const { pinMode(mClkPin, OUTPUT); bitDelay(); }

    void dataHigh() const { pinMode(mDioPin, INPUT); bitDelay(); }

    void dataLow() const { pinMode(mDioPin, OUTPUT); bitDelay(); }

  private:
    uint8_t const mDioPin;
    uint8_t const mClkPin;
    uint8_t const mDelayMicros;
};

} // ace_segment

#endif
