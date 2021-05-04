# Developer Notes

These are internal notes to help me remember the class hierarchies, their
dependencies and configuration combinations.

## Source Code Organization

All classes are in the `ace_segment` namespace, but the `.h` and `.cpp` files
are organized in subdirectories under `src/ace_segment`. The depedency graph is
roughly like this:

```
                    ace_segment/writer/
                      *Writer.h
                          |
                          v
                    ace_segment/
                      LedDisplay.h
                          |
                          v
                    ace_segment/
                      LedModule.h
                          ^
                          |
  +-----------------------+----------------+-------------------+
  |                       |                |                   |
  |                       |                |                   |
ace_segment/direct/ ace_segment/hc595/  ace_segment/tm1637/ ace_segment/max7219/
 DirectModule.h      SingleHc595Module.h Tm1637Module.h      Max7219Module.h
 DirectFast4Module.h DualHc595Module.h     /                /
       \               /                  /                /
        \             /                  /                /
         v           v                  /                /
       ace_segment/scanning/           /                /
         ScanningModule.h             /                /
         LedMatrix*.h                /                /
                        \            |               /
                         \           |              /
                          v          v             v
                           ace_segment/hw/
                             ClockInterface.h
                             GpioInterface.h
                             SoftSpiInterface.h
                             SoftSpiFastInterface.h
                             HardSpiInterface.h
                             HardSpiFastInterface.h
                             SoftWireInterface.h
                             SoftWireFastInterface.h
```

## Hardware and Communcation Abstraction

The classes under `ace_segment/hw` provide a thin layer of abstraction over
low-level hardware and communication. This allows alternate implementation to be
provided for testing purposes. In almost all cases, these classes are used as
C++ template parameters so the compiler is often able to optimize away the
abstraction so that the generated code is as-if the low-level functions were
called directly. ("Zero-cost abstraction").

* `ClockInterface`
    * access to clock and timer functions
    * micros()
    * millis()
* `GpioInterface`
    * access GPIO pins
    * digitalWrite()
    * pinMode()
* `SoftSpiInterface`
    * software SPI
* `SoftFastSpiInterface`
    * software SPI using `digitalWriteFast` library on AVR processors
* `HardSpiInterface`
    * hardware SPI
* `HardSpiFastInterface`
    * hardware SPI using `digitalWriteFast` for controlling the LatchPin
* `SoftWireInterface`
    * software implmentation of the TM1637 protocol similar to I2C
* `SoftWireFastInterface`
    * same as `SoftWireInterface` but using `digitalWriteFast` library

## LED Modules

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
    * Digit pins are wired to another 74HC575 shift register.
    * The 2 shift registers are daisy chained, so that they can be
      controlled at the same time using a single SPI transaction that sends
      16-bits.
    * The digit pins are assumed to be in the upper byte, the segment pins are
      in the lower byte.
* TM1637
    * 1 to 6 digit LED modules using a TM1637 chip
    * Uses a custom protocol similar to I2C.
* MAX7219
    * 1 to 8 digit LED modules using a MAX7219 chip
    * Can be daisy chained (not yet supported).
    * Uses SPI.

## LedMatrix Class

The `LedMatrix` classes provide access to the LED diodes using one of the 3
wiring configurations described above. There are 3 classes provided:

* LedMatrixDirect
* LedMatrixSingleHc595
* LedMatrixDualHc595

These classes were originally design for a more general LED module, so they use
slighlty more general terminologies. The digit pins are called "group" pins. And
the segment pins are called "element" pins.

The 74HC595 shift registers are controlled using SPI (Serial Peripheral
Interface). Most microcontrollers support either hardware SPI or software SPI.
You can choose to use either modes, and the library provides 2 classes that
allows this configuration:

* `SoftSpiInterface`
    * Uses `digitalWrite()` function to manually write the correct signal at the
      correct time. (Also called "big-banging").
* `HardSpiInterface`
    * Uses the `SPI.h` library that is provided on most microcontrollers
      using the Arduino platform.

Since the 74HC595 shift registers can be communicated using 2 different methods,
there are 5 combinations possible:

* LedMatrixDirect
* LedMatrixSingleHc595 + SoftSpiInterface
* LedMatrixSingleHc595 + HardSpiInterface
* LedMatrixDualHc595 + SoftSpiInterface
* LedMatrixDualHc595 + HardSpiInterface

## ScanningModule

Writes the LED segment pattern for a digit to the LED module, using one of the
`LedMatrix` classes. There are roughly 2 parts to the `ScanningModule` class:

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

## Tm1637Module

An implementation of `LedModule` that uses a TM1637 chip.

## Max7219Module

An implementation of `LedModule` that uses a MAX7219 chip.

## LedDisplay

A thin layer above an `LedModule` that provides a consistent API to the various
`Writer` classes.

## Writers

Writer classes provide mappings between characters, numbers or digits into
specific LED segment patterns using an internal "font" table. These classes use
the `LedDisplay` interface (implemented by `ScanningModule`), so in theory,
other types of LED displays could be used by these Writer classes.

This library currently provides 4 Writer classes:

* `NumberWriter`
    * `writeHexCharAt()`
    * `writeHexByteAt()`
    * `writeHexWordAt()`
    * `writeSignedDecimalAt()`
    * `writeUnsignedDecimalAt()`
    * `writeUnsignedDecimal2At()`
    * `clearToEnd()`
* `ClockWriter`
    * `writeCharAt()`
    * `writeChar2At()`
    * `writeBcd2At()`
    * `writeDec2At()`
    * `writeDec4At()`
    * `writeHourMinute(hh, mm)`
    * `writeColon()`
* `TemperatureWriter`
    * display temperature in C and F units
    * `writeTempAt()`
    * `writeTempDegAt()`
    * `writeTempDegCAt()`
    * `writeTempDegFAt()`
* `CharWriter`
    * `writeCharAt()`
* `StringWriter`
    * `writeStringAt(uint8_t pos, const char* cs)`
    * `writeStringAt(uint8_t pos, const __FlashStringHelper* fs)`
    * `clearToEnd()`
* `StringScoller`
    * scroll string left and right
