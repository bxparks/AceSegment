#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License
"""
Generate an implementation of ace_segment::Driver which uses the
digitalWriteFast() methods of https://github.com/NicksonYap/digitalWriteFast.
The resulting class can be used where a 'Driver' object created by
'DriverBuilder' would normally be used. There are 3 versions:
  * --segment_pins - equivalent to LedMatrixDirect
  * --segment_serial_pins - equivalent to LedMatrixSerial
  * --segment_spi_pins - equivalent to LedMatrixSpi

Usage: fast_driver.py [-h] [flags ...]

  --class_name name of class
  --segment_pins space-separated list of segment pins
  --digit_pins space-separate list of digit pins
  --common_cathode
  --common_anode
  --output_header print {class_name}.h file on stdout
  --output_source print {class_name}.cpp file on stdout
  --output_files generate the {class_name}.h and {class_name}.cpp files
  --[no]digital_write_fast

Examples:

  $ ./fast_driver.py --digit_pins 12 14 15 16 --segment_pins 4 5 6 7 8 9 10 11 \
      --class_name FastDirectDriver --output_files
  $ ./fast_driver.py --digit_pins 4 5 6 7 --segment_serial_pins 10 11 13 \
      --class_name FastSerialDriver --output_files
  $ ./fast_driver.py --digit_pins 4 5 6 7 --segment_spi_pins 10 11 13 \
      --class_name FastSpiDriver --output_files

Benchmarks for AceSegmentDemo
(frame rate: 60Hz, 4 fields/frame, 16 subfields/field):

  --segment_pins:
      LedMatrixDirect
          flash/static: 9252/506
          min: 8us; avg: 16us; max: 140us
      --digital_write_fast
          flash/static: 8544/490
          min: 8us; avg: 13us; max: 84us
      --nodigital_write_fast
          flash/static: 8640/490
          min: 8us; avg: 15us; max: 120us
  --segment_serial_pins:
      LedMatrixSerial
          flash/static: 9248/498
          min: 8us; avg: 20us; max: 212us
      --digital_write_fast
          flash/static: 8394/450
          min: 8us; avg: 13us; max: 80us
  --segment_spi_pins:
      LedMatrixSpi
          flash/static: 9248/498
          min: 12us; avg: 14us; max: 100us
      --digital_write_fast
          flash/static: 8434/451
          min: 8us; avg: 13us; max: 76us
"""

import argparse
import logging
import sys
import direct_generator
import serial_generator
import spi_generator


def main():
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Generate Fast Driver.')
    parser.add_argument(
        '--class_name', help='Name of the Driver class', required=True)
    parser.add_argument(
        '--segment_pins',
        help='Space-separated list of segment pins (ordered from 0 to 7)',
        nargs='+',
        type=int)
    parser.add_argument(
        '--segment_serial_pins',
        help='Space-separated list of segment pins (latch, data, clock)',
        nargs='+',
        type=int)
    parser.add_argument(
        '--segment_spi_pins',
        help='Space-separated list of SPI segment pins (latch, data, clock)',
        nargs='+',
        type=int)
    parser.add_argument(
        '--digit_pins',
        help='Comma-separated list of digit pins (ordered from 0 to n)',
        nargs='+',
        type=int,
        required=True)
    parser.add_argument(
        '--common_cathode',
        help='LED display uses common cathode on the digit (default: True)',
        dest='common_cathode',
        action="store_true",
        default=True)
    parser.add_argument(
        '--common_anode',
        help='LED display uses common anode on the digit (default: False)',
        dest='common_cathode',
        action="store_false")
    parser.add_argument(
        '--output_header',
        help='Output the *.h header file for debugging (default: False)',
        action="store_true")
    parser.add_argument(
        '--output_source',
        help='Output the *.cpp source file for debugging (default: False)',
        action="store_true")
    parser.add_argument(
        '--output_files',
        help='Create class_name.h and class_name.cpp files (default: False)',
        action="store_true",
        default=False)
    parser.add_argument(
        '--digital_write_fast',
        help='Use digitalWriteFast() instead of digitalWrite() (default: True)',
        dest='digital_write_fast',
        action="store_true",
        default=True)
    parser.add_argument(
        '--nodigital_write_fast',
        help='Use digitalWrite() instead of digitalWriteFast()',
        dest='digital_write_fast',
        action="store_false")
    args = parser.parse_args()

    # Configure logging.
    logging.basicConfig(level=logging.INFO)

    # How the script was invoked
    invocation = " ".join(sys.argv)

    # Get the DriverGenerator for the given segment_pin configuration.
    if args.segment_pins:
        generator = direct_generator.DriverGenerator(
            invocation, args.class_name, args.segment_pins, args.digit_pins,
            args.common_cathode, args.output_header, args.output_source,
            args.output_files, args.digital_write_fast)
    elif args.segment_serial_pins:
        generator = serial_generator.DriverGenerator(
            invocation, args.class_name, args.segment_serial_pins,
            args.digit_pins, args.common_cathode, args.output_header,
            args.output_source, args.output_files, args.digital_write_fast)
    elif args.segment_spi_pins:
        generator = spi_generator.DriverGenerator(
            invocation, args.class_name, args.segment_spi_pins,
            args.digit_pins, args.common_cathode, args.output_header,
            args.output_source, args.output_files, args.digital_write_fast)
    else:
        logging.error(
            "Must provide one of " +
            "(--segment_pins, --segment_serial_pins, --segment_spi_pins)")
        sys.exit(1)

    generator.run()


if __name__ == '__main__':
    main()
