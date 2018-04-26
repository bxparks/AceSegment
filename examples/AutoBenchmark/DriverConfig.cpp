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

#include "Flash.h"
#include "DriverConfig.h"

static const char kLabelDigitsDirect[] PROGMEM =
  //"------------+--------+------------+------+-------------+"
    "digits      | direct |            |      |";
static const char kLabelDigitsSerial[] PROGMEM =
    "digits      | serial |            |      |";
static const char kLabelDigitsSpi[] PROGMEM =
    "digits      | spi    |            |      |";
static const char kLabelSegmentsDirect[] PROGMEM =
    "segments    | direct |            |      |";
static const char kLabelSegmentsSerial[] PROGMEM =
    "segments    | serial |            |      |";
static const char kLabelSegmentsSpi[] PROGMEM =
    "segments    | spi    |            |      |";
static const char kLabelSegmentsDirectModulation[] PROGMEM =
    "segments    | direct | modulation |      |";
static const char kLabelSegmentsSerialModulation[] PROGMEM =
    "segments    | serial | modulation |      |";
static const char kLabelSegmentsSpiModulation[] PROGMEM =
    "segments    | spi    | modulation |      |";
static const char kLabelSegmentsDirectModulationFast[] PROGMEM =
    "segments    | direct | modulation | fast |";
static const char kLabelSegmentsSerialModulationFast[] PROGMEM =
    "segments    | serial | modulation | fast |";
static const char kLabelSegmentsSpiModulationFast[] PROGMEM =
    "segments    | spi    | modulation | fast |";

// The strings below would consume 47 x 12 = 564 bytes of static memory if they
// were not placed in PROGMEM.
const DriverConfig DriverConfig::kDriverConfigs[] {
  DriverConfig(
      ResistorsOnDigits, DirectPins, NoModulation, NoFastDriver,
      FPSTR(kLabelDigitsDirect)),
  DriverConfig(
      ResistorsOnDigits, SerialPins, NoModulation, NoFastDriver,
      FPSTR(kLabelDigitsSerial)),
  DriverConfig(
      ResistorsOnDigits, SpiPins, NoModulation, NoFastDriver,
      FPSTR(kLabelDigitsSpi)),

  DriverConfig(
      ResistorsOnSegments, DirectPins, NoModulation, NoFastDriver,
      FPSTR(kLabelSegmentsDirect)),
  DriverConfig(
      ResistorsOnSegments, SerialPins, NoModulation, NoFastDriver,
      FPSTR(kLabelSegmentsSerial)),
  DriverConfig(
      ResistorsOnSegments, SpiPins, NoModulation, NoFastDriver,
      FPSTR(kLabelSegmentsSpi)),

  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, NoFastDriver,
      FPSTR(kLabelSegmentsDirectModulation)),
  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, NoFastDriver,
      FPSTR(kLabelSegmentsSerialModulation)),
  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, NoFastDriver,
      FPSTR(kLabelSegmentsSpiModulation)),

#ifdef __AVR__
  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, UseFastDriver,
      FPSTR(kLabelSegmentsDirectModulationFast)),
  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, UseFastDriver,
      FPSTR(kLabelSegmentsSerialModulationFast)),
  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, UseFastDriver,
      FPSTR(kLabelSegmentsSpiModulationFast)),
#endif
};

const uint8_t DriverConfig::kNumDriverConfigs =
    sizeof(kDriverConfigs) / sizeof(kDriverConfigs[0]);
