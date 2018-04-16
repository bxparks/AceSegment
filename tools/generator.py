# Copyright 2018 Brian T. Park
#
# MIT License
"""
Base class of various generator classes.
"""

import logging


class Generator:
    def __init__(self, invocation, **kwargs):
        self.invocation = invocation
        self.class_name = kwargs['class_name']
        self.segment_direct_pins = kwargs.get('segment_direct_pins')
        self.segment_serial_pins = kwargs.get('segment_serial_pins')
        self.segment_spi_pins = kwargs.get('segment_spi_pins')
        self.digit_pins = kwargs['digit_pins']
        self.common_cathode = kwargs['common_cathode']
        self.use_transistor_drivers = kwargs['use_transistor_drivers']
        self.digital_write_fast = kwargs['digital_write_fast']
        self.output_header = kwargs['output_header']
        self.output_source = kwargs['output_source']
        self.output_files = kwargs['output_files']
        logging.info("invocation: %s", self.invocation)
        logging.info("class_name: %s", self.class_name)
        logging.info("segment_direct_pins: %s", self.segment_direct_pins)
        logging.info("segment_serial_pins: %s", self.segment_serial_pins)
        logging.info("segment_spi_pins: %s", self.segment_spi_pins)
        logging.info("digit_pins: %s", self.digit_pins)
        logging.info("common_cathode: %s", self.common_cathode)
        logging.info("use_transistor_drivers: %s", self.use_transistor_drivers)
        logging.info("digital_write_fast: %s", self.digital_write_fast)

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
            segment_direct_pins=self.get_segment_pins_array(),
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

    def get_segment_pins_array(self):
        entries = []
        for pin in self.segment_direct_pins:
            entry = ('%s,' % (pin))
            entries.append(entry)
        return '\n  '.join(entries)

    def get_digit_pins_array(self):
        entries = []
        for pin in self.digit_pins:
            entry = ('%s,' % (pin))
            entries.append(entry)
        return '\n  '.join(entries)

    def get_segment_writers(self):
        writers = []
        if self.digital_write_fast:
            method = 'digitalWriteFast'
        else:
            method = 'digitalWrite'
        for i in range(len(self.segment_direct_pins)):
            pin = self.segment_direct_pins[i]
            low = ('static void digitalWriteFastSegment%02dLow() ' +
                   '{ %s(%s, LOW); }') % (i, method, pin)
            high = ('static void digitalWriteFastSegment%02dHigh() ' +
                    '{ %s(%s, HIGH); }') % (i, method, pin)
            writers.append(low)
            writers.append(high)
        return '\n    '.join(writers)

    def get_segment_writers_array(self):
        entries = []
        for i in range(len(self.segment_direct_pins)):
            low = ('digitalWriteFastSegment%02dLow,' % (i))
            high = ('digitalWriteFastSegment%02dHigh,' % (i))
            entries.append(low)
            entries.append(high)
        return '\n  '.join(entries)

    def get_digit_writers(self):
        writers = []
        if self.digital_write_fast:
            method = 'digitalWriteFast'
        else:
            method = 'digitalWrite'
        for i in range(len(self.digit_pins)):
            pin = self.digit_pins[i]
            low = ('static void digitalWriteFastDigit%02dLow() ' +
                   '{ %s(%s, LOW); }') % (i, method, pin)
            high = ('static void digitalWriteFastDigit%02dHigh() ' +
                    '{ %s(%s, HIGH); }') % (i, method, pin)
            writers.append(low)
            writers.append(high)
        return '\n    '.join(writers)

    def get_digit_writers_array(self):
        entries = []
        for i in range(len(self.digit_pins)):
            low = ('digitalWriteFastDigit%02dLow,' % (i))
            high = ('digitalWriteFastDigit%02dHigh,' % (i))
            entries.append(low)
            entries.append(high)
        return '\n  '.join(entries)

    def get_on_off_constants(self):
        if self.common_cathode:
            digit_on = 'LOW'
            digit_off = 'HIGH'
            segment_on = 'HIGH'
            segment_off = 'LOW'
        else:
            digit_on = 'HIGH'
            digit_off = 'LOW'
            segment_on = 'LOW'
            segment_off = 'HIGH'

        if self.use_transistor_drivers:
            digit_on, digit_off = digit_off, digit_on

        constants = []
        constants.append('static const uint8_t kDigitOn = %s;' % (digit_on))
        constants.append('static const uint8_t kDigitOff = %s;' % (digit_off))
        constants.append('static const uint8_t kSegmentOn = %s;' %
                         (segment_on))
        constants.append('static const uint8_t kSegmentOff = %s;' %
                         (segment_off))
        return '\n    '.join(constants)
