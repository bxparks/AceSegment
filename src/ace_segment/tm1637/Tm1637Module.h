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

namespace ace_segment {

/**
 * In theory, the chip should be able to handle fairly small delays, like
 * 250 kHz or 4 microseconds. But Many TM1637 LED Modules from eBay
 * apparently uses a low value pullup resistor, coupled with high valued
 * capacitor, so the rise time of the signal on these lines are slow. A
 * value of 50 microseconds does not work on my LED Modules, but 100 does
 * work.
 */
static const uint16_t kDefaultTm1637DelayMicros = 100;

/**
 * Many (if not all) of the 6-digit LED modules on eBay and Amazon using the
 * TM1637 chip have their digit addresses incorrectly ordered. Not sure why the
 * did that since their 4-digit LED modules follow the natural order. This array
 * remaps the digit position to the correct order expected by this library where
 * digit 0 is on the left, and digit 5 is on the far right.
 *
 * You can create your own remap array to handle other LED modules with
 * different physical ordering compared to the logical ordering.
 *
 * Pass this array into the Tm1637Module constructor.
 */
static const uint8_t kSixDigitRemapArray[6] = {
  2, 1, 0, 5, 4, 3
};

/**
 * An implementation of seven-segment LedModule using the TM1637 chip.
 * The chip communicates using a protocol that is electrically similar to I2C,
 * but does not use a 7-8 bit address. We can use a software-based I2C
 * interface.
 *
 * @tparam WI wire protocol interface, either SwWireInterface or
 *    FastSwWireInterface
 * @tparam DIGITS number of digits in the LED module (usually 4 or 6)
 */
template <typename WI, uint8_t DIGITS>
class Tm1637Module : public LedModule {
  public:

    /**
     * Constructor.
     * @param wireInterface instance of either SwWireInterface or
     *    FastSwWireInterface
     * @param remapArray (optional) some (most?) six-digit LED modules using the
     *      TM1637 chip need remapping of the digit addresses
     */
    explicit Tm1637Module(
        const WI& wireInterface,
        const uint8_t* remapArray = nullptr
    ) :
        LedModule(DIGITS),
        mWireInterface(wireInterface),
        mRemapArray(remapArray)
    {}

    //-----------------------------------------------------------------------
    // Initialization and termination.
    //-----------------------------------------------------------------------

    /**
     * Initialize the module. The SwWireInterface object must be initialized
     * separately.
     *
     * @param remapArray optional array of positions to handle LED modules whose
     *    digit ordering is physically different than the logical ordering
     *    (where digit 0 is on the left, and digit (DIGITS-1) is on the far
     *    right).
     */
    void begin() {
      memset(mPatterns, 0, DIGITS);
      mBrightness = kBrightnessCmd | kBrightnessLevelOn | 0x7;
      mIsDirty = 0xFF; // force initial values to LED module
      mFlushStage = 0;
    }

    /** Signal end of usage. Currently does nothing. */
    void end() {}

    //-----------------------------------------------------------------------
    // Implement the LedModule interface
    //-----------------------------------------------------------------------

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return DIGITS; }

    void setPatternAt(uint8_t pos, uint8_t pattern) override {
      uint8_t actualPos = remap(pos);
      mPatterns[actualPos] = pattern;
      setDirtyBit(actualPos);
    }

