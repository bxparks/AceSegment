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

#ifndef ACE_SEGMENT_LED_MATRIX_DIRECT_FAST_H
#define ACE_SEGMENT_LED_MATRIX_DIRECT_FAST_H

// This header file requires the digitalWriteFast library on AVR, or the
// EpoxyMockDigitalWriteFast library on EpoxyDuino.
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)

#include <stdint.h>
#include <Arduino.h> // OUTPUT, INPUT
#include "LedMatrixBase.h"

// Select OPTION_ARRAY to use an array of function pointers. Using an array
// of function takes 27 microseconds for displayCurentField(), but 8 extra bytes
// of flash, and 48 extra bytes of static RAM, compared to the SWITCH option.
//
// Select OPTION_SWITCH to use a switch statement. This is 30% slower for
// displayCurentField(), 35 microseconds, but has smaller memory footprint).
#define ACE_SEGMENT_LMDF_OPTION_ARRAY 0
#define ACE_SEGMENT_LMDF_OPTION_SWITCH 1
#if ! defined(ACE_SEGMENT_LMDF_OPTION)
#define ACE_SEGMENT_LMDF_OPTION ACE_SEGMENT_LMDF_OPTION_ARRAY
#endif

namespace ace_segment {

#if ACE_SEGMENT_LMDF_OPTION == ACE_SEGMENT_LMDF_OPTION_ARRAY
typedef void (*DigitalWriter)(void);
#endif

/**
 * An LedMatrix very similar to LedMatrixDirect, with the element (segment) pins
 * and group (digit) pins directly connected to the microcontroller. But this
 * version uses the `pinModeFast()` and `digitalWriteFast()` functions from
 * https://github.com/NicksonYap/digitalWriteFast.
 *
 * The pin numbers must be given as compile-time constants, so they are passed
 * in as template parameters. This forces the number of groups (digits) to be
 * fixed at compile-time. This particular class supports exactly 4 digits. The
 * number of elements (segments) is always 8.
 *
 * @tparam eX element (segment) pin numbers
 * @tparam gX group (digit) pin numbers
 */
template <
  uint8_t e0, uint8_t e1, uint8_t e2, uint8_t e3,
  uint8_t e4, uint8_t e5, uint8_t e6, uint8_t e7,
  uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3
>
class LedMatrixDirectFast4 : public LedMatrixBase {
  public:
    static const uint8_t kNumElements = 8;
    static const uint8_t kNumGroups = 4;

    LedMatrixDirectFast4(
        uint8_t elementOnPattern,
        uint8_t groupOnPattern
    ) :
        LedMatrixBase(elementOnPattern, groupOnPattern)
    {}

    void begin() const {
      // Set LEDs to off.
      clear();

      // Set pins to OUTPUT mode.
      pinModeFast(e0, OUTPUT);
      pinModeFast(e1, OUTPUT);
      pinModeFast(e2, OUTPUT);
      pinModeFast(e3, OUTPUT);
      pinModeFast(e4, OUTPUT);
      pinModeFast(e5, OUTPUT);
      pinModeFast(e6, OUTPUT);
      pinModeFast(e7, OUTPUT);

      // Set pins to OUTPUT mode.
      pinModeFast(g0, OUTPUT);
      pinModeFast(g1, OUTPUT);
      pinModeFast(g2, OUTPUT);
      pinModeFast(g3, OUTPUT);
    }

    void end() const {
      // Set pins to INPUT mode.
      pinModeFast(g0, INPUT);
      pinModeFast(g1, INPUT);
      pinModeFast(g2, INPUT);
      pinModeFast(g3, INPUT);

      // Set pins to INPUT mode.
      pinModeFast(e0, INPUT);
      pinModeFast(e1, INPUT);
      pinModeFast(e2, INPUT);
      pinModeFast(e3, INPUT);
      pinModeFast(e4, INPUT);
      pinModeFast(e5, INPUT);
      pinModeFast(e6, INPUT);
      pinModeFast(e7, INPUT);
    }

