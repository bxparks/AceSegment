/*
MIT License

Copyright (c) 2022 Brian T. Park

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

#ifndef ACE_SEGMENT_TM1638_MODULE_H
#define ACE_SEGMENT_TM1638_MODULE_H

#include <stdint.h>
#include <string.h> // memset()
#include <Arduino.h> // delayMicroseconds()
#include "../LedModule.h"

class Tm1638ModuleTest_flushIncremental;
class Tm1638ModuleTest_flush;

namespace ace_segment {

/**
 * According to the TM1638 datasheet, the chip should be able to handle a
 * clock of 500 kHz, or a half-cycle bit delay of 1 microseconds.
 */
static const uint16_t kDefaultTm1638DelayMicros = 1;

/**
 * A map of the logical digit position to its physical position. In other words
 * `physicalPos = kDigitRemapArray[logicalPos]`. Pass this array into the
 * Tm1638Module constructor. This was not needed for the TM1638 module that I
 * got on Amazon, but other modules may require a custom version of this.
 *
 * You can create your own remap array to handle other LED modules with
 * different physical ordering compared to the logical ordering.
 */
extern const uint8_t kDigitRemapArray8Tm1638[8];

/**
 * An implementation of LedModule using the TM1638 chip. The chip communicates
 * using a protocol that is electrically similar to SPI.
 *
 * @tparam T_TMII class that implements the three wire SPI-like protocol
 *    interface for TM1638, usually one of the classes from the AceTMI library:
 *    SimpleTmi1638Interface or SimpleTmi1638FastInterface.
 * @tparam T_DIGITS number of digits in the LED module (usually 8)
 */
template <typename T_TMII, uint8_t T_DIGITS>
class Tm1638Module : public LedModule {
  public:

    /**
     * Constructor.
     * @param tmiInterface instance of TM1638 interface class
     * @param remapArray (optional, nullable) a mapping of the logical digit
     *    positions to their physical positions, useful for 8-digt LED modules
     *    whose digits are wired out of order
     */
    explicit Tm1638Module(
        const T_TMII& tmiInterface,
        const uint8_t* remapArray = nullptr
    ) :
        LedModule(mPatterns, T_DIGITS),
        mTmiInterface(tmiInterface),
        mRemapArray(remapArray)
    {}

    //-----------------------------------------------------------------------
    // Initialization and termination.
    //-----------------------------------------------------------------------

    /**
     * Initialize the module. The SimpleTmi1638Interface or
     * SimpleTmi1638FastInterface object must be initialized separately.
     */
    void begin() {
      LedModule::begin();

      memset(mPatterns, 0, T_DIGITS);
      setDisplayOn(true);
    }

    /** Signal end of usage. Currently does nothing. */
    void end() {
      LedModule::end();
    }

    //-----------------------------------------------------------------------
    // Additional brightness control supported by the TM1638 chip.
    //-----------------------------------------------------------------------

    /**
     * Turn off the entire display. The brightness is not affected so when it is
     * turned back on, the previous brightness will be used.
     */
    void setDisplayOn(bool on = true) {
      mDisplayOn = on;
      setBrightness(getBrightness()); // mark the brightness dirty
    }

    //-----------------------------------------------------------------------
    // Methods related to rendering.
    //-----------------------------------------------------------------------

    /** Return true if flushing required. */
    bool isFlushRequired() const {
      return isAnyDigitDirty() || isBrightnessDirty();
    }

