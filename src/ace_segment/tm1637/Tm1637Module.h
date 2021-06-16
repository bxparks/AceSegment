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

#ifndef ACE_SEGMENT_TM1637_MODULE_H
#define ACE_SEGMENT_TM1637_MODULE_H

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include "../LedModule.h"

class Tm1637ModuleTest_flushIncremental;
class Tm1637ModuleTest_flush;

namespace ace_segment {

/**
 * According to the TM1637 datasheet, the chip should be able to handle fairly
 * small delays, like 2 microseconds, giving a 250kHz clock cycle (fullcycle = 2
 * * 2 microseconds). But many TM1637 LED Modules from eBay apparently use
 * capacitors which are far too large (~10 nF, instead of ~100 pF). So the rise
 * time of the signal on these lines is too slow, and we need to use a very
 * large delay between bits. A value of 50 microseconds does not work on my LED
 * Modules, but 100 microseconds does. If you remove the 10nF capacitors on the
 * DIO and CLK lines, then you can use a 2 microsecond bit delay.
 */
static const uint16_t kDefaultTm1637DelayMicros = 100;

/**
 * A map of the logical digit position to its physical position. In other words
 * `physicalPos = kDigitRemapArray[logicalPos]`. Pass this array into the
 * Tm1637Module constructor.
 *
 * Many (if not all) of the 6-digit LED modules on eBay and Amazon using the
 * TM1637 chip are wired so that the digits appear in the order of "2 1 0 5 4 3"
 * instead of "0 1 2 3 4 5". Not sure why since the 4-digit LED modules seem to
 * follow the natural order.
 *
 * You can create your own remap array to handle other LED modules with
 * different physical ordering compared to the logical ordering.
 */
extern const uint8_t kDigitRemapArray6Tm1637[6];

namespace internal {

/**
 * Return a bit mask that sets the lower `numBits` bits to 1. For example, if
 * `numBits=4`, then we want 0b00001111 or 0x0F. The general expression to get
 * this result is `2^N - 1`. The exponential operation can be performed using a
 * left bit-shift. I believe this expression is well-defined in the C++ language
 * even if `numBits == 8` because `(1 << 8)` is 0x100, and `0x100 - 1` is `0xFF`
 * using unsigned integer arithmetics.
 */
constexpr uint8_t initialDirtyBits(uint8_t numBits) {
  return ((uint16_t) 0x1 << numBits) - 1;
}

} // namespace internal

/**
 * An implementation of LedModule using the TM1637 chip. The chip communicates
 * using a protocol that is electrically similar to I2C, but does not use an
 * address byte at the beginning of the protocol. We can use a software-based
 * I2C interface.
 *
 * @tparam T_TMII two wire protocol interface for TM1637, either
 *    SoftTmiInterface or SoftTmiFastInterface
 * @tparam T_DIGITS number of digits in the LED module (usually 4 or 6)
 */
template <typename T_TMII, uint8_t T_DIGITS>
class Tm1637Module : public LedModule {
  public:

    /**
     * Constructor.
     * @param tmiInterface instance of either SoftTmiInterface or
     *    SoftTmiFastInterface
     * @param remapArray (optional, nullable) a mapping of the logical digit
     *    positions to their physical positions, useful for 6-digt LED modules
     *    using the TM1637 chip whose digits are wired out of order
     */
    explicit Tm1637Module(
        const T_TMII& tmiInterface,
        const uint8_t* remapArray = nullptr
    ) :
        LedModule(T_DIGITS),
        mTmiInterface(tmiInterface),
        mRemapArray(remapArray)
    {}

    //-----------------------------------------------------------------------
    // Initialization and termination.
    //-----------------------------------------------------------------------