    void draw(uint8_t group, uint8_t elementPattern) const {
      if (group != mPrevGroup) {
        disableGroup(mPrevGroup);
      }

      drawElements(elementPattern);
      enableGroup(group);
      mPrevGroup = group;
    }

    void enableGroup(uint8_t group) const {
      writeGroupPin(group, 0x1);
      mPrevGroup = group;
    }

    void disableGroup(uint8_t group) const {
      writeGroupPin(group, 0x0);
      mPrevGroup = group;
    }

    void clear() const {
      for (uint8_t group = 0; group < kNumGroups; group++) {
        writeGroupPin(group, 0x0);
      }
      drawElements(0x00);
    }

  private:
    /** Send the pattern to the element pins. */
    void drawElements(uint8_t pattern) const {
      for (uint8_t element = 0; element < kNumElements; element++) {
        writeElementPin(element, pattern);
        pattern >>= 1;
      }
    }

    /** Write bit 0 of output to the element pin. */
    void writeElementPin(uint8_t element, uint8_t output) const {
      uint8_t actualOutput = (output ^ mElementXorMask) & 0x1;
      uint8_t index = element * 2 + actualOutput;

    #if ACE_SEGMENT_LMDF_OPTION == ACE_SEGMENT_LMDF_OPTION_ARRAY
      DigitalWriter writer = kElementWriters[index];
      writer();

    #else
      switch (index) {
        case  0: digitalWriteFast(e0, LOW); break;
        case  1: digitalWriteFast(e0, HIGH); break;
        case  2: digitalWriteFast(e1, LOW); break;
        case  3: digitalWriteFast(e1, HIGH); break;
        case  4: digitalWriteFast(e2, LOW); break;
        case  5: digitalWriteFast(e2, HIGH); break;
        case  6: digitalWriteFast(e3, LOW); break;
        case  7: digitalWriteFast(e3, HIGH); break;
        case  8: digitalWriteFast(e4, LOW); break;
        case  9: digitalWriteFast(e4, HIGH); break;
        case 10: digitalWriteFast(e5, LOW); break;
        case 11: digitalWriteFast(e5, HIGH); break;
        case 12: digitalWriteFast(e6, LOW); break;
        case 13: digitalWriteFast(e6, HIGH); break;
        case 14: digitalWriteFast(e7, LOW); break;
        case 15: digitalWriteFast(e7, HIGH); break;
        default: break;
      }
    #endif
    }

    /** Write bit 0 of output to group pin. */
    void writeGroupPin(uint8_t group, uint8_t output) const {
      uint8_t actualOutput = (output ^ mGroupXorMask) & 0x1;
      uint8_t index = group * 2 + actualOutput;

    #if ACE_SEGMENT_LMDF_OPTION == ACE_SEGMENT_LMDF_OPTION_ARRAY
      DigitalWriter writer = kGroupWriters[index];
      writer();

    #else
      switch (index) {
        case 0: digitalWriteFast(g0, LOW); break;
        case 1: digitalWriteFast(g0, HIGH); break;
        case 2: digitalWriteFast(g1, LOW); break;
        case 3: digitalWriteFast(g1, HIGH); break;
        case 4: digitalWriteFast(g2, LOW); break;
        case 5: digitalWriteFast(g2, HIGH); break;
        case 6: digitalWriteFast(g3, LOW); break;
        case 7: digitalWriteFast(g3, HIGH); break;
        default: break;
      }
    #endif
    }

