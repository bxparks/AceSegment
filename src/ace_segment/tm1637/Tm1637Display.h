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

#ifndef ACE_SEGMENT_TM1637_DISPLAY_H
#define ACE_SEGMENT_TM1637_DISPLAY_H

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include "../LedDisplay.h"

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
 * An implementation of LedDisplay that works with 7-segment LED modules
 * using the TM1637 chip.
 *
 * @tparam DRIVER driver class, either Tm1637Driver or Tm1637DriverFast
 * @tparam DIGITS number of digits in the LED module (usually 4 or 6)
 */
template <typename DRIVER, uint8_t DIGITS>
class Tm1637Display : public LedDisplay {
  public:
    explicit Tm1637Display(const DRIVER& driver) :
        LedDisplay(DIGITS),
        mDriver(driver)
    {}

    /** Return the number of digits supported by this display instance. */
    uint8_t getNumDigits() const { return DIGITS; }

    void begin() {
      mDriver.begin();
      memset(mPatterns, 0, DIGITS);
      mBrightness = kBrightnessCmd | kBrightnessLevelOn | 0x7;
      mIsDirty = 0xFF;
      mFlushStage = 0;
    }

    void writePatternAt(uint8_t pos, uint8_t pattern) override {
      if (pos >= DIGITS) return;
      mPatterns[pos] = pattern;
      setDirtyBit(pos);
    }

    void writeDecimalPointAt(uint8_t pos, bool state = true) override {
      if (pos >= DIGITS) return;
      uint8_t pattern = mPatterns[pos];
      if (state) {
        pattern |= 0x80;
      } else {
        pattern &= ~0x80;
      }
      mPatterns[pos] = pattern;

      setDirtyBit(pos);
    }

    void setBrightness(uint8_t brightness) override {
      mBrightness = (mBrightness & ~0x7) | (brightness & 0x7);
      setDirtyBit(kBrightnessDirtyBit);
    }

    void setDisplayOn(bool on = true) {
      if (on) {
        mBrightness |= kBrightnessLevelOn;
      } else {
        mBrightness &= ~kBrightnessLevelOn;
      }
      setDirtyBit(kBrightnessDirtyBit);
    }

    /**
     * Send segment patterns of all digits, plus the brightness information to
     * the display. Takes about 22 ms using a 100 microsecond delay.
     */
    void flush() {
      // Update the brightness first
      mDriver.startCondition();
      mDriver.sendByte(mBrightness);
      mDriver.stopCondition();

      // Update the digits.
      mDriver.startCondition();
      mDriver.sendByte(kDataCmdAutoAddress);
      mDriver.stopCondition();

      mDriver.startCondition();
      mDriver.sendByte(kAddressCmd);
      for (uint8_t i = 0; i < DIGITS; ++i) {
        mDriver.sendByte(mPatterns[i]);
      }
      mDriver.stopCondition();

      mIsDirty = 0x0;
    }

    /**
     * Use the mFlushStage and the mIsDirty bit array to update only the part
     * that needs updating. If the entire display needs to be updated, then it
     * is about 50% slower 30 ms, versus 22 ms for flush(). Using 100 micro
     * delay, I see the following durations:
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
     */
    void flushIncremental() {
      if (isDirtyBit(mFlushStage)) {
        if (mFlushStage == DIGITS) {
          // Check for brightness change.
          mDriver.startCondition();
          mDriver.sendByte(mBrightness);
          mDriver.stopCondition();
        } else {
          // Check for changed digits.
          mDriver.startCondition();
          mDriver.sendByte(kDataCmdFixedAddress);
          mDriver.stopCondition();

          mDriver.startCondition();
          mDriver.sendByte(kAddressCmd | mFlushStage);
          mDriver.sendByte(mPatterns[mFlushStage]);
          mDriver.stopCondition();
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

    const DRIVER& mDriver;
    uint8_t mIsDirty; // bit array
    uint8_t mBrightness; // maps to dirty bit 7
    uint8_t mFlushStage; // [0, DIGITS], DIGITS for brightness update
    uint8_t mPatterns[DIGITS]; // maps to dirty bits 0-5
};


} // ace_segment

#endif
