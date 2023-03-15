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

#ifndef ACE_SEGMENT_LED_MODULE_H
#define ACE_SEGMENT_LED_MODULE_H

#include <stdint.h>

namespace ace_segment {

/**
 * General interface that represents a generic seven-segment LED module with
 * multiple digits. Subclasses will support different driver chips (e.g. TM1637,
 * MAX7219, or even a 74HC595).
 *
 * The digit addressing scheme is normalized at this layer so that digit 0 is
 * the left most digit, and digit 'numDigits-1' is the right most digit. The
 * segment addressing is also normalized so that bit 0 is the 'A' segment, bit 6
 * is 'G' segment, and bit 7 is the decimal point. If an LED module does not
 * conform to this convention, the implementation class for that hardware must
 * remap the digit and segment addresses.
 */
class LedModule {
  public:
    /**
     * Constructor.
     *
     * @param patterns pointer to an array of bytes representing LED segment
     *    patterns
     * @param numDigits number of digits in the LED module; this value is
     *    returned by size(). The value is usually a compile-time
     *    constant passed in through a template parameter, so it is faster and
     *    cheaper to use the template parameter. However, sometimes the calling
     *    code needs this value but it has only a reference or pointer to the
     *    LedModule. Then size() can be used.
     */
    explicit LedModule(uint8_t* patterns, uint8_t numDigits) :
        mPatterns(patterns),
        mNumDigits(numDigits)
    {}

    /**
     * Return the number of digits supported by this display instance.
     * Deprecated, but retained for backwards compatibility.
     */
    uint8_t getNumDigits() const { return mNumDigits; }

    /** Return the number of digits supported by this display instance. */
    uint8_t size() const { return mNumDigits; }

    /** Set the led digit pattern at position pos. */
    void setPatternAt(uint8_t pos, uint8_t pattern) {
      mPatterns[pos] = pattern;
      setDigitDirty(pos);
    }

    /** Get the led digit pattern at position pos. */
    uint8_t getPatternAt(uint8_t pos) const {
      return mPatterns[pos];
    }

    /**
     * Set global brightness of all digits. Different subclasses will interpret
     * the brightness integer value differently.
     */
    void setBrightness(uint8_t brightness) {
      mBrightness = brightness;
      mIsBrightnessDirty = true;
    }

    /** Get the current brightness. */
    uint8_t getBrightness() const {
      return mBrightness;
    }

    /** Set decimal point. */
    void setDecimalPointAt(uint8_t pos, bool state = true) {
      if (pos >= mNumDigits) return;
      if (state) {
        mPatterns[pos] |= 0x80;
      } else {
        mPatterns[pos] &= ~0x80;
      }
    }

  protected:
    /** Subclasses should call this from its own begin(). */
    void begin() {
      // Dirty bits are set to true so that the first refresh sends the current
      // pattern to the LED module. Otherwise, nothing will be displayed until
      // a setPatternAt() or setBrightness() is called.
      mDigitDirtyBits = 0xFF;
      mIsBrightnessDirty = true;

      // On some LEDs, level 0 turns off the display, but on others level 0 is
      // the lowest brightness level. Let's set the initial brightness to 1.
      mBrightness = 1;
    }

    /**
     * Subclasses should call this from its own end(). Currently does nothing.
     */
    void end() {}

    /** Set the dirty bit of digit `pos`. */
    void setDigitDirty(uint8_t pos) {
      mDigitDirtyBits |= (1 << pos);
    }

    /** Clear the dirty bit of digit `pos`. */
    void clearDigitDirty(uint8_t pos) {
      mDigitDirtyBits &= ~(1 << pos);
    }

    /** Check the dirty bit of digit `pos`. */
    bool isDigitDirty(uint8_t pos) const {
      return mDigitDirtyBits & (1 << pos);
    }

    /** Clear dirty bits of all digits. */
    void clearDigitsDirty() {
      mDigitDirtyBits = 0x0;
    }

    /** Return true if any digits are dirty. */
    bool isAnyDigitDirty() const {
      return mDigitDirtyBits != 0;
    }

    /** Check if the brightness level is dirty. */
    bool isBrightnessDirty() const {
      return mIsBrightnessDirty;
    }

    /** Mark the brightness as dirty. */
    void setBrightnessDirty() {
      mIsBrightnessDirty = true;
    }

    /** Clear the dirty bit for brightness. */
    void clearBrightnessDirty() {
      mIsBrightnessDirty = false;
    }

  private:
    // disable copy-constructor and assignment operator
    LedModule(const LedModule&) = delete;
    LedModule& operator=(const LedModule&) = delete;

  private:
    // The order of these instance variables is partially motivated to save
    // memory on 32-bit processors.
    uint8_t* const mPatterns;
    uint8_t const mNumDigits;

    uint8_t mDigitDirtyBits; // array of 8 dirty bits
    uint8_t mBrightness;
    bool mIsBrightnessDirty;
};

} // ace_segment

#endif
