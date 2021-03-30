# Copyright 2018 Brian T. Park
#
# MIT License
"""
Generate a version of SplitDigitDriver using digitalWriteFast()
assuming a directing wiring of LED display to the microcontroller pins.
"""

# TODO: I think the drawSegments() method can be dramatically sped up by
# unrolling the for-loop, then using the pin numbers that's known at compile
# time and provided to this script, we can avoid the call to writeSegment(),
# avoiding the array look up in kSegmentWriters[]. We can write out the 16
# variations of the digitalWriteFast() methods directly, in a series of eight
# if-statements that look like this:
#
#   if (pattern & elementMask)
#     digitalWriteFast(x, kSegmentOn)
#   else
#     digitalWriteFast(x, kSegmentOff);


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
#include <digitalWriteFast.h>
#include <AceCommon.h> // incrementMod()
#include <ace_segment/SplitDigitDriver.h>

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

    static const uint8_t kNumSegments = {num_segments};

    // define pin values depending on common cathode or anode wiring
    {on_off_constants}

    static const uint8_t kSegmentPins[];
    static const uint8_t kDigitPins[];
    static const DigitalWriter kSegmentWriters[];
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

    static void writeSegment(uint8_t segment, uint8_t value) {{
      uint8_t index = segment * 2 + value;
      DigitalWriter writer = kSegmentWriters[index];
      writer();
    }}

    static void drawSegments(uint8_t pattern);

    // DigitalWriter functions for writing segment pins.
    {segment_writers}

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
#include <digitalWriteFast.h>
#include "{class_name}.h"

const uint8_t {class_name}::kSegmentPins[] = {{
  {segment_pins}
}};

const uint8_t {class_name}::kDigitPins[] = {{
  {digit_pins}
}};

const {class_name}::DigitalWriter {class_name}::kSegmentWriters[] = {{
  {segment_writers}
}};

const {class_name}::DigitalWriter {class_name}::kDigitWriters[] = {{
  {digit_writers}
}};

void {class_name}::configure() {{
  for (uint8_t digit = 0; digit < mNumDigits; digit++) {{
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, OUTPUT);
    disableDigit(digit);
  }}

  for (uint8_t segment = 0; segment < kNumSegments; segment++) {{
    uint8_t elementPin = kSegmentPins[segment];
    pinMode(elementPin, OUTPUT);
    writeSegment(segment, kSegmentOff);
  }}

  ace_segment::SplitDigitDriver::configure();
}}

void {class_name}::finish() {{
  ace_segment::SplitDigitDriver::finish();

  for (uint8_t digit = 0; digit < mNumDigits; digit++) {{
    uint8_t groupPin = kDigitPins[digit];
    pinMode(groupPin, INPUT);
  }}

  for (uint8_t segment = 0; segment < kNumSegments; segment++) {{
    uint8_t elementPin = kSegmentPins[segment];
    pinMode(elementPin, INPUT);
  }}
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
    ace_common::incrementMod(mCurrentDigit, mNumDigits);
    mCurrentSubField = 0;
  }}
}}

void {class_name}::drawSegments(uint8_t pattern) {{
  uint8_t elementMask = 0x1;
  for (uint8_t segment = 0; segment < kNumSegments; segment++) {{
    uint8_t output = (pattern & elementMask) ? kSegmentOn : kSegmentOff;
    writeSegment(segment, output);
    elementMask <<= 1;
  }}
}}

void {class_name}::prepareToSleep() {{
  Driver::prepareToSleep();
  disableDigit(mPrevDigit);
}}

#endif
"""

    def __init__(self, invocation, **kwargs):
        super().__init__(invocation, **kwargs)
        if len(self.segment_direct_pins) == 0:
            logging.error("Must provide segment_direct_pins")
            sys.exit(1)

    def run(self):
        header = self.HEADER_FILE.format(
            invocation=self.invocation,
            class_name=self.class_name,
            num_segments=len(self.segment_direct_pins),
            on_off_constants=self.get_on_off_constants(),
            segment_writers=self.get_segment_writers(),
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
            segment_pins=self.get_segment_pins_array(),
            digit_pins=self.get_digit_pins_array(),
            segment_writers=self.get_segment_writers_array(),
            digit_writers=self.get_digit_writers_array())
        if self.output_source:
            print(source)
        if self.output_files:
            source_filename = self.class_name + ".cpp"
            with open(source_filename, 'w', encoding='utf-8') as source_file:
                print(source, end='', file=source_file)
            logging.info("Created %s", source_filename)
