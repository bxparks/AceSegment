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

#ifndef ACE_SEGMENT_MAX7219_MODULE_H
#define ACE_SEGMENT_MAX7219_MODULE_H

#include <stdint.h>
#include <string.h> // memset()
#include "../LedModule.h"

class Max7219ModuleTest_convertPattern;

namespace ace_segment {

/**
 * The 8-digit MAX7219 LED modules that I bought on eBay and Amazon are wired
 * such that digit 0 is the *right* most digit, and digit 7 is the *left* most
 * digit. This is the reverse of the convention used by the `LedDisplay` class.
 * Use this remap array to reverse the digit addresses by passing it into the
 * Max7219Module constructor.
 *
 * You can create your own remap array to handle other LED modules with
 * different physical ordering compared to the logical ordering.
 */
extern const uint8_t kEightDigitRemapArray[8];

template <typename SPII, uint8_t DIGITS>
class Max7219Module : public LedModule {
  public:
    /** Constructor */
    Max7219Module(
        const SPII& spiInterface,
        const uint8_t* remapArray = nullptr
    ) :
        LedModule(DIGITS),
        mSpiInterface(spiInterface),
        mRemapArray(remapArray)
    {}

    //-----------------------------------------------------------------------
    // Initialization and termination.
    //-----------------------------------------------------------------------

    void begin() {
      memset(mPatterns, 0, DIGITS);

      mSpiInterface.send16(kRegisterScanLimit, 7); // all digits
      mSpiInterface.send16(kRegisterDecodeMode, 0); // no BCD decoding
      mSpiInterface.send16(kRegisterShutdown, 0x1); // turn on
    }

    void end() {
      mSpiInterface.send16(kRegisterShutdown, 0x0); // turn off
    }

    //-----------------------------------------------------------------------
    // Implement the LedModule interface
    //-----------------------------------------------------------------------

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return DIGITS; }

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
     * Send segment patterns of all digits. For a rough idea of how long
     * this function takes, here are the numbers on a 16 MHz AVR:
     *
     *  * HW SPI: 170 microseconds
     *  * SW SPI: 1800 microseconds
     *  * SW SPI Fast: 210 microseconds
     */
    void flush() {
      for (uint8_t i = 0; i < DIGITS; ++i) {
        uint8_t actualPos = remapDigit(i);
        uint8_t convertedPattern = convertPattern(mPatterns[i]);
        mSpiInterface.send16(actualPos + 1, convertedPattern);
      }

      mSpiInterface.send16(kRegisterIntensity, mBrightness);
    }

  private:
    friend class ::Max7219ModuleTest_convertPattern;

    /**
     * MAX7219 uses bit 0 for segment G, and bit 6 for segment A. This is
     * the reverse of what I would normally expect, certainly the reverse of the
     * TM1637. The weird thing is that the MAX7219 still uses bit 7 for the
     * decimal point. This method converts the normalized pattern into the
     * pattern expected by the MAX7219.
     */
    static uint8_t convertPattern(uint8_t pattern) {
      uint8_t result = 0;

      // Reverse the first 7 bits.
      for (uint8_t i = 0; i < 7; ++i) {
        result <<= 1;
        if (pattern & 0x1) {
          result |= 0x1;
        }
        pattern >>= 1;
      }

      // Transfer the decimal point on bit 8.
      if (pattern & 0x1) {
        result |= 0x80;
      }

      return result;
    }

    /** Convert a logical position into the physical position. */
    uint8_t remapDigit(uint8_t pos) const {
      return mRemapArray ? mRemapArray[pos] : pos;
    }

  private:
    static uint8_t const kRegisterNoop        = 0x00;
    static uint8_t const kRegisterDigit0      = 0x01;
    static uint8_t const kRegisterDigit1      = 0x02;
    static uint8_t const kRegisterDigit2      = 0x03;
    static uint8_t const kRegisterDigit3      = 0x04;
    static uint8_t const kRegisterDigit4      = 0x05;
    static uint8_t const kRegisterDigit5      = 0x06;
    static uint8_t const kRegisterDigit6      = 0x07;
    static uint8_t const kRegisterDigit7      = 0x08;
    static uint8_t const kRegisterDecodeMode  = 0x09;
    static uint8_t const kRegisterIntensity   = 0x0A;
    static uint8_t const kRegisterScanLimit   = 0x0B;
    static uint8_t const kRegisterShutdown    = 0x0C;
    static uint8_t const kRegisterDisplayTest = 0x0F;

    /** SPI interface. */
    const SPII& mSpiInterface;

    /** Array to map digit addresses. */
    const uint8_t* const mRemapArray;

    /** Pattern for each digit. */
    uint8_t mPatterns[DIGITS];

    /** Brightness 0 - 15 */
    uint8_t mBrightness;
};

}

#endif