    /**
     * Initialize the module. The SoftTmiInterface object must be initialized
     * separately.
     *
     * @param remapArray optional array of positions to handle LED modules whose
     *    digit ordering is physically different than the logical ordering
     *    (where digit 0 is on the left, and digit (T_DIGITS-1) is on the far
     *    right).
     */
    void begin() {
      memset(mPatterns, 0, T_DIGITS);
      mBrightness = kBrightnessCmd | kBrightnessLevelOn | 0x7;

      // Initially, we want to set all the dirty bits for digits and brightness,
      // so that they are sent to the LED module upon the first flush() or
      // flushIncremental(). The number of dirty bits required is `T_DIGITS +
      // 1` because we use an extra bit for the brightness.
      mIsDirty = internal::initialDirtyBits(T_DIGITS + 1);

      mFlushStage = 0;
    }

    /** Signal end of usage. Currently does nothing. */
    void end() {}

    //-----------------------------------------------------------------------
    // Implement the LedModule interface
    //-----------------------------------------------------------------------

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return T_DIGITS; }

    void setPatternAt(uint8_t pos, uint8_t pattern) override {
      mPatterns[pos] = pattern;
      setDirtyBit(pos);
    }

    uint8_t getPatternAt(uint8_t pos) override {
      return mPatterns[pos];
    }

    void setBrightness(uint8_t brightness) override {
      mBrightness = (mBrightness & ~0x7) | (brightness & 0x7);
      setDirtyBit(kBrightnessDirtyBit);
    }

    //-----------------------------------------------------------------------
    // Additional brightness control supported by the TM1637 chip.
    //-----------------------------------------------------------------------

    /**
     * Turn off the entire display. The brightness is not affected so when it is
     * turned back on, the previous brightness will be used.
     */
    void setDisplayOn(bool on = true) {
      if (on) {
        mBrightness |= kBrightnessLevelOn;
      } else {
        mBrightness &= ~kBrightnessLevelOn;
      }
      setDirtyBit(kBrightnessDirtyBit);
    }

    //-----------------------------------------------------------------------
    // Methods related to rendering.
    //-----------------------------------------------------------------------

    /**
     * Send segment patterns of all digits plus the brightness to the display,
     * if any of the digits or brightness is dirty. Takes about 22 ms using a
     * 100 microsecond delay.
     */
    void flush() {
      if (! mIsDirty) return;

      // Update the brightness first
      mTmiInterface.startCondition();
      mTmiInterface.sendByte(mBrightness);
      mTmiInterface.stopCondition();

      // Update the digits using auto incrementing mode.
      mTmiInterface.startCondition();
      mTmiInterface.sendByte(kDataCmdAutoAddress);
      mTmiInterface.stopCondition();

      mTmiInterface.startCondition();
      mTmiInterface.sendByte(kAddressCmd);
      for (uint8_t chipPos = 0; chipPos < T_DIGITS; ++chipPos) {
        // Remap the logical position used by the controller to the actual
        // position. For example, if the controller digit 0 appears at physical
        // digit 2, we need to display the segment pattern given by logical
        // position 2 when sending the byte to controller digit 0.
        uint8_t physicalPos = remapLogicalToPhysical(chipPos);
        uint8_t effectivePattern = mPatterns[physicalPos];
        mTmiInterface.sendByte(effectivePattern);
      }
      mTmiInterface.stopCondition();

      mIsDirty = 0x0;
    }

