# Copyright 2018 Brian T. Park
#
# MIT License
"""
Generate a version of SplitDigitDriver using digitalWriteFast()
with the segment pins connected through a 74HC595 serial-to-parallel chip,
using SPI to transfer bits to the chip. Similar to LedMatrixSpi class.
"""

import logging
from generator import Generator


class DriverGenerator(Generator):
    HEADER_FILE = """\
// This file was generated by the following script:
//   {invocation}
//
// DO NOT EDIT

#ifdef __AVR__

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include <ace_segment/SplitDigitDriver.h>
#include <ace_segment/Util.h>

#ifndef ACE_SEGMENT_{class_name}_H
#define ACE_SEGMENT_{class_name}_H

class {class_name}: public ace_segment::SplitDigitDriver {{
  public:
    // Constructor
    {class_name}(ace_segment::DimmablePattern* dimmablePatterns,
            uint8_t numDigits, uint8_t numSubFields):
        ace_segment::SplitDigitDriver(
            nullptr /* ledMatrix */, dimmablePatterns, numDigits, numSubFields)
    {{}}

    // Destructor
    ~{class_name}() override {{}}

    void configure() override;
    void finish() override;
    void displayCurrentField() override;
    void prepareToSleep() override;

  private:
    typedef void (*DigitalWriter)(void);

    static const uint8_t kLatchPin = {latch_pin};
    static const uint8_t kDataPin = {data_pin};
    static const uint8_t kClockPin = {clock_pin};

    // define pin values depending on common cathode or anode wiring
    {on_off_constants}

    static const uint8_t kDigitPins[];
    static const DigitalWriter kDigitWriters[];

    static void disableDigit(uint8_t digit) {{
      uint8_t index = digit * 2 + kDigitOff;
      DigitalWriter writer = kDigitWriters[index];
      writer();
    }}

    static void enableDigit(uint8_t digit) {{
      uint8_t index = digit * 2 + kDigitOn;
      DigitalWriter writer = kDigitWriters[index];
      writer();
    }}

    static void drawSegments(uint8_t pattern);

    // DigitalWriter functions for writing digit pins.
    {digit_writers}
}};

#endif

#endif
"""

    SOURCE_FILE = """\
// This file was generated by the following script:
//   {invocation}
//
// DO NOT EDIT

#ifdef __AVR__

#include <stdint.h>
#include <Arduino.h>
#include <SPI.h>
#include <digitalWriteFast.h>
#include "{class_name}.h"

const uint8_t {class_name}::kDigitPins[] = {{
  {digit_pins}
}};

const {class_name}::DigitalWriter {class_name}::kDigitWriters[] = {{
  {digit_writers_array}
}};

void {class_name}::configure() {{
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {{
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, OUTPUT);
    disableDigit(digit);
  }}

  pinMode(kLatchPin, OUTPUT);
  pinMode(kDataPin, OUTPUT);
  pinMode(kClockPin, OUTPUT);

  SPI.begin();
  ace_segment::SplitDigitDriver::configure();
}}

void {class_name}::finish() {{
  ace_segment::SplitDigitDriver::finish();
  SPI.end();

  for (uint8_t digit = 0; digit < mNumDigits; digit++) {{
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, INPUT);
  }}

  pinMode(kLatchPin, INPUT);
  pinMode(kDataPin, INPUT);
  pinMode(kClockPin, INPUT);
}}

void {class_name}::displayCurrentField() {{
  if (mPreparedToSleep) return;

  bool isCurrentDigitOn;
  ace_segment::DimmablePattern& dimmablePattern =
      mDimmablePatterns[mCurrentDigit];
  uint8_t brightness = dimmablePattern.brightness;
  if (mCurrentDigit != mPrevDigit) {{
    disableDigit(mPrevDigit);
    isCurrentDigitOn = false;
    mCurrentSubFieldMax = ((uint16_t) mNumSubFields * brightness) / 256;
  }} else {{
    isCurrentDigitOn = mIsPrevDigitOn;
  }}

  if (brightness < 255 && mCurrentSubField >= mCurrentSubFieldMax) {{
    if (isCurrentDigitOn) {{
      disableDigit(mCurrentDigit);
      isCurrentDigitOn = false;
    }}
  }} else {{
    if (!isCurrentDigitOn) {{
      SegmentPatternType segmentPattern = dimmablePattern.pattern;
      if (segmentPattern != mSegmentPattern) {{
        drawSegments(segmentPattern);
        mSegmentPattern = segmentPattern;
      }}
      enableDigit(mCurrentDigit);
      isCurrentDigitOn = true;
    }}
  }}

  mCurrentSubField++;
  mPrevDigit = mCurrentDigit;
  mIsPrevDigitOn = isCurrentDigitOn;
  if (mCurrentSubField >= mNumSubFields) {{
    ace_segment::Util::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }}
}}

void {class_name}::drawSegments(uint8_t pattern) {{
  digitalWriteFast(kLatchPin, LOW);
  uint8_t actualPattern = (kSegmentOn == HIGH) ? pattern : ~pattern;
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(actualPattern);
  SPI.endTransaction();
  digitalWriteFast(kLatchPin, HIGH);
}}

void {class_name}::prepareToSleep() {{
  Driver::prepareToSleep();
  disableDigit(mPrevDigit);
}}

#endif
"""

    def __init__(self, invocation, **kwargs):
        super().__init__(invocation, **kwargs)
        if len(self.segment_spi_pins) != 3:
            logging.error("Must provide (latch, data, clock) pins")
            sys.exit(1)

    def run(self):
        header = self.HEADER_FILE.format(
            invocation=self.invocation,
            class_name=self.class_name,
            latch_pin=self.segment_spi_pins[0],
            data_pin=self.segment_spi_pins[1],
            clock_pin=self.segment_spi_pins[2],
            on_off_constants=self.get_on_off_constants(),
            digit_writers=self.get_digit_writers())
        if self.output_header:
            print(header)
        if self.output_files:
            header_filename = self.class_name + ".h"
            with open(header_filename, 'w', encoding='utf-8') as header_file:
                print(header, end='', file=header_file)
            logging.info("Created %s", header_filename)

        source = self.SOURCE_FILE.format(
            invocation=self.invocation,
            class_name=self.class_name,
            digit_pins=self.get_digit_pins_array(),
            digit_writers_array=self.get_digit_writers_array())
        if self.output_source:
            print(source)
        if self.output_files:
            source_filename = self.class_name + ".cpp"
            with open(source_filename, 'w', encoding='utf-8') as source_file:
                print(source, end='', file=source_file)
            logging.info("Created %s", source_filename)