    uint8_t getPatternAt(uint8_t pos) override {
      uint8_t actualPos = remap(pos);
      return mPatterns[actualPos];
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
     * Send segment patterns of all digits, plus the brightness information to
     * the display. Takes about 22 ms using a 100 microsecond delay.
     */
    void flush() {
      // Update the brightness first
      mWireInterface.startCondition();
      mWireInterface.sendByte(mBrightness);
      mWireInterface.stopCondition();

      // Update the digits.
      mWireInterface.startCondition();
      mWireInterface.sendByte(kDataCmdAutoAddress);
      mWireInterface.stopCondition();

      mWireInterface.startCondition();
      mWireInterface.sendByte(kAddressCmd);
      for (uint8_t i = 0; i < DIGITS; ++i) {
        mWireInterface.sendByte(mPatterns[i]);
      }
      mWireInterface.stopCondition();

      mIsDirty = 0x0;
    }

    /**
     * Update only a single digit or the brightness. This method must be called
     * (DIGITS + 1) times to update the digits of entire module, including the
     * brightness which is updated using a separate step. Uses the mFlushStage
     * and the mIsDirty bit array to update only the part that needs updating.
     * This method should be used if the processor cannot be blocked for the
     * entire duration of the flush() method (e.g. on the ESP8266, which will
     * cause a WDT reset when it is blocked for more than 20-40 ms).
     *
     * Using 100 micro delay, I see the following durations:
     *
     * 1) If brightness is checked and updated on every iteration, I get
     * 'min/avg/max:4/494/13780', so a maximum of 14 ms, which is still a little
     * bit high.
     *
     * 2) If brightness is checked and updated during its own mFlushStage (==
     * DIGITS), then I see `min/avg/max:4/492/10152`, saving about 3.5ms from
     * the latency. The side effect is a slightly flicker when the display and
     * brightness changes at the same time, because this incrementally updating
     * function makes those changes in 2 steps.
     *
     * The incremental flushing must use fixed addressing mode to write specific
     * digits, which adds extra commands to the wire protocol to the LED module.
     * If this algorithm is used to send all the digits in one-shot, then this
     * method is about 50% slower (30 ms), compared to flush() (22 ms).
     */
    void flushIncremental() {
      if (isDirtyBit(mFlushStage)) {
        if (mFlushStage == DIGITS) {
          // Check for brightness change.
          mWireInterface.startCondition();
          mWireInterface.sendByte(mBrightness);
          mWireInterface.stopCondition();
        } else {
          // Check for changed digits.
          mWireInterface.startCondition();
          mWireInterface.sendByte(kDataCmdFixedAddress);
          mWireInterface.stopCondition();

          mWireInterface.startCondition();
          mWireInterface.sendByte(kAddressCmd | mFlushStage);
          mWireInterface.sendByte(mPatterns[mFlushStage]);
          mWireInterface.stopCondition();
        }
        clearDirtyBit(mFlushStage);
      }

      ace_common::incrementMod(mFlushStage, (uint8_t) (DIGITS + 1));
    }

  private:
    void setDirtyBit(uint8_t bit) {
      mIsDirty |= (0x1 << bit);
    }

    void clearDirtyBit(uint8_t bit) {
      mIsDirty &= ~(0x1 << bit);
    }

    bool isDirtyBit(uint8_t bit) {
      return mIsDirty & (0x1 << bit);
    }

    /** Convert a logical position into the physical position. */
    uint8_t remap(uint8_t pos) {
      return mRemapArray ? mRemapArray[pos] : pos;
    }

  private:
    static uint8_t const kDataCmdWriteDisplay = 0b01000000;
    static uint8_t const kDataCmdReadKeys =     0b01000010;
    static uint8_t const kDataCmdAutoAddress =  0b01000000;
    static uint8_t const kDataCmdFixedAddress = 0b01000100;
    static uint8_t const kAddressCmd =          0b11000000;
    static uint8_t const kBrightnessCmd =       0b10000000;
    static uint8_t const kBrightnessLevelOn =   0b00001000;

    // Use the bit at position 'DIGITS' as the dirty bit for brightness.
    // A TM1637 can have a maximum of 6 DIGITS, so we are safe.
    static uint8_t const kBrightnessDirtyBit = DIGITS;

    // The ordering of these fields is partially determined to save memory on
    // 32-bit processors.
    const WI& mWireInterface;
    const uint8_t* const mRemapArray;
    uint8_t mPatterns[DIGITS]; // maps to dirty bits 0-5
    uint8_t mBrightness; // maps to dirty bit 7
    uint8_t mIsDirty; // bit array
    uint8_t mFlushStage; // [0, DIGITS], DIGITS for brightness update
};


} // ace_segment

#endif
