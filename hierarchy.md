# Class Hierarchy and Wiring Configurations

Internal notes to help me remember the various class hierarchies and
configuration combinations. I think I can create a templatized wrapper class
that incorporates all this information and expose a much easier API to the user
to help create the right set of objects for the given LED wiring situtation.

## Hardware Abstraction

A class that encapsulates the lowest level of access to the microcontroller's
GPIO pins and internal timers:

* Hardware Class
    * digitalWrite()
    * pinMode()
    * micros()
    * millis()

Most (all?) if the following classes will take the `Hardware` class as a C++
template parameter. This allows us to inject a different hardware
(`TestableHardware`) for unit testing purposes.

## Wiring

**Wiring Types**

* Direct
    * All LedMatrix segment and digit pins are directly connect to the the
      microcontroller.
    * The segment pins have current limiting resistors.
    * There may transistors on the digit pins to enable higher current.
    * Not all common Arduino microcontrollers have enough pins to support a
      4-digit LED segment module.
* Single 74HC595 Shift Register
    * LED segment pins are hooked up to a 74HC575 shfit register,
      which can be accessed through SPI (software or hardware)
    * The digit pins are still directly connected to the microtroller. If there
      are only 4 digits on the LED module, then only 4 pins are needed on the
      MCU.
    * There are current limiting resistors between the LED segment pins and the
      shift register chip.
    * There may be transistors on digit pins to provide extra current.
* Dual 74HC595 Shift Register
    * Segment pins are wired to one 74HC575 shift register.
    * Digit [ins are wired to another 74HC575 shift register.
    * The 2 shift registers are daisy chained, so that they can be
      controlled at the same time using a single SPI transaction that sends
      16-bits.
    * The digit pins are assumed to be in the upper byte, the segment pins are
      in the lower byte.

## LedMatrix Class

The `LedMatrix` classes provide access to the LED diodes using one of the 3
wiring configurations described above. There are 3 classes provided:

* LedMatrixDirect
* LedMatrixSingleShiftRegister
* LedMatrixDualShiftRegister

These classes were originally design for a more general LED module, so they use
slighlty more general terminologies. The digit pins are called "group" pins. And
the segment pins are called "element" pins.

The 74HC595 shift registers are controlled using SPI (Serial Peripheral
Interface). Most microcontrollers support either hardware SPI or software SPI.
You can choose to use either modes, and the library provides 2 classes that
allows this configuration:

* `SwSpiAdapter`
    * Uses `digitalWrite()` function to manually write the correct signal at the
      correct time. (Also called "big-banging").
* `HwSpiAdapter`
    * Uses the `SPI.h` library that is provided on most microcontrollers
      using the Arduino platform.

Since the 74HC595 shift registers can be communicated using 2 different methods,
there are 5 combinations possible:

* LedMatrixDirect
* LedMatrixSingleShiftRegister + SwSpiAdapter
* LedMatrixSingleShiftRegister + HwSpiAdapter
* LedMatrixDualShiftRegister + SwSpiAdapter
* LedMatrixDualShiftRegister + HwSpiAdapter

## SegmentDisplay

Writes the LED segment pattern for a digit to the LED module, using one of the
`LedMatrix` classes. There are roughly 2 parts to the `SegmentDisplay` class:

1) It implements the externally facing `LedDisplay` interface which has methods
that allow LED segment patterns for each digit to be written into an internal
pattern buffer.
    * `writePatternAt(digit, pattern, brightness)`
    * `writeBrightnessAt(digit, brightness)`
    * `writeDecimalPointAt(digit, state)`
2) It has rendering methods which multiplexes these segment patterns to the LED
module at a configurable frame rate (e.g. 60 frames/second). Only one digit is
activated at a precise moment in time, but when the frame rate is high enough,
it as if the entire LED module is lit up.
    * `renderFieldNow()` - render the current field immediately
    * `renderFieldWhenReady()` - render the current field if the time is right

## Writers

Writer classes provide mappings between characters, numbers or digits into
specific LED segment patterns using an internal "font" table. These classes use
the `LedDisplay` interface (implemented by `SegmentDisplay`), so in theory,
other types of LED displays could be used by these Writer classes.

This library currently provides 3 Writer classes:

* `HexWriter(Renderer*)`
    * `writeHexAt()`
    * `writeBrightnessAt()`
    * `writeDecimalPointAt()`
* `CharWriter(Renderer*)`
    * `writeCharAt()`
    * `writeBrightnessAt()`
    * `writeDecimalPointAt()`
* `StringWriter(Renderer*)`
    * `writeStringAt()`
* `ClockWriter(Renderer*)`
    * `writeBcdAt()`
    * `writeDecimalAt()`
    * `writeClock(hh, mm)`
    * `writeBcdClock(hh, mm)`
    * `writeColon()`
