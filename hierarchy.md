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
    * virtual shiftOut()
    * virtual micros()
    * virtual millis()

* SpiAdapter
    * virtual spiBegin()
    * virtual spiEnd()
    * virtual spiTransfer()
    * virtual spiTransfer16()
    * SwSpiAdapter
    * HwSpiAdapter

## Wiring and LedMatrix Classes

**Wiring Types**

* Split
    * SplitDirect (all pins direct to MCU)
    * SplitSerial (element pins through 74HC575 via SwSPI, group pins
        direct)
    * SplitSpi (element pins through 74HC575 via HwSPI, group pins direct)
* Merged
    * MergedSerial (all pins through 74HC575 via SwSPI)
    * MergedSpi (all pins through 74HC575 via HwSPI)

**LedMatrix Class**

* Implements the 5 wiring combinations listed above.
* Uses `Hardware` to determine whether to use `digitalWrite()` or
    `spiTransfer()`
* The resistor determines which LEDs can be turned on at the same time.
    * LEDs with resistors become "element"
    * The other lines become "groups"
* Does not have any state information
* subclasses (5 options)
    * LedMatrixSplit
        * LedMatrixSplitDirect (all pins are directly driven)
        * LedMatrixSplitSerial (element pins via SwSPI)
            * LedMatrixSplitSpi (element pins via HwSPI)
    * LedMatrixMerged
        * LedMatrixMergedSerial (group and element pins via SwSPI)
            * LedMatrixMergedSpi (group and element pins via HwSPI)

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
    * MergedDigitDriver
    * SplitSegmentDriver (X - archived)
    * MergedSegmentDriver (X - not implemented)

## CompositeDrivers

Composite classes combine the correct `LedMatrix` object with the matching
`Driver` object. The composite object multiplely inherits from both `LedMatrix`
and `Driver` class. The number of combinations are:

* (SplitDigitDriver) x LedMatrixDirect = 1
* (SplitDigitDriver) x (LedMatrixSplitSerial | LedMatrixSplitSpi) = 2
* (MergedDigitDriver) x (LedMatrixMergedSerial | LedMatrixMergedSpi) = 2
* Total: 1 + 2 + 2 = 5
* classes
    * SplitDirectDigitDriver
        * LedMatrixSplitDirect
        * SplitDigitDriver
        * (Resistors on segments, transistor on digits, all pins direct to MCU)
    * SplitSerialDigitDriver
        * LedMatrixSplitSerial
        * SplitDigitDriver
        * (Resistors on segments, transistors on digits thru 74HC595 via SwSPI)
    * SplitSpiDigitDriver
        * LedMatrixSplitSpi
        * SplitDigitDriver
        * (Resistors on segments, transistors on digits thru 74HC595 via HwSPI)
    * MergedSerialDigitDriver
        * LedMatrixMergedSerial
        * MergedDigitDriver
        * (Resistors on segments, transistors on digits, all pins via SwSPI)
    * MergedSpiDigitDriver
        * LedMatrixMergedSpi
        * MergedDigitDriver
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
