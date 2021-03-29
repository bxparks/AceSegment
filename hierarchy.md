# Class Hierarchy and Wiring Configurations

Internal notes to help me remember the various class hierarchies and
configuration combinations. I think I can create a templatized wrapper class
that incorporates all this information and expose a much easier API to the user
to help create the right set of objects for the given LED wiring situtation.

## Hardware Abstraction

* Hardware Class
    * virtual ~Hardware()
    * virtual digitalWrite()
    * virtual pinMode()
    * virtual micros()
    * virtual millis()

* SpiAdapter
    * virtual spiBegin()
    * virtual spiEnd()
    * virtual spiTransfer()
    * virtual spiTransfer16()
    * Two implementation classes:
        * SwSpiAdapter
        * HwSpiAdapter

## Wiring and LedMatrix Classes

**Wiring Types**

* Split
    * LedMatrixDirect (all pins direct to MCU)
    * LedMatrixPartialSpi (element pins through 74HC575 via SPI, group pins
* Merged
    * LedMatrixFullSpi (all pins through 74HC575 via HwSPI)

**LedMatrix Class**

* Implements the 3 wiring combinations listed above.
* The resistor determines which LEDs can be turned on at the same time.
    * LEDs with resistors become "element"
    * The other lines become "groups"

## Driver Classes

* contains array of digit patterns to render (array provided externally)
* knows how to transfer the bit patterns in DimmablePattern object to the
    LED matrix.
* keeps track of internal information to determine which field of which
    frame to render to the LED segments
* each call to `displayCurrentField()` display one field of a frame
* methods
    * configure(), finish()
    * displayCurrentField()
    * getFieldsPerFrame()
    * isBrightnessSupported()
    * setPattern()
    * setBrightness()
* subclasses
    * SplitDigitDriver
        * SplitDirectDigitDriver (uses LedMatrixSplitDirect)
        * SplitSerialDigitDriver
        * SplitSpiDigitDriver
    * MergedDigitDriver
        * MergedSerialDigitDriver
        * MergedSpiDigitDriver
    * SplitSegmentDriver (X - archived)
    * MergedSegmentDriver (X - not implemented)

## CompositeDrivers

Composite classes combine the correct `LedMatrix` object with the matching
`Driver` object. The composite object multiplely inherits from both `LedMatrix`
and `Driver` class. The number of combinations are:

* (SplitDigitDriver) x LedMatrixDirect = 1
* (SplitDigitDriver) x (LedMatrixPartialSpi (SW | HW)) = 2
* (MergedDigitDriver) x (LedMatrixFullSpi (SW | HW)) = 2
* Total: 1 + 2 + 2 = 5
* classes
    * SplitDirectDigitDriver
        * LedMatrixDirect
        * SplitDigitDriver
        * (Resistors on segments, transistor on digits, all pins direct to MCU)
    * SplitSerialDigitDriver -> PartialSwSpiDriver
        * SplitDigitDriver
        * LedMatrixPartialSpi (SW)
        * (Resistors on segments, transistors on digits thru 74HC595 via SwSPI)
    * SplitSpiDigitDriver -> PartialHwSpiDriver
        * SplitDigitDriver
        * LedMatrixPartialSpi (HW)
        * (Resistors on segments, transistors on digits thru 74HC595 via HwSPI)
    * MergedSerialDigitDriver -> FullSwSpiDriver
        * MergedDigitDriver
        * LedMatrixFullSpi (SW)
        * (Resistors on segments, transistors on digits, all pins via SwSPI)
    * MergedSpiDigitDriver -> FullHwSpiDriver
        * MergedDigitDriver
        * LedMatrixFullSpi (HW)
        * (Resistors on segments, transistors on digits, all pins via HwSPI)

## Renderer

Writes the `DimmablePattern` into the into the `Driver` class.

* `updateFrame()`
    * -> `mDriver->setPattern(digit, pattern, brightness)`
* deposes each frame into multiple fields, and calls the Driver for each field
    * `renderFieldWhenReady()` - render by polling
    * `renderField()` - render immediately, called from ISR
* `writePatternAt(digit, pattern, brightness)`
* `writeBrightnessAt(digit, brightness)`
* `writeDecimalPointAt(digit, state)`

## Writers

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
