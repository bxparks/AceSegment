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

class Ht16k33ModuleTest_patternForChipPos_colonDisabled;
class Ht16k33ModuleTest_patternForChipPos_colonEnabled;

namespace ace_segment {

/**
 * An implementation of LedModule using the HT16K33 chip. The chip uses I2C
 * for communication.
 *
 * This class is customized for the 4-digit LED module from Adafruit
 * (https://www.adafruit.com/product/878) or one of its clones where the decimal
 * point after digit 1 (second from left) and the colon between digits 1 and 2
 * can be controlled independently and turned on at the same time.
 *
 * Unfortunately, the AceSegment library does *not* support controlling both the
 * decimal point and the colon at the same time. Instead, this class provides
 * the option of enabling one or the other, making the LED module behave like a
 * normal 4-digit LED module (witout a colon), or a 4-digit LED clock module
 * (with a colon). With the `enableColon` parameter set to false (default), the
 * decimal point (bit 7) of digit 1 controls the decimal point to the right of
 * digit 1. With `enableColon` set to true, the same bit controls the colon to
 * the right of digit 1.
 *
 * The behavior of the colon segment to be changed at runtime as well using the
 * enableColon() method. You can display the time of a clock (`hh:mm`), then
 * display a normal number with a decimal point (`xx.yy`).
 *
 * @tparam T_WIREI the class that wraps the I2C Wire interface (one of
 *    TwoWireInterface, SimpleWireInterface of SimpleWireFastInterface)
 * @tparam T_DIGITS number of logical digits in the module. Currently this
 *    should always be set to 4 because it is designed to support the 4-digit
 *    LED modules found on Adafruit, Amazon or eBay.
 */
template <typename T_WIREI, uint8_t T_DIGITS>
class Ht16k33Module : public LedModule {
  public:
    /**
     * Constructor.
     * @param wireInterface instance of T_WIREI class
     * @param addr the 7-bit I2C addr
     * @param enableColon enable the colon segment to behave like a 4-digit
     *    LED module for clocks (displaying a colon in `hh:mm`). The
     *    decimal point (bit 7) of digit 1 (second from left) becomes wired
     *    to the colon segment. (default: false)
     */
    explicit Ht16k33Module(
        T_WIREI& wireInterface,
        uint8_t addr,
        bool enableColon = false
    ) :
        LedModule(mPatterns, T_DIGITS),
        mWireInterface(wireInterface),
        mAddr(addr),
        mEnableColon(enableColon)
    {}

    //-----------------------------------------------------------------------
    // Initialization, termination, and configuration.
    //-----------------------------------------------------------------------

    void begin() {
      LedModule::begin();

      memset(mPatterns, 0, T_DIGITS);
      writeCommand(kSystemOn);
      writeCommand(kDisplayOn);
    }

    void end() {
      writeCommand(kDisplayOff);
      writeCommand(kSystemOff);

      LedModule::end();
    }

    /**
     * Set true to enable the colon segment on the module, which replaces the
     * decimal point on digit 1 (second from left). This has the same meaning as
     * the `enableColon` parameter in the constructor.
     *
     * You can enable the colon at runtime, just before the flush() method, to
     * render a `hh:mm` clock display. Then you can call this method to disable
     * the colon to enable the decimal point to render a `xx.yy` number.
     */
    void enableColon(bool enable) {
      mEnableColon = enable;
    }

    //-----------------------------------------------------------------------
    // Methods related to rendering.
    //-----------------------------------------------------------------------

    /** Return true if flushing required. */
    bool isFlushRequired() const {
      return isAnyDigitDirty() || isBrightnessDirty();
    }

    /**
     * Send segment patterns of all digits. Using the default 100kHz speed of
     * Wire, this takes about 1.2 millis to send 4 digits.
     *
     * On this particular LED module, the 4 logical LED digits are connected to
     * 5 COM lines (COM0 to COM4), i.e. 5 physical digits. The mapping is:
     *
     *    * digit 0 = COM0
     *    * digit 1 = COM1
     *    * colon = COM2 (bit 1)
     *    * digit 2 = COM3
     *    * digit 3 = COM4
     *
     * Unlike some LED modules, it can display both the decimal point on digit 1
     * as well as the colon segment at the same time. However, various writers
     * (e.g. ClockWriter) assumes that the most-significant-bit of digit 1 is
     * connected to the colon. So if enableColon is set, map the decimal point
     * bit to the colon bit.
     *
     * The isFlushRequired() method can be used to optimize the number of calls
     * to flush(), but often it is not necessary.
     */
    void flush() {
      // Write digits.
      mWireInterface.beginTransmission(mAddr);
      mWireInterface.write(0x00); // start at position 0
      // Loop over the 5 physical digit lines of this module.
      for (uint8_t chipPos = 0; chipPos < T_DIGITS + 1; ++chipPos) {
        uint8_t pattern = patternForChipPos(chipPos, mPatterns, mEnableColon);
        mWireInterface.write(pattern); // ROW0-ROW7
        mWireInterface.write(0); // ROW8-ROW15 unused
      }
      mWireInterface.endTransmission(false); // HT16K33 supports repeated START

      // Write brightness.
      writeCommand(getBrightness() | kBrightness);

      clearDigitsDirty();
      clearBrightnessDirty();
    }

  private:
    friend class ::Ht16k33ModuleTest_patternForChipPos_colonDisabled;
    friend class ::Ht16k33ModuleTest_patternForChipPos_colonEnabled;

    /** Write a single byte command to the LED module. */
    void writeCommand(uint8_t command) {
      mWireInterface.beginTransmission(mAddr);
      mWireInterface.write(command);
      mWireInterface.endTransmission();
    }

    /**
     * Return the segment pattern appropriate for the given physical digit
     * position (COM{N}}. This function is static for unit testing purposes.
     */
    static uint8_t patternForChipPos(
        uint8_t chipPos, const uint8_t patterns[], bool enableColon) {
      switch (chipPos) {
        case 0:
          return patterns[0];
        case 1: {
          uint8_t pattern = patterns[1];
          return (enableColon) ? pattern & 0x7F : pattern;
        }
        case 2: {
          if (enableColon) {
            // Connect the colon bit 7 on logical digit 1 (that was stripped
            // above) and send to the colon segment at physical digit COM2. It
            // looks like on the 4-digit HT16K33 modules that I got, the colon
            // is connected to ROW1 (i.e. 0x02) instead of ROW7 (i.e. 0x80) that
            // I would have expected.
            bool hasColon = patterns[1] & 0x80;
            return hasColon ? 0x02 : 0x00;
          } else {
            return 0;
          }
        }
        case 3:
          return patterns[2];
        case 4:
          return patterns[3];
        default:
          return 0;
      }
    }

  private:
    static uint8_t const kSystemOff  = 0x20;
    static uint8_t const kSystemOn   = 0x21;
    static uint8_t const kDisplayOff = 0x80;
    static uint8_t const kDisplayOn  = 0x81;
    static uint8_t const kBrightness = 0xE0;

    /**
     * I2C Wire interface. Copied by value instead of reference to avoid an
     * extra layer of indirection.
     */
    T_WIREI mWireInterface;

    /** The 7-bit I2C address. */
    uint8_t const mAddr;

    /** Pattern for each digit. */
    uint8_t mPatterns[T_DIGITS];

    /** Enable colon. */
    bool mEnableColon;
};

}

#endif
