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

#include <string.h> // memset()
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

/**
 * An implementation of LedModule using the TM1637 chip. The chip communicates
 * using a protocol that is electrically similar to I2C, but does not use an
 * address byte at the beginning of the protocol.
 *
 * @tparam T_TMII class that implements the two wire protocol interface for
 *    TM1637, usually one of the classes from the AceTMI library:
 *    SimpleTmi1637Interface or SimpleTmi1637FastInterface.
 * @tparam T_DIGITS number of digits in the LED module (usually 4 or 6)
 */
template <typename T_TMII, uint8_t T_DIGITS>
class Tm1637Module : public LedModule {
  public:

    /**
     * Constructor.
     * @param tmiInterface instance of TM1637 interface class
     * @param remapArray (optional, nullable) a mapping of the logical digit
     *    positions to their physical positions, useful for 6-digt LED modules
     *    whose digits are wired out of order
     */
    explicit Tm1637Module(
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
     * Initialize the module. The SimpleTmi1637Interface or
     * SimpleTmi1637FastInterface object must be initialized separately.
     */
    void begin() {
      LedModule::begin();

      memset(mPatterns, 0, T_DIGITS);
      setDisplayOn(true);
      mFlushStage = 0;
    }

    /** Signal end of usage. Currently does nothing. */
    void end() {
      LedModule::end();
    }

    //-----------------------------------------------------------------------
    // Additional brightness control supported by the TM1637 chip.
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
     * Takes about 22 ms using a 100 microsecond delay.
     *
     * The isFlushRequired() method can be used to optimize the number of calls
     * to flush(), but often it is not necessary.
     */
    void flush() {
      // Command1: Update the digits using auto incrementing mode.
      mTmiInterface.startCondition();
      mTmiInterface.write(kDataCmdAutoAddress);
      mTmiInterface.stopCondition();

      // Command2: Send the LED patterns.
      mTmiInterface.startCondition();
      mTmiInterface.write(kAddressCmd);
      for (uint8_t chipPos = 0; chipPos < T_DIGITS; ++chipPos) {
        // Remap the logical position used by the controller to the actual
        // position. For example, if the controller digit 0 appears at physical
        // digit 2, we need to display the segment pattern given by logical
        // position 2 when sending the byte to controller digit 0.
        uint8_t physicalPos = remapLogicalToPhysical(chipPos);
        uint8_t effectivePattern = mPatterns[physicalPos];
        mTmiInterface.write(effectivePattern);
      }
      mTmiInterface.stopCondition();

      // Command3: Update the brightness last. This matches the recommendation
      // given in the Titan Micro TM1637 datasheet. But experimentation shows
      // that things seems to work even if brightness is sent first, before the
      // digit patterns.
      mTmiInterface.startCondition();
      mTmiInterface.write(kBrightnessCmd
          | (mDisplayOn ? kBrightnessLevelOn : 0x0)
          | (getBrightness() & 0xF));
      mTmiInterface.stopCondition();

      clearDigitsDirty();
      clearBrightnessDirty();
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
        if (isBrightnessDirty()) {
          mTmiInterface.startCondition();
          mTmiInterface.write(kBrightnessCmd
              | (mDisplayOn ? kBrightnessLevelOn : 0x0)
              | (getBrightness() & 0xF));
          mTmiInterface.stopCondition();
          clearBrightnessDirty();
        }
      } else {
        // Remap the logical position used by the controller to the actual
        // position. For example, if the controller digit 0 appears at physical
        // digit 2, we need to display the segment pattern given by logical
        // position 2 when sending the byte to controller digit 0.
        const uint8_t chipPos = mFlushStage;
        const uint8_t physicalPos = remapLogicalToPhysical(chipPos);
        if (isDigitDirty(physicalPos)) {
          // Update changed digit.
          mTmiInterface.startCondition();
          mTmiInterface.write(kDataCmdFixedAddress);
          mTmiInterface.stopCondition();

          mTmiInterface.startCondition();
          mTmiInterface.write(kAddressCmd | chipPos);
          mTmiInterface.write(mPatterns[physicalPos]);
          mTmiInterface.stopCondition();
          clearDigitDirty(physicalPos);
        }
      }

      // An extra dirty bit is used for the brightness so use `T_DIGITS + 1`.
      ace_common::incrementMod(mFlushStage, (uint8_t) (T_DIGITS + 1));
    }

    //-----------------------------------------------------------------------
    // Methods related to buttons
    //-----------------------------------------------------------------------

    /**
     * Read the 1 byte with key scan with the bits coming out in LSBFIRST order
     * with the following bits: `S0 S1 S2 K1 K2 X X X`. The S0,S1,S2 bits are
     * the binary encoding of one of the SG1 to SG8 segment lines using a
     * 0-index, so SG1 is 0 and SG8 is 7. The K1 and K2 bits are not encoded and
     * correspond directly to the K1 and K2 control lines.
     *
     * The Sn and Kn lines seem to be pulled up high, so when no buttons are
     * pressed, the data value from the TM1637 controller is 0xFF. When a button
     * is pressed, the corresponding Kn and Sn lines go to 0. For example, if
     * the button on SG2 and K1 is pressed, the SG2 generates a bit pattern of
     * `0b??101` and the K1 line corresponds to bit pattern `0b10???`, so when
     * these are combined, the final button data is `0b11110101` or 0xF5.
     */
    uint8_t readButtons() const {
      mTmiInterface.startCondition();
      mTmiInterface.write(kDataCmdReadKeys);
      uint8_t data = mTmiInterface.read();
      mTmiInterface.stopCondition();
      return data;
    }

  private:
    /** Convert a logical position into the physical position. */
    uint8_t remapLogicalToPhysical(uint8_t pos) const {
      return mRemapArray ? mRemapArray[pos] : pos;
    }

  private:
    // Give access to mIsDirty and mFlushStage
    friend class ::Tm1637ModuleTest_flushIncremental;
    friend class ::Tm1637ModuleTest_flush;

    // These come from the TM1637 controller chip datasheet.
    static uint8_t const kDataCmdWriteDisplay = 0b01000000;
    static uint8_t const kDataCmdReadKeys =     0b01000010;
    static uint8_t const kDataCmdAutoAddress =  0b01000000;
    static uint8_t const kDataCmdFixedAddress = 0b01000100;
    static uint8_t const kAddressCmd =          0b11000000;
    static uint8_t const kBrightnessCmd =       0b10000000;
    static uint8_t const kBrightnessLevelOn =   0b00001000;

    // The ordering of these fields is partially determined to save memory on
    // 32-bit processors.

    // TM1637 interface object. Copied by value instead of reference to avoid an
    // extra level of indirection.
    const T_TMII mTmiInterface;

    const uint8_t* const mRemapArray;
    uint8_t mPatterns[T_DIGITS];
    bool mDisplayOn;
    uint8_t mFlushStage; // [0, T_DIGITS], with T_DIGITS for brightness update
};

} // ace_segment

#endif
