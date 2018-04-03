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

#include <Arduino.h>
#include <stdint.h>

namespace ace_segment {

class Hardware {
  public:
    /** Constructor. */
    Hardware() {}

    /** Write value to pin. */
    virtual void writePin(uint8_t pin, uint8_t value) {
      digitalWrite(pin, value);
    }

    /** Write analog value to pin. */
    virtual void writeAnalogPin(uint8_t pin, uint8_t value) {
      analogWrite(pin, value);
    }

    /** Set pin mode. */
    virtual void setPinMode(uint8_t pin, uint8_t mode) {
      pinMode(pin, mode);
    }

    /** Get the current micros  */
    virtual unsigned long getMicros() {
      return micros();
    }

    /** Get the current millis  */
    virtual unsigned long getMillis() {
      return millis();
    }

  private:
    // disable copy-constructor and assignment operator
    Hardware(const Hardware&) = delete;
    Hardware& operator=(const Hardware&) = delete;
};

}

#endif