    /**
     * Send segment patterns of all digits plus the brightness to the display.
     *
     * Performance, for sending 8 digits (total of 1+1+16+1 = 19 bytes), using
     * a 1 microsecond delay, on an SparkFun Pro Micro (AVR):
     *
     *    * SimpleTm1638Interface, using digitalWrite()
     *        * flush() takes 2.6 ms
     *        * effective speed of 58 kbps
     *    * SimpleTm1638FastInterface, using digitalWriteFast()
     *        * flush() takes 0.3 ms
     *        * effective speed of 500 kbps
     *
     * The isFlushRequired() method can be used to optimize the number of calls
     * to flush(), but often it is not necessary.
     */
    void flush() {
      // Command1: Update the digits using auto incrementing mode.
      mTmiInterface.beginTransaction();
      mTmiInterface.write(kDataCmdAutoAddress);
      mTmiInterface.endTransaction();

      // Command2: Send the LED patterns.
      mTmiInterface.beginTransaction();
      mTmiInterface.write(kAddressCmd);
      for (uint8_t chipPos = 0; chipPos < T_DIGITS; ++chipPos) {
        // Remap the logical position used by the controller to the actual
        // position. For example, if the controller digit 0 appears at physical
        // digit 2, we need to display the segment pattern given by logical
        // position 2 when sending the byte to controller digit 0.
        uint8_t physicalPos = remapLogicalToPhysical(chipPos);
        uint8_t effectivePattern = mPatterns[physicalPos];
        mTmiInterface.write(effectivePattern);
        mTmiInterface.write(0x00); // SEG8 and SEG9 not supported in this class
      }
      mTmiInterface.endTransaction();

      // Command3: Update the brightness last. This matches the recommendation
      // given in the Titan Micro TM1638 datasheet. But experimentation shows
      // that things seems to work even if brightness is sent first, before the
      // digit patterns.
      mTmiInterface.beginTransaction();
      mTmiInterface.write(kBrightnessCmd
          | (mDisplayOn ? kBrightnessLevelOn : 0x0)
          | (getBrightness() & 0xF));
      mTmiInterface.endTransaction();

      clearDigitsDirty();
      clearBrightnessDirty();
    }

    //-----------------------------------------------------------------------
    // Methods related to buttons
    //-----------------------------------------------------------------------

    /**
     * Read the 4 bytes with key information. Use little-endian ordering since
     * the bits in each byte come out of the device in LSBFIRST order. In other
     * words, first byte is the least signficant byte of the 32-bit result, and
     * bit0 of the first byte is the first bit that streamed out of the device.
     */
    uint32_t readButtons() const {
      mTmiInterface.beginTransaction();
      mTmiInterface.write(kDataCmdReadKeys);

      // The datasheet says that at least 2 micros are needed between the
      // write() and the read(). On some microcontrollers (e.g. AVR), the
      // accuracy of delayMicroseconds() is terrible for small values. So let's
      // use 3 micros just in case.
      delayMicroseconds(3);

      uint8_t byte1 = mTmiInterface.read();
      uint8_t byte2 = mTmiInterface.read();
      uint8_t byte3 = mTmiInterface.read();
      uint8_t byte4 = mTmiInterface.read();
      mTmiInterface.endTransaction();

      uint32_t data = ((uint32_t) byte4 << 24)
          | ((uint32_t) byte3 << 16)
          | ((uint32_t) byte2 << 8)
          | (uint32_t) byte1;
      return data;
    }

  private:
    /** Convert a logical position into the physical position. */
    uint8_t remapLogicalToPhysical(uint8_t pos) const {
      return mRemapArray ? mRemapArray[pos] : pos;
    }

  private:
    // Give access to mIsDirty.
    friend class ::Tm1638ModuleTest_flush;

    // These come from the TM1638 controller chip datasheet.
    static uint8_t const kDataCmdWriteDisplay = 0b01000000;
    static uint8_t const kDataCmdReadKeys =     0b01000010;
    static uint8_t const kDataCmdAutoAddress =  0b01000000;
    static uint8_t const kDataCmdFixedAddress = 0b01000100;
    static uint8_t const kAddressCmd =          0b11000000;
    static uint8_t const kBrightnessCmd =       0b10000000;
    static uint8_t const kBrightnessLevelOn =   0b00001000;

    // The ordering of these fields is partially determined to save memory on
    // 32-bit processors.

    // TM1638 interface object. Copied by value instead of reference to avoid an
    // extra level of indirection.
    const T_TMII mTmiInterface;

    const uint8_t* const mRemapArray;
    uint8_t mPatterns[T_DIGITS];
    bool mDisplayOn;
};

} // ace_segment

#endif