  #if ACE_SEGMENT_LMDF_OPTION == ACE_SEGMENT_LMDF_OPTION_ARRAY
    // DigitalWriter functions for writing element pins.
    static void digitalWriteFastElement0L() { digitalWriteFast(e0, LOW); }
    static void digitalWriteFastElement0H() { digitalWriteFast(e0, HIGH); }
    static void digitalWriteFastElement1L() { digitalWriteFast(e1, LOW); }
    static void digitalWriteFastElement1H() { digitalWriteFast(e1, HIGH); }
    static void digitalWriteFastElement2L() { digitalWriteFast(e2, LOW); }
    static void digitalWriteFastElement2H() { digitalWriteFast(e2, HIGH); }
    static void digitalWriteFastElement3L() { digitalWriteFast(e3, LOW); }
    static void digitalWriteFastElement3H() { digitalWriteFast(e3, HIGH); }
    static void digitalWriteFastElement4L() { digitalWriteFast(e4, LOW); }
    static void digitalWriteFastElement4H() { digitalWriteFast(e4, HIGH); }
    static void digitalWriteFastElement5L() { digitalWriteFast(e5, LOW); }
    static void digitalWriteFastElement5H() { digitalWriteFast(e5, HIGH); }
    static void digitalWriteFastElement6L() { digitalWriteFast(e6, LOW); }
    static void digitalWriteFastElement6H() { digitalWriteFast(e6, HIGH); }
    static void digitalWriteFastElement7L() { digitalWriteFast(e7, LOW); }
    static void digitalWriteFastElement7H() { digitalWriteFast(e7, HIGH); }

    // DigitalWriter functions for writing group pins.
    static void digitalWriteFastGroup0L() { digitalWriteFast(g0, LOW); }
    static void digitalWriteFastGroup0H() { digitalWriteFast(g0, HIGH); }
    static void digitalWriteFastGroup1L() { digitalWriteFast(g1, LOW); }
    static void digitalWriteFastGroup1H() { digitalWriteFast(g1, HIGH); }
    static void digitalWriteFastGroup2L() { digitalWriteFast(g2, LOW); }
    static void digitalWriteFastGroup2H() { digitalWriteFast(g2, HIGH); }
    static void digitalWriteFastGroup3L() { digitalWriteFast(g3, LOW); }
    static void digitalWriteFastGroup3H() { digitalWriteFast(g3, HIGH); }
  #endif

  private:
  #if ACE_SEGMENT_LMDF_OPTION == ACE_SEGMENT_LMDF_OPTION_ARRAY
    static const DigitalWriter kGroupWriters[2 * kNumGroups];
    static const DigitalWriter kElementWriters[2 * kNumElements];
  #endif

    /** Store the previous group, to turn it off after moving to new group. */
    mutable uint8_t mPrevGroup = 0;
};

#if ACE_SEGMENT_LMDF_OPTION == ACE_SEGMENT_LMDF_OPTION_ARRAY

template <
  uint8_t e0, uint8_t e1, uint8_t e2, uint8_t e3,
  uint8_t e4, uint8_t e5, uint8_t e6, uint8_t e7,
  uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3
>
const DigitalWriter
LedMatrixDirectFast4<e0, e1, e2, e3, e4, e5, e6, e7, g0, g1, g2, g3>
::kElementWriters[2 * kNumElements] = {
  digitalWriteFastElement0L,
  digitalWriteFastElement0H,
  digitalWriteFastElement1L,
  digitalWriteFastElement1H,
  digitalWriteFastElement2L,
  digitalWriteFastElement2H,
  digitalWriteFastElement3L,
  digitalWriteFastElement3H,
  digitalWriteFastElement4L,
  digitalWriteFastElement4H,
  digitalWriteFastElement5L,
  digitalWriteFastElement5H,
  digitalWriteFastElement6L,
  digitalWriteFastElement6H,
  digitalWriteFastElement7L,
  digitalWriteFastElement7H,
};

template <
  uint8_t e0, uint8_t e1, uint8_t e2, uint8_t e3,
  uint8_t e4, uint8_t e5, uint8_t e6, uint8_t e7,
  uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3
>
const DigitalWriter
LedMatrixDirectFast4<e0, e1, e2, e3, e4, e5, e6, e7, g0, g1, g2, g3>
::kGroupWriters[2 * kNumGroups] = {
  digitalWriteFastGroup0L,
  digitalWriteFastGroup0H,
  digitalWriteFastGroup1L,
  digitalWriteFastGroup1H,
  digitalWriteFastGroup2L,
  digitalWriteFastGroup2H,
  digitalWriteFastGroup3L,
  digitalWriteFastGroup3H,
};

#endif

} // ace_segment

#endif // defined(ARDUINO_ARCH_AVR)

#endif

