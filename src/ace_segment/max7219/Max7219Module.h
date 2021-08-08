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

namespace ace_segment {

namespace internal {

/**
  * MAX7219 uses bit 0 for segment G, and bit 6 for segment A. This is the
  * reverse of the convention used by `LedModule`, and the reverse of the
  * TM1637. The weird thing is that the MAX7219 still uses bit 7 for the decimal
  * point. This method converts the normalized pattern used by `LedModule` into
  * the pattern expected by the MAX7219.
  *
  * This was pulled out of the Max7219Module class into a separate function
  * because Max7219Module is a template class, and I didn't want the compiler
  * generating extra copies of this function. Maybe the compiler (or linker) is
  * smart enough to optimize away the multiple copies, but by using this
  * separate function, I don't have to worry about the compiler doing the wrong
  * thing.
  *
  * TODO: This would be a great piece of code to rewrite in hand-optimized
  * assembly, for the learning experience.
  *
  */
inline uint8_t convertPatternMax7219(uint8_t pattern) {
  // Transfer the decimal point on bit 8.
  // Amazingly, the AVR compiler is able to implement this using bit shifting
  // operations so that no branching is performed.
  uint8_t result = (pattern & 0x80) ? 0x1 : 0x0;

  // Reverse the remaining 7 bits.
  for (uint8_t i = 0; i < 7; ++i) {
    result <<= 1;
    if (pattern & 0x1) {
      result |= 0x1;
    }

    // Division by 2 produces more efficient machine code on the AVR compiler (a
    // single `lsr` instruction) compared to the right-shift-operator. For some
    // reason, the right-shift operator (`pattern >>= 1`) produces code that
    // uses 8 extra bytes of extraneous instructions, casting the uint8_t into a
    // 16-bit word, setting the high byte to 0, then doing a `asr` and a `ror`,
    // then ignoring the high byte. For 32-bit processors, they have so much
    // flash memory that we don't need to optimize their code. But I suspect
    // that their compilers optimize the integer division by 2 just as well as
    // the AVR compiler.
    pattern /= 2;
  }

  return result;
}

} // internal

/**
 * A map of the physical digit position to its physical position. In other words
 * `logicalPos = kDigitRemapArray[physicalPos]`. Pass this array into the
 * Max7219Module constructor.
 *
 * The 8-digit MAX7219 LED modules that I bought on eBay and Amazon are wired
 * such that the digits appear as "7 6 5 4 3 2 1 0" intead of "0 1 2 3 4 5 6 7".
 * This is the reverse of the convention used by other `LedModule` classes.
 *
 * You can create your own remap array to handle other LED modules with
 * different physical ordering compared to the logical ordering.
 */
extern const uint8_t kDigitRemapArray8Max7219[8];

/**
 * An implementation of LedModule using the MAX7219 chip. The chip uses SPI.
 *
 * @tparam T_SPII class that implements the SPI interface, usually one of the
 *    classes in the AceSPI library: SimpleSpiInterface, SimpleSpiFastInterface,
 *    HardSpiInterface, HardSpiFastInterface.
 * @tparam T_DIGITS number of digits in the module
 */
template <typename T_SPII, uint8_t T_DIGITS>
class Max7219Module : public LedModule {
  public:
    /**
     * Constructor.
     * @param spiInterface instance of SPI interface class
     * @param remapArray (optional, nullable) a mapping of the physical digit
     *    positions to their logical positions
     */
    explicit Max7219Module(
        const T_SPII& spiInterface,
        const uint8_t* remapArray = nullptr
    ) :
        LedModule(mPatterns, T_DIGITS),
        mSpiInterface(spiInterface),
        mRemapArray(remapArray)
    {}

    //-----------------------------------------------------------------------
    // Initialization and termination.
    //-----------------------------------------------------------------------

    void begin() {
      LedModule::begin();

      memset(mPatterns, 0, T_DIGITS);

      // Set to a non-zero value to avoid using uninitialized value.
      setBrightness(1);

      // **WARNING**: Do NOT set this smaller than 3, or you may damage the
      // controller chip due to excessive current. See the MAX7219 datasheet for
      // details.
      mSpiInterface.send16(kRegisterScanLimit, 7); // scan all digits

      mSpiInterface.send16(kRegisterDecodeMode, 0); // no BCD decoding
      mSpiInterface.send16(kRegisterShutdown, 0x1); // turn on
    }

    void end() {
      mSpiInterface.send16(kRegisterShutdown, 0x0); // turn off

      LedModule::end();
    }

    //-----------------------------------------------------------------------
    // Methods related to rendering.
    //-----------------------------------------------------------------------

    /** Return true if flushing required. */
    bool isFlushRequired() const {
      return isAnyDigitDirty() || isBrightnessDirty();
    }

    /**
     * Send segment patterns of all digits. For a rough idea of how long
     * this function takes, here are the numbers on a 16 MHz AVR:
     *
     *  * HW SPI: 170 microseconds
     *  * SW SPI: 1800 microseconds
     *  * SW SPI Fast: 210 microseconds
     *
     * The isFlushRequired() method can be used to optimize the number of calls
     * to flush(), but often it is not necessary.
     */
    void flush() {
      for (uint8_t chipPos = 0; chipPos < T_DIGITS; ++chipPos) {
        // Remap the logical position used by the controller to the actual
        // position. For example, if the controller digit 0 appears at physical
        // digit 2, we need to display the segment pattern given by logical
        // position 2 when sending the byte to controller digit 0.
        uint8_t physicalPos = remapLogicalToPhysical(chipPos);
        uint8_t convertedPattern = internal::convertPatternMax7219(
            mPatterns[physicalPos]);
        mSpiInterface.send16(chipPos + 1, convertedPattern);
      }

      mSpiInterface.send16(kRegisterIntensity, getBrightness());

      clearDigitsDirty();
      clearBrightnessDirty();
    }

  private:
    /** Convert a logical position into its physical position. */
    uint8_t remapLogicalToPhysical(uint8_t pos) const {
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

    /**
     * SPI interface object. Copied by value instead of reference to avoid an
     * extra level of indirection.
     */
    const T_SPII mSpiInterface;

    /** Array to map digit addresses. */
    const uint8_t* const mRemapArray;

    /** Pattern for each digit. */
    uint8_t mPatterns[T_DIGITS];
};

}

#endif
