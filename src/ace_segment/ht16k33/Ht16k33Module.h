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

#ifndef ACE_SEGMENT_HT16K33_MODULE_H
#define ACE_SEGMENT_HT16K33_MODULE_H

#include <stdint.h>
#include <string.h> // memset()
#include "../LedModule.h"

namespace ace_segment {

/**
 * An implementation of LedModule using the HT16K33 chip. The chip uses SPI.
 *
 * @tparam T_WIREI the class that implements the SPI interface, usually
 *    either SoftSpiInterface or HardSpiInterface
 * @tparam T_DIGITS number of digits in the module
 */
template <typename T_WIREI, uint8_t T_DIGITS>
class Ht16k33Module : public LedModule {
  public:
    /**
     * Constructor.
     * @param wire instance of T_WIREI class
     * @param remapArray (optional, nullable) a mapping of the physical digit
     *    positions to their logical positions
     */
    explicit Ht16k33Module(const T_WIREI& wire) :
        LedModule(T_DIGITS),
        mWire(wire),
        mBrightness(1) // set to 1 to avoid using uninitialized value
    {}

    //-----------------------------------------------------------------------
    // Initialization and termination.
    //-----------------------------------------------------------------------

    void begin() {
      memset(mPatterns, 0, T_DIGITS);
      writeCommand(kSystemOn);
      writeCommand(kDisplayOn);
    }

    void end() {
      writeCommand(kDisplayOff);
      writeCommand(kSystemOff);
    }

    //-----------------------------------------------------------------------
    // Implement the LedModule interface
    //-----------------------------------------------------------------------

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return T_DIGITS; }

    void setPatternAt(uint8_t pos, uint8_t pattern) override {
      mPatterns[pos] = pattern;
    }

    uint8_t getPatternAt(uint8_t pos) override {
      return mPatterns[pos];
    }

    void setBrightness(uint8_t brightness) override {
      mBrightness = brightness & 0xF;
    }

    //-----------------------------------------------------------------------
    // Methods related to rendering.
    //-----------------------------------------------------------------------

    /**
     * Send segment patterns of all digits. Using the default 100kHz speed of
     * Wire, this takes about 1.2 millis to send 4 digits.
     *
     * On this particular LED module, the 4 LED digits are connected to 5 COM
     * lines (COM0 to COM4). The mapping is:
     *
     *    * digit 0 = COM0
     *    * digit 1 = COM1
     *    * colon = COM2
     *    * digit 2 = COM3
     *    * digit 3 = COM4
     *
     * Unlike some LED modules, it can display both the decimal point on digit 1
     * as well as the colon. However, various writers (e.g. ClockWriter) assumes
     * that the most-significant-bit of digit 1 is connected to the colon. So we
     * make the logical mapping so the physical COM line.
     */
    void flush() {
      mWire.beginTransmission();
      mWire.write(0x00); // start at position 0
      for (uint8_t chipPos = 0; chipPos < T_DIGITS; ++chipPos) {
        uint8_t pattern = mPatterns[chipPos];
        if (chipPos == 1) {
          // Strip the colon off Digit1 and send to COM1.
          pattern &= 0x7F;
        } else if (chipPos == 2) {
          // Logically connect the Digit1 colon bit (that was stripped above)
          // and send to the colon LED at COM2. It looks like the colon is
          // connected to ROW1 (i.e. 0x02) instead of ROW7 (i.e. 0x80) that I
          // would have expected.
          uint8_t hasColon = mPatterns[1] & 0x80;
          mWire.write(hasColon ? 0x02 : 0x00);
          mWire.write(0);
        }
        mWire.write(pattern); // row0-row7
        mWire.write(0); // row8-row15 unused
      }
      mWire.endTransmission();

      // TODO: Send out brightness
    }

  private:
    void writeCommand(uint8_t command) {
      mWire.beginTransmission();
      mWire.write(command);
      mWire.endTransmission();
    }

  private:
    static uint8_t const kSystemOff  = 0x20;
    static uint8_t const kSystemOn   = 0x21;
    static uint8_t const kDisplayOff = 0x80;
    static uint8_t const kDisplayOn  = 0x81;
    static uint8_t const kBrightness = 0xE0;

    /** SPI interface. */
    const T_WIREI& mWire;

    /** Pattern for each digit. */
    uint8_t mPatterns[T_DIGITS];

    /** Brightness 0 - 15 */
    uint8_t mBrightness;
};

}

#endif