    /**
     * Update only a single digit or the brightness. This method must be called
     * (T_DIGITS + 1) times to update the digits of entire module, including the
     * brightness which is updated using a separate step. Uses the mFlushStage
     * and the mIsDirty bit array to update only the part that needs updating.
     *
     * This method should be used if the processor cannot be blocked for the
     * entire duration of the flush() method (e.g. on the ESP8266, which will
     * cause a WDT reset when it is blocked for more than 20-40 ms).
     *
     * Using 100 micro delay, I see the following durations:
     *
     * 1) If brightness is updated on every iteration, I get
     * 'min/avg/max:4/494/13780', so a maximum of 14 ms, which is still a little
     * bit high.
     *
     * 2) If brightness is updated during its own mFlushStage (== T_DIGITS),
     * then I see `min/avg/max:4/492/10152`, saving about 3.5ms from the
     * latency. The side effect is a slightly flicker when the display and
     * brightness changes at the same time, because this incrementally updating
     * function makes those changes in 2 steps.
     *
     * The incremental flushing must use fixed addressing mode to write specific
     * digits, which adds extra commands to the wire protocol to the LED module.
     * If this algorithm is used to send all the digits in one-shot, then this
     * method is about 50% slower (30 ms), compared to flush() (22 ms).
     *
     * Technical note: The TM1637 datasheet seems to suggest that the brightness
     * must always be sent after a set of digits are sent. However,
     * experimentation shows that the brightness can be sent as an independent
     * transimission, so this method splits out each digit and the brightness as
     * separate iterations.
     */
    void flushIncremental() {
      if (mFlushStage == T_DIGITS) {
        // Update brightness.
        if (isDirtyBit(T_DIGITS)) {
          mTmiInterface.startCondition();
          mTmiInterface.sendByte(mBrightness);
          mTmiInterface.stopCondition();
          clearDirtyBit(T_DIGITS);
        }
      } else {
        // Remap the logical position used by the controller to the actual
        // position. For example, if the controller digit 0 appears at physical
        // digit 2, we need to display the segment pattern given by logical
        // position 2 when sending the byte to controller digit 0.
        const uint8_t chipPos = mFlushStage;
        const uint8_t physicalPos = remapLogicalToPhysical(chipPos);
        if (isDirtyBit(physicalPos)) {
          // Update changed digit.
          mTmiInterface.startCondition();
          mTmiInterface.sendByte(kDataCmdFixedAddress);
          mTmiInterface.stopCondition();

          mTmiInterface.startCondition();
          mTmiInterface.sendByte(kAddressCmd | chipPos);
          mTmiInterface.sendByte(mPatterns[physicalPos]);
          mTmiInterface.stopCondition();
          clearDirtyBit(physicalPos);
        }
      }

      // An extra dirty bit is used for the brightness so use `T_DIGITS + 1`.
      ace_common::incrementMod(mFlushStage, (uint8_t) (T_DIGITS + 1));
    }

  private:
    void setDirtyBit(uint8_t bit) {
      mIsDirty |= (0x1 << bit);
    }

    void clearDirtyBit(uint8_t bit) {
      mIsDirty &= ~(0x1 << bit);
    }

    bool isDirtyBit(uint8_t bit) const {
      return mIsDirty & (0x1 << bit);
    }

    /** Convert a logical position into the physical position. */
    uint8_t remapLogicalToPhysical(uint8_t pos) const {
      return mRemapArray ? mRemapArray[pos] : pos;
    }

  private:
    // Give access to mIsDirty and mFlushStage
    friend class ::Tm1637ModuleTest_flushIncremental;
    friend class ::Tm1637ModuleTest_flush;

    static uint8_t const kDataCmdWriteDisplay = 0b01000000;
    static uint8_t const kDataCmdReadKeys =     0b01000010;
    static uint8_t const kDataCmdAutoAddress =  0b01000000;
    static uint8_t const kDataCmdFixedAddress = 0b01000100;
    static uint8_t const kAddressCmd =          0b11000000;
    static uint8_t const kBrightnessCmd =       0b10000000;
    static uint8_t const kBrightnessLevelOn =   0b00001000;

    // Use the bit at position 'T_DIGITS' as the dirty bit for brightness.
    // A TM1637 can have a maximum of 6 T_DIGITS, so we are safe.
    static uint8_t const kBrightnessDirtyBit = T_DIGITS;

    // The ordering of these fields is partially determined to save memory on
    // 32-bit processors.
    const T_TMII& mTmiInterface;
    const uint8_t* const mRemapArray;
    uint8_t mPatterns[T_DIGITS]; // maps to dirty bits [0, T_DIGITS-1]
    uint8_t mBrightness; // maps to dirty bit at T_DIGITS
    uint8_t mIsDirty; // bit array
    uint8_t mFlushStage; // [0, T_DIGITS], with T_DIGITS for brightness update
};

} // ace_segment

#endif
