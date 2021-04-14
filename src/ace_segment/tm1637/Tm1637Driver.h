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

#ifndef ACE_SEGMENT_TM1637_DRIVER_H
#define ACE_SEGMENT_TM1637_DRIVER_H

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
 *
 * Since the protocol does not match I2C, we cannot use the hardware I2C
 * capabilities of the microcontroller, so we have to implement a software
 * version of this protocol.
 *
 * This class is stateless. It is thread-safe.
 */
class Tm1637Driver {
  public:
    explicit Tm1637Driver(
        uint8_t clockPin,
        uint8_t dioPin,
        uint16_t delayMicros
    ) :
        mClockPin(clockPin),
        mDioPin(dioPin),
        mDelayMicros(delayMicros)
    {}

    void begin() const {
      // These are open-drain lines, with a pull-up resistor. We must not drive
      // them HIGH actively since that could damage the transitor at the other
      // end of the line pulling LOW. Instead, we go into INPUT mode to let the
      // line to HIGH through the pullup resistor, then go to OUTPUT mode only
      // to pull down.
      digitalWrite(mClockPin, LOW);
      digitalWrite(mDioPin, LOW);

      // Begin with both lines at HIGH.
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
      pinMode(mDioPin, INPUT);
      bitDelay();
      uint8_t ack = digitalRead(mDioPin);

      // Device releases DIO upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

  private:
    void bitDelay() const { delayMicroseconds(mDelayMicros); }

    void clockHigh() const { pinMode(mClockPin, INPUT); bitDelay(); }

    void clockLow() const { pinMode(mClockPin, OUTPUT); bitDelay(); }

    void dataHigh() const { pinMode(mDioPin, INPUT); bitDelay(); }

    void dataLow() const { pinMode(mDioPin, OUTPUT); bitDelay(); }

  private:
    uint8_t const mClockPin;
    uint8_t const mDioPin;
    uint16_t const mDelayMicros;
};

} // ace_segment

#endif
