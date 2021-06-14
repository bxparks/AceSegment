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

#ifndef ACE_SEGMENT_SIMPLE_WIRE_INTERFACE_H
#define ACE_SEGMENT_SIMPLE_WIRE_INTERFACE_H

#include <stdint.h>
#include <Arduino.h> // pinMode(), digitalWrite()

namespace ace_segment {

/**
 * A software I2C implementation for sending LED segment patterns over I2C. This
 * has the same API has HardWireInterface so it can be a drop-in replacement.
 *
 * The implementation is very similar to SoftTmiInterface because the TM1637
 * protocol is very similar to I2C. To keep everything simple, so the
 * beginTransmission(), write() and endTransimision() methods are *blocking*
 * calls because interrupts are not used. This means that we can eliminate the
 * send buffer, saving static memory.
 */
class SimpleWireInterface {
  public:
    /**
     * Constructor.
     * @param addr I2C address of slave device
     * @param dataPin SDA pin
     * @param clockPin SCL pin
     * @param delayMicros delay after each bit transition (full cycle = 2 *
     *    delayMicros)
     */
    SimpleWireInterface(
        uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t delayMicros
    ) :
        mAddr(addr),
        mDataPin(dataPin),
        mClockPin(clockPin),
        mDelayMicros(delayMicros)
    {}

    /** Initialize the clock and data pins. */
    void begin() {
      // These are open-drain lines, with a pull-up resistor. We must not drive
      // them HIGH actively since that could damage the transitor at the other
      // end of the line pulling LOW. Instead, we go into INPUT mode to let the
      // line to HIGH through the pullup resistor, then go to OUTPUT mode only
      // to pull down.
      digitalWrite(mClockPin, LOW);
      digitalWrite(mDataPin, LOW);

      // Begin with both lines at HIGH.
      clockHigh();
      dataHigh();
    }

    /** Set clock and data pins to INPUT mode. */
    void end() const {
      clockHigh();
      dataHigh();
    }

    /** Send start condition. */
    void beginTransmission() {
      clockHigh();
      dataHigh();

      dataLow();
      clockLow();

      // Send I2C addr (7 bits) and R/W bit set to "write" (0x00).
      uint8_t effectiveAddr = (mAddr << 1) | 0x00;
      write(effectiveAddr);
    }

    /**
     * Send the data byte on the data bus, with MSB first as specified by I2C.
     *
     * @return 0 means ACK, 1 means NACK.
     */
    uint8_t write(uint8_t data) {
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

      // Device places the ACK/NACK bit upon the falling edge of the 8th CLK,
      // which happens in the loop above.
      pinMode(mDataPin, INPUT);
      bitDelay();
      uint8_t ack = digitalRead(mDataPin);

      // Device releases SDA upon falling edge of the 9th CLK.
      clockHigh();
      clockLow();
      return ack;
    }

    /** Send stop condition. */
    void endTransmission() {
      dataLow();
      clockHigh();
      dataHigh();
    }

  private:
    void bitDelay() const { delayMicroseconds(mDelayMicros); }

    void clockHigh() const { pinMode(mClockPin, INPUT); bitDelay(); }

    void clockLow() const { pinMode(mClockPin, OUTPUT); bitDelay(); }

    void dataHigh() const { pinMode(mDataPin, INPUT); bitDelay(); }

    void dataLow() const { pinMode(mDataPin, OUTPUT); bitDelay(); }

  private:
    uint8_t const mAddr;
    uint8_t const mDataPin;
    uint8_t const mClockPin;
    uint8_t const mDelayMicros;
};

}

#endif
