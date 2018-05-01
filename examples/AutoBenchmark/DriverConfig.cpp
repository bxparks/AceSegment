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

  //"------------+--------+------------+------+--------+-------------+";
static const char kLabelDigitsDirect[] PROGMEM =
    "digits      | direct |            |      |        |";
static const char kLabelDigitsDirectStyles[] PROGMEM =
    "digits      | direct |            |      | styles |";

static const char kLabelDigitsSerial[] PROGMEM =
    "digits      | serial |            |      |        |";
static const char kLabelDigitsSerialStyles[] PROGMEM =
    "digits      | serial |            |      | styles |";

static const char kLabelDigitsSpi[] PROGMEM =
    "digits      | spi    |            |      |        |";
static const char kLabelDigitsSpiStyles[] PROGMEM =
    "digits      | spi    |            |      | styles |";

static const char kLabelSegmentsDirect[] PROGMEM =
    "segments    | direct |            |      |        |";
static const char kLabelSegmentsDirectStyles[] PROGMEM =
    "segments    | direct |            |      | styles |";

static const char kLabelSegmentsSerial[] PROGMEM =
    "segments    | serial |            |      |        |";
static const char kLabelSegmentsSerialStyles[] PROGMEM =
    "segments    | serial |            |      | styles |";

static const char kLabelSegmentsSpi[] PROGMEM =
    "segments    | spi    |            |      |        |";
static const char kLabelSegmentsSpiStyles[] PROGMEM =
    "segments    | spi    |            |      | styles |";

static const char kLabelSegmentsDirectModulation[] PROGMEM =
    "segments    | direct | modulation |      |        |";
static const char kLabelSegmentsDirectModulationStyles[] PROGMEM =
    "segments    | direct | modulation |      | styles |";

static const char kLabelSegmentsSerialModulation[] PROGMEM =
    "segments    | serial | modulation |      |        |";
static const char kLabelSegmentsSerialModulationStyles[] PROGMEM =
    "segments    | serial | modulation |      | styles |";

static const char kLabelSegmentsSpiModulation[] PROGMEM =
    "segments    | spi    | modulation |      |        |";
static const char kLabelSegmentsSpiModulationStyles[] PROGMEM =
    "segments    | spi    | modulation |      | styles |";

static const char kLabelSegmentsDirectModulationFast[] PROGMEM =
    "segments    | direct | modulation | fast |        |";
static const char kLabelSegmentsDirectModulationFastStyles[] PROGMEM =
    "segments    | direct | modulation | fast | styles |";

static const char kLabelSegmentsSerialModulationFast[] PROGMEM =
    "segments    | serial | modulation | fast |        |";
static const char kLabelSegmentsSerialModulationFastStyles[] PROGMEM =
    "segments    | serial | modulation | fast | styles |";

static const char kLabelSegmentsSpiModulationFast[] PROGMEM =
    "segments    | spi    | modulation | fast |        |";
static const char kLabelSegmentsSpiModulationFastStyles[] PROGMEM =
    "segments    | spi    | modulation | fast | styles |";


// The strings below would consume 47 x 12 = 564 bytes of static memory if they
// were not placed in PROGMEM.
const DriverConfig DriverConfig::kDriverConfigs[] {

  // Renderers using no Styles, which should make these fast.

  DriverConfig(
      ResistorsOnDigits, DirectPins, NoModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelDigitsDirect)),
  DriverConfig(
      ResistorsOnDigits, DirectPins, NoModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelDigitsDirectStyles)),

  DriverConfig(
      ResistorsOnDigits, SerialPins, NoModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelDigitsSerial)),
  DriverConfig(
      ResistorsOnDigits, SerialPins, NoModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelDigitsSerialStyles)),

  DriverConfig(
      ResistorsOnDigits, SpiPins, NoModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelDigitsSpi)),
  DriverConfig(
      ResistorsOnDigits, SpiPins, NoModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelDigitsSpiStyles)),

  DriverConfig(
      ResistorsOnSegments, DirectPins, NoModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelSegmentsDirect)),
  DriverConfig(
      ResistorsOnSegments, DirectPins, NoModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelSegmentsDirectStyles)),

  DriverConfig(
      ResistorsOnSegments, SerialPins, NoModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelSegmentsSerial)),
  DriverConfig(
      ResistorsOnSegments, SerialPins, NoModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelSegmentsSerialStyles)),

  DriverConfig(
      ResistorsOnSegments, SpiPins, NoModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelSegmentsSpi)),
  DriverConfig(
      ResistorsOnSegments, SpiPins, NoModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelSegmentsSpiStyles)),

  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelSegmentsDirectModulation)),
  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelSegmentsDirectModulationStyles)),

  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelSegmentsSerialModulation)),
  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelSegmentsSerialModulationStyles)),

  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, NoFastDriver, NoStyles,
      FPSTR(kLabelSegmentsSpiModulation)),
  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, NoFastDriver, UseStyles,
      FPSTR(kLabelSegmentsSpiModulationStyles)),

#ifdef __AVR__
  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, UseFastDriver, NoStyles,
      FPSTR(kLabelSegmentsDirectModulationFast)),
  DriverConfig(
      ResistorsOnSegments, DirectPins, UseModulation, UseFastDriver, UseStyles,
      FPSTR(kLabelSegmentsDirectModulationFastStyles)),

  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, UseFastDriver, NoStyles,
      FPSTR(kLabelSegmentsSerialModulationFast)),
  DriverConfig(
      ResistorsOnSegments, SerialPins, UseModulation, UseFastDriver, UseStyles,
      FPSTR(kLabelSegmentsSerialModulationFastStyles)),

  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, UseFastDriver, NoStyles,
      FPSTR(kLabelSegmentsSpiModulationFast)),
  DriverConfig(
      ResistorsOnSegments, SpiPins, UseModulation, UseFastDriver, UseStyles,
      FPSTR(kLabelSegmentsSpiModulationFastStyles)),
#endif
};

const uint8_t DriverConfig::kNumDriverConfigs =
    sizeof(kDriverConfigs) / sizeof(kDriverConfigs[0]);
