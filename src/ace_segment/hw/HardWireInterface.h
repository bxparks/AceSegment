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

#ifndef ACE_SEGMENT_HARD_WIRE_INTERFACE_H
#define ACE_SEGMENT_HARD_WIRE_INTERFACE_H

#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

namespace ace_segment {

class HardWireInterface {
  public:
    HardWireInterface(uint8_t addr) : mAddr(addr) {}

    void begin() const {}

    void end() const {}

    void beginTransmission() const {
      Wire.beginTransmission(mAddr);
    }

    void write(uint8_t data) const {
      Wire.write(data);
    }

    void endTransmission() const {
      Wire.endTransmission();
    }

  private:
    uint8_t const mAddr;
};

}

#endif
