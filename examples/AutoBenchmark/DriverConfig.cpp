/*
MIT License

Copyright (c) 2018 Brian T. Park

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

#include "DriverConfig.h"

// The c-strings below consume 47 x 12 = 564 bytes of static memory. They could
// be moved into flash memory. But the F() macro doesn't work outside of a
// function context, so it's a bit more effort to move them into PROGMEM. Since
// this is just a diagnostic sketch that still fits inside 2kB, it doesn't seem
// to be worthwhile to do this work. However, if we start running out of static
// memory, this would be the first place to save memory.
const DriverConfig DriverConfig::kDriverConfigs[] {
  //F("------------+-----------+------------+------+-------------+"));
  DriverConfig(
      ResistorsOnDigits, DirectPins, NoModulation, NoFastDriver,
      "digits      | direct    |            |      |"),
  DriverConfig(
      ResistorsOnDigits, SerialPins, NoModulation, NoFastDriver,
      "digits      | serial    |            |      |"),
  DriverConfig(
      ResistorsOnDigits, SpiPins, NoModulation, NoFastDriver,
      "digits      | spi       |            |      |"),

  DriverConfig(
      ResistorsOnSegments, DirectPins, NoModulation, NoFastDriver,
      "segments    | direct    |            |      |"),
  DriverConfig(
      ResistorsOnSegments, SerialPins, NoModulation, NoFastDriver,
      "segments    | serial    |            |      |"),
  DriverConfig(
      ResistorsOnSegments, SpiPins, NoModulation, NoFastDriver,
      "segments    | spi       |            |      |"),

  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, NoFastDriver,
      "segments    | direct    | modulation |      |"),
  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, NoFastDriver,
      "segments    | serial    | modulation |      |"),
  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, NoFastDriver,
      "segments    | spi       | modulation |      |"),

#ifdef __AVR__
  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, UseFastDriver,
      "segments    | direct    | modulation | fast |"),
  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, UseFastDriver,
      "segments    | serial    | modulation | fast |"),
  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, UseFastDriver,
      "segments    | spi       | modulation | fast |"),
#endif
};

const uint8_t DriverConfig::kNumDriverConfigs =
    sizeof(kDriverConfigs) / sizeof(kDriverConfigs[0]);
