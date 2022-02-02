# Developer Notes

These are internal notes to help me remember the class hierarchies, their
dependencies and configuration combinations.

## Source Code Organization

All classes are in the `ace_segment` namespace, but the `.h` and `.cpp` files
are organized in subdirectories under `src/ace_segment`. The dependency graph is
roughly like this:

```
                    ace_segment/
                      LedModule.h
                          ^
                          |
  +-----------------------+----------+---------------+----------------+
  |                       |          |               |                |
  |                       |          |               |                |
as/direct/          as/hc595/      as/tm1637/     as/max7219/     as/ht16k33/
DirectModule.h      HybridModule.h Tm1637Module.h Max7219Module.h Ht16k33Module
DirectFast4Module.h Hc595Module.h    |             /                /
       \               /             |            /                /
        \             /              |           /                /
         v           v               |          /      -----------
       ace_segment/scanning/         |         /      /
         ScanningModule.h            |        /      /
         LedMatrix*.h                |       /      /
                         \           |      /      /
                          \          |     /      /
                           v         v    v      v
                            ace_segment/hw/
                            ClockInterface.h
                            GpioInterface.h
```

## Hardware and Communication Abstraction

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
* AceSPI library
    * `SimpleSpiInterface`
        * software SPI
    * `SimpleSpiFastInterface`
        * software SPI using `digitalWriteFast` library on AVR processors
    * `HardSpiInterface`
        * hardware SPI
    * `HardSpiFastInterface`
        * hardware SPI using `digitalWriteFast` for controlling the LatchPin
* AceTMI library
    * `SimpleTmi1637Interface`
        * software implementation of the TM1637 protocol similar to I2C
    * `SimpleTmi1637FastInterface`
        * same as `SimpleTmi1637Interface` but using `digitalWriteFast` library
* AceWire library
    * `TwoWireInterface`
    * `SimpleWireInterface`
    * `SimpleWireFastInterface`

## Supported LED Modules

* DirectModule, DirectFast4Module
    * All LedMatrix segment and digit pins are directly connect to the
      microcontroller.
    * The segment pins have current limiting resistors.
    * There may transistors on the digit pins to enable higher current.
    * Not all common Arduino microcontrollers have enough pins to support a
      4-digit LED segment module.
* HybridModule
    * Segment pins are hooked up to a single 74HC575 shfit register,
      which can be accessed through SPI (software or hardware)
    * The digit pins are still directly connected to the microcontroller.
        * If there are only 4 digits on the LED module, then only 4 pins are
          needed on the MCU.
    * There are current limiting resistors between the LED segment pins and the
      shift register chip.
    * There may be transistors on digit pins to provide extra current.
* Hc595Module
    * Segment pins are wired to one 74HC575 shift register.
    * Digit pins are wired to another 74HC575 shift register.
    * The 2 shift registers are daisy chained, so that they can be
      controlled at the same time using a single SPI transaction that sends
      16-bits.
    * The digit pins are assumed to be in the upper byte, the segment pins are
      in the lower byte.
* Tm1637Module
    * 1 to 6 digit LED modules using a TM1637 chip
    * Uses a custom protocol similar to I2C.
* Max7219Module
    * 1 to 8 digit LED modules using a MAX7219 chip
    * Can be daisy chained (not yet supported).
    * Uses SPI.
* Ht16k33Module
    * 1 to 8 digit LED modules using a HT16K33 chip
    * Uses I2C.

## LedMatrix Class

The `LedMatrix` classes provide access to the LED diodes using one of the 3
wiring configurations:

* LedMatrixDirect
* LedMatrixSingleHc595
* LedMatrixDualHc595

These classes were originally design for a more general LED module, so they use
slightly more general terminologies. The digit pins are called "group" pins. And
the segment pins are called "element" pins.

The 74HC595 shift registers are controlled using SPI (Serial Peripheral
Interface). Most microcontrollers support either hardware SPI or software SPI.
You can choose to use either modes, and the library provides 2 classes that
allows this configuration:

* `SimpleSpiInterface`
    * Uses `digitalWrite()` function to manually write the correct signal at the
      correct time. (Also called "big-banging").
* `HardSpiInterface`
    * Uses the `SPI.h` library that is provided on most microcontrollers
      using the Arduino platform.

Since the 74HC595 shift registers can be communicated using 2 different methods,
there are 5 combinations possible:

* LedMatrixDirect
* LedMatrixSingleHc595 + SimpleSpiInterface
* LedMatrixSingleHc595 + HardSpiInterface
* LedMatrixDualHc595 + SimpleSpiInterface
* LedMatrixDualHc595 + HardSpiInterface

## ScanningModule

Writes the LED segment pattern for a digit to the LED module, using one of the
`LedMatrix` classes. There are roughly 2 parts to the `ScanningModule` class:

1) It implements the externally facing `LedModule` interface which has methods
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

## Remap Arrays

On many LED displays, the logical digit addresses used by the controller chip
are mapped to different physical addresses on the actual LED module. For
example, the 6-digit LED display using a TM1637 chip manufactured by diymore.cc
maps "0 1 2 3 4 5" so that the digits are displayed as "2 1 0 5 4 3".

The constructors of `Hc595Module`, `Tm1637Module` and `Max7219Module` accept
an optional `remapArray` parameter to account for the wiring variations. Various
predefined `kDigitRemapArrayXxx` are provided for the modules that I have
encountered.

These arrays are created using the following procedure:

1. Write the segment patterns for the numbers 0 to N-1 to the led module at
   their natural positions using the `LedModule::setPattern(pos, pattern)`
   method.
1. Observe the patterns that are actually displayed on the LED module. For
   example, on the 6-digit TM1637 modules, "0 1 2 3 4 5" is displayed as "2 1 0
   5 4 3".
1. The displayed array is the mapping from physical positions to their logical
   positions. In other words, if set set `uint8_t physicalToLogicalArray[] = {2
   1 0 5 4 3}', then we have that `logicalPos =
   physicalToLogicalArray[physicalPos]`.
1. Create the inverse mapping array by looping through each element of
   `physicalToLogicalArray` and creating a `logicalToPhysicalArray` whose index
   and value are flipped. For example, since `logicalToPhysical[0] == 2' we
   set `physicalToLogical[2] = 0`.
1. It turns out that for all the LED modules that I have looked at, the inverse
   array is **identical** to the original array, because the mapping from
   logical positions to physical positions are caused by pair-wise swaps between
   the controller pins and the LED module pins. In the example above,
   `logicalToPhysicalArray[] = {2 1 0 5 4 3}`.
1. Use this inverted `logicalToPhysicalArray` in the various constructors.

Graphically, the process is:

```
logical:            0 1 2 3 4 5

controller:         0 1 2 3 4 5
                     \ /   \ /
                      X     X
                     / \   / \
physical:           2 1 0 5 4 3

physicalToLogical:  2 1 0 5 4 3

logicalToPhysical:  2 1 0 5 4 3
```

If the wires from the controller chip to the LED digits are not simple pair-wise
swaps, then the `physicalToLogical` and `logicalToPhysical` arrays are not
identical. Here is an example where the first and second groups of 3 digits are
rotated instead of swapped pairwise:

```
logical:            0 1 2 3 4 5

controller:         0 1 2 3 4 5
                     \|/   \|/
                     /|\   /|\
physical:           2 0 1 5 3 4

physicalToLogical:  2 0 1 5 3 4

logicalToPhysical:  1 2 0 4 5 3
```
