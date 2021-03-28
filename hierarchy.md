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
    * virtual spiBegin()
    * virtual spiEnd()
    * virtual spiTransfer()
    * virtual spiTransfer16()

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
    * SplitSegmentDriver
    * MergedDigitDriver
    * MergedSegmentDriver (X - not implemented)

## CompositeDrivers

Composite classes combine the correct `LedMatrix` object with the matching
`Driver` object. The composite object multiplely inherits from both `LedMatrix`
and `Driver` class. The number of combinations are:

* (SplitDigitDriver | SplitSegmentDriver) x LedMatrixDirect = 2
* (SplitDigitDriver | SplitSegmentDriver)
    x (LedMatrixSplitSerial | LedMatrixSplitSpi) = 4
    * But we don't implement SplitSegmentDriver, so only 2
* (MergedDigitDriver | MergedSegmentDriver)
    x (LedMatrixMergedSerial | LedMatrixMergedSpi) = 4
    * But we don't implement MergedSegmentDriver, so only 2
* Total: 2 + 2 + 2 = 6
* classes
    * SplitDirectDigitDriver
        * LedMatrixSplitDirect
        * SplitDigitDriver
        * (Resistors on segments, transistor on digits, all pins direct to MCU)
    * SplitDirectSegmentDriver
        * LedMatrixSplitDirect
        * SplitSegmentDriver
        * (Resistors on digits, transitors on segments, all pins direct to MCU)
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

Translates `StyledPatter` (bit patterns with style meta information) into
appropriate calls into `Driver` class. Style data is an index into `StyleTable`.

* StyleTable
    * array of `Styler` objects
    * index 0 is the no-op styler
* `updateFrame()`
    * -> `renderStyledPattern()`
        * -> `styler->apply()`
        * -> `mDriver->setPattern(digit, pattern, brightness)`
* `writePatternAt(digit, pattern, style)`
* `writeStyleAt(digit, style)`
* `writeDecimalPointAt(digit, state)`
* `renderFieldWhenReady()` - render by polling
* `renderField()` - render immediately, called from ISR

## Stylers

* Styler
    * `calcForFrame()`
    * `apply(uint8_t* pattern, uint8_t* brightness)` - update brightness
    * `bool requiresBrigthness()`
* subclasses
    * PulseStyler
        * pulsate the digit
    * BlinkStyler
        * blink the digit

## Writers

* `HexWriter(Renderer*)`
    * `writeHexAt()`
    * `writeStyleAt()`
    * `writeDecimalPointAt()`
* `CharWriter(Renderer*)`
    * `writeCharAt()`
    * `writeStyleAt()`
    * `writeDecimalPointAt()`
* `StringWriter(Renderer*)`
    * `writeStringAt()`
* `ClockWriter(Renderer*)`
    * `writeBcdAt()`
    * `writeDecimalAt()`
    * `writeClock(hh, mm)`
    * `writeBcdClock(hh, mm)`
    * `writeColon()`
