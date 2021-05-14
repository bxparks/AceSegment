# AceSegment

[![AUnit Tests](https://github.com/bxparks/AceSegment/actions/workflows/aunit_tests.yml/badge.svg)](https://github.com/bxparks/AceSegment/actions/workflows/aunit_tests.yml)

An adjustable, configurable, and extensible framework for rendering seven
segment LED displays on Arduino platforms. The library supports multiple types
of LED displays:

* LED modules using the TM1637 controller chip
* LED modules using the MAX7219/MAX7221 controller chip
* LED modules using two 74HC595 shift register chips
* LED modules using a hybrid of one 74HC595 chip and direct GPIO connections
* LED modules directly connected to the GPIO pins, no controller chips

The first 3 types are readily available from consumer sources such as Amazon and
eBay, in multiple colors and sizes. The final 2 types of modules (hybrid and
directly connected) are usually custom creations. The AceSegment library hopes
to support as many of these configurations as possible within a single
framework.

Different types of seven-segment LED modules using different controller chips
are similar enough to each other that code at the application layer should be
mostly agnostic to the hardware differences. The AceSegment library is organized
into hardware-dependent components and hardware-independent components to allow
application code to be written without worrying too much about the low-level
details of the specific LED module.

**Version**: 0.4+ (2021-04-17)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**Status**: This is a **work in progress**. It is not ready for public
consumption.

## Table of Contents

* [Installation](#Installation)
    * [Source Code](#SourceCode)
    * [Dependencies](#Dependencies)
* [Documentation](#Documentation)
    * [Examples](#Examples)
* [High Level Overview](#HighLevelOverview)
    * [Classes](#Classes)
    * [Dependency Diagram](#DependencyDiagram)
    * [Digit and Segment Addressing](#DigitAndSegmentAddressing)
* [Usage](#Usage)
    * [Include Header and Namespace](#HeaderAndNamespace)
    * [Tm1637Module](#Tm1637Module)
    * [Max7219Module](#Max7219Module)
    * [Hc595Module](#Hc595Module)
        * [Rendering the Hc595Module](#RenderingHc595Module)
    * [LedDisplay](#LedDisplay)
    * [NumberWriter](#NumberWriter)
    * [ClockWriter](#ClockWriter)
    * [TemperatureWriter](#TemperatureWriter)
    * [CharWriter](#CharWriter)
    * [StringWriter](#StringWriter)
    * [StringScroller](#StringScroller)
* [Advanced Usage](#AdvancedUsage)
    * [DigitalWriteFast on AVR](#DigitalWriteFast)
    * [ScanningModule](#ScanningModule)
* [Resource Consumption](#ResourceConsumption)
* [System Requirements](#SystemRequirements)
    * [Hardware](#Hardware)
    * [Tool Chain](#ToolChain)
    * [Operating System](#OperatingSystem)
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="Installation"></a>
## Installation

The latest stable release will eventually be available in the Arduino IDE
Library Manager. Search for "AceSegment". Click install. (It is not there
yet.)

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceSegment), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceSegment`.)

The `master` branch contains the stable release.

<a name="SourceCode"></a>
### Source Code

The source files are organized as follows:
* `src/AceSegment.h` - main header file
* `src/ace_segment/` - implementation files
* `src/ace_segment/testing/` - internal testing files
* `tests/` - unit tests which require [AUnit](https://github.com/bxparks/AUnit)
* `examples/` - example sketches
* `docs/` - contains the doxygen docs and additional manual docs

<a name="Dependencies"></a>
### Dependencies

This library depends on the following additional libraries:

* AceCommon (https://github.com/bxparks/AceCommon)

The unit tests depend on:

* AUnit (https://github.com/bxparks/AUnit)

Some of the examples may depend on:

* AceButton (https://github.com/bxparks/AceButton)

<a name="Documentation"></a>
## Documentation

* this `README.md` file
* [advanced.md](docs/advanced.md): Advanced Usage Guide
* [Doxygen docs published on GitHub Pages](https://bxparks.github.io/AceSegment/html).

<a name="Examples"></a>
### Examples

The following example sketches are provided:

* Basic
    * [Tm1637Demo.ino](examples/Tm1637Demo)
    * [Max7219Demo.ino](examples/Max7219Demo)
    * [Hc595Demo.ino](examples/Hc595Demo)
    * [DirectModuleDemo.ino](examples/DirectModuleDemo)
    * [HybridDemo.ino](examples/HybridDemo)
* Advanced
    * [Tm1637DualDemo.ino](examples/Tm1637DualDemo)
    * [ModulatingDemo.ino](examples/ModulatingDemo)
    * [ScanningDirectDemo.ino](examples/ScanningDirectDemo)
    * [DirectFast4ModuleDemo.ino](examples/DirectModuleDemo)
    * [AceSegmentDemo.ino](examples/AceSegmentDemo)
        * runs through some features of the library
        * uses 2 buttons for "single step" debugging mode
* Benchmarks
    * [AutoBenchmark.ino](examples/AutoBenchmark): performs CPU benchmarking of
      most of the supported configurations of the framework
    * [MemoryBenchmark.ino](examples/MemoryBenchmark): determines the size of
      the various components of the library

<a name="HighLevelOverview"></a>
## High Level Overview

<a name="Classes"></a>
### Classes

Here are the classes in the library which will be most useful to the
end-users, listed roughly from low-level to higher-level classes which often
depend on the lower-level classes:

* SpiInterface
    * Thin-wrapper classes for communicating with the LED module that support
      SPI
    * Used by `Max7219Module` and `Hc595Module`.
    * There are 4 implementations.
        * `SoftSpiInterface`
            * Software SPI using `shiftOut()`
        * `SoftSpiFastInterface`
            * Software SPI using `digitalWriteFast()` on AVR processors
        * `HardSpiInterface`
            * Hardware SPI using `digitalWrite()` to control the latch pin.
        * `HardSpiFastInterface`
            * Hardware SPI using `digitalWriteFast()` to control the latch pin.
* WireInterface
    * Thin-wrapper classes to communicating with LED modules using the TM1637
      protocol. Similar to I2C but not exactly the same.
    * Used by `Tm1637Module`.
    * There are 2 implementations:
        * `SoftWireInterface`
            * Implement the TM1637 protocol using `digitalWrite()`.
        * `SoftWireFastInterface`
            * Implement the TM1637 protocol using `digitalWriteFast()`.
* `LedModule`
    * Base interface for all hardware dependent implementation of a
      seven-segment LED module.
    * `Tm1637Module`
        * An implementation using a TM1637 controller.
    * `Max7219Module`
        * An implementation using a MAX7219 controller.
    * `Hc595Module`
        * An implementation using two 74HC595 shift registers.
    * `HybridModule`
        * An implementation using one 74HC595 shift registers
          to handle the 8 segment lines, with the digit lines directly connected
          to the GPIO pins of the micrcontroller.
    * `DirectModule`
        * An implementation with all segment and digit pins connected directly
          to the microcontroller.
* `LedDisplay`
    * Class that knows how to write segment bit patterns to an `LedModule`.
    * Provides a single, common API to the various Writer classes.
* Writers
    * Helper classes built on top of the `LedDisplay` which provide higher-level
      interface to the LED module, such as printing numbers, time (hh:mm),
      and ASCII characters and strings.
    * `NumberWriter`
        * A class that writes integers in decimal or hexadecimal format to the
          `LedDisplay`.
        * A few additional characters are supported: `kCharSpace`, `kCharMinus`
    * `ClockWriter`
        * A class that writes a clock string "hh:mm" to `LedDisplay`.
        * A few additional symbols are supported: `kCharSpace`, `kCharMinus` and
          `kPatternA` ("A" for AM) and `kPatternP` ("P" for PM).
    * `TemperatureWriter`
        * A class that writes temperatures with a degrees symbol or optionally
          with "C" or "F" symbol.
    * `CharWriter`
        * A class that convert an ASCII character represented by a `char` (code
          0-127) to a bit pattern used by `SegmentDriver` class.
        * Not all ASCII characters can be rendered on a seven segment display
          legibly but the `CharWriter` tries its best.
    * `StringWriter`
        * A class that prints strings of `char` to a `CharWriter`, which in
          turns, prints to the `LedDisplay`.
    * `StringScoller`
        * Scroll a string left and right.

<a name="DependencyDiagram"></a>
### Dependency Diagram

The dependency diagram among these classes looks something like this
(simplified for ease of understanding):

```
        StringScroller
        StringWriter      ClockWriter  TemperatureWriter
               |              \           /
               V               v         v
            CharWriter         NumberWriter
                    \            /
                     v          v
                      LedDisplay
                          |            (hardware independent)
--------------------------|-------------------------------------
                          |            (hardware dependent)
                          v
                       LedModule
                          ^
                          |
      +-------------------+-----------------+--------------+--------------+
      |                   |                 |              |              |
Tm1637Module          Max7219Module     Hc595Module  HybridModule  DirectModule
      |                         \           |         /
      |                          \          |        /
      v                           v         v       v
SoftWireInterface                 SoftSpiInterface
SoftWireFastInterface             SoftSpiFastInterface
                                  HardSpiInterface
                                  HardSpiFastInterface
```

<a name="DigitAndSegmentAddressing"></a>
### Digit and Segment Addressing

The `LedModule` and `LedDisplay` classes use the following conventions for
addressing the digits and segments:

* digits start at position 0 on the left and increase to the right
* segments are assigned bits 0 to 7 of an unsigned byte (type `uint8_t`) with
  segment `a` assigned to bit 0, segment `g` assigned to bit 6, and the optional
  decimal point assigned to bit 7

```
+------------+  +------------+           +------------+
|   aaaa     |  |   aaaa     |           |   aaaa     |
|  f    b    |  |  f    b    |           |  f    b    |
|  f    b    |  |  f    b    |           |  f    b    |
|   gggg     |  |   gggg     |  *  *  *  |   gggg     |
|  e    c    |  |  e    c    |           |  e    c    |
|  e    c    |  |  e    c    |           |  e    c    |
|   dddd  dp |  |   dddd  dp |           |   dddd  dp |
+------------+  +------------+           +------------+
  Digit 0         Digit 1                 Digit {N-1}


Segment: dp g f e d c b a
   Bit:  7  6 5 4 3 2 1 0
```

Some 4-digit LED modules are designed to be used in a clock to display the hour
and minute components of the time as `HH:MM`. In these modules, it is common for
the decimal point on Digit 1 to be replaced with the `colon` segment between
Digit 1 and Digit 2. In these modules, sometimes the decimal points for the
other digits work normally, but sometimes, the remaining decimal points do not
work at all.

Sometimes the LED modules are hardwired to the controller chip so that the
positions of the digits (and sometimes segments) do not not match the logical
arrangement described above. Fortunately, we can rearrange the digit and segment
bits in software so that everything is remapped to their correct places. For
example, the diymore.cc 6-digit TM1637 LED module is wired so that the digits
are displayed like this:

```
2, 1, 0, 5, 4, 3
```

The `Tm1637Module` class can remap these digits into their correct order:

```
0, 1, 2, 3, 4, 5
```

Since it is impossible to predict all the different ways that the LED modules
can be wired, various classes in the AceSegment library allow the remapping
array to be supplied by the library user.

<a name="Usage"></a>
## Usage

<a name="HeaderAndNamespace"></a>
### Include Header and Namespace

Only a single header file `AceSegment.h` is required to use this library.
To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_segment` namespace. To use the code without
prepending the `ace_segment::` prefix, use the `using` directive:

```C++
#include <AceSegment.h>
using namespace ace_segment;
```

<a name="Tm1637Module"></a>
### Tm1637Module

LED modules based on the Titan TM1637 controller chips are abundant on Amazon
and eBay. The controller chip supports up to 6 digits. Consumer LED modules
seem have either 4 digits or 6 digits.

![TM1637 LED Module](docs/tm1637/TM1637_4_digits.png)

The TM1637 controller uses a wire protocol that is very close to, but not quite
the same as, I2C. Unfortunately this means that we cannot use the usual `Wire`
library. The `Tm1637Module` class uses bit-banging to send the correct bits in
the right order with the right timing to the chip.

The `BIT_DELAY` parameter below is the number of microseconds to wait between
each bit transition (0 to 1, or 1 to 0). According the datasheet of the TM1637
chip, it can support oscillator frequencies as high as 450 kHz, which means that
theoretically, the `BIT_DELAY` could be as low as 1 microseconds.

However, the LED modules manufactured by diymore.cc (shown above) contains a 20
nF capacitor and a 4.7k ohm pullup resistor on each of the `DIO` and `CLK`
lines. This seems to be a design flaw, because the capacitor is about 100X
larger than it should be, it should have been only be about 200 pF. The effect
of the large capacitor is that bit transitions on these lines take an incredibly
long time due the `RC` time constant of 94 microseconds. After experimenting
with various values, it seems like a `BIT_DELAY` of 100 microseconds will
usually work.

A `BIT_DELAY` of 100 microseconds means that the time to transmit 4 digits to
the TM1637 chip becomes about 22 milliseconds. The time to transmit a single
digit is about 10 milliseconds due to the overhead in the protocol. Since these
durations are so large, the `Tm1637Module` class does not send the segment bit
patterns directly to the TM1637 controller when `Tm1637Module::setPattern()`
method is called. Instead, the class holds a buffer of segment patterns, and
keeps track of a set of dirty bits. The buffer is sent to the TM1637 controller
upon the execution of the `Tm1637Module::flush()` or
Tm1637Module::flushIncremental()` method.

The `flush()` method unconditionally sends all digits and the brightness
information to the TM1637 chip in a single protocol transmission. For 4 digits,
using a `BIT_DELAY` of 100 micros, the entire tranmission takes about 22 millis.
For 6 digits, this method takes about 27 millis. The `flush()` method is a
blocking call, nothing else can be done during this time (outside of
interrupts). This can be a problem for processors like the ESP8266 which must
yield back to the main loop every 20-40 milliseconds to keep its WiFi stack
working. Otherwise, the watch dog timer performs a system reboot.

The `flushIncremental()` method was created to reduce the amount of time spent
in the blocking call to `flush()`. The `Tm1637Module` class contains a set of
dirty bits which keeps track of which digits have been modified since the last
flush to the TM1637 chip. It also keeps a reference counter that keeps track of
the digit that `flushIncremental()` handled previously. When
`flushIncremental()` is called, only a single digit is sent to the TM1637, and
only if that digit was marked to be dirty. Each subsequent call to
`flushIncremental()` examines the next digit. When the last digit is handled,
the next call to `flushIncremental()` looks at the brightness level, and sends
that information to the TM1637 chip if the brightness dirty bit is set.

The configuration of the `Tm1637Module` class for the 4-digit module looks like
this:

```C++
#include <AceSegment.h>
using namespace ace_segment;

const uint8_t CLK_PIN = 10;
const uint8_t DIO_PIN = 9;
const uint16_t BIT_DELAY = 100;
const uint8_t NUM_DIGITS = 4;

using WireInterface = SoftWireInterface;
WireInterface wireInterface(CLK_PIN, DIO_PIN, BIT_DELAY);
Tm1637Module<WireInterface, NUM_DIGITS> ledModule(wireInterface);

LedDisplay display(ledModule);

void setupAceSegment() {
  wireInterface.begin();
  ledModule.begin();
}

// Flush to LED module every 50 millis.
void flushModule() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 50) {
    prevFlushMillis = nowMillis;
    //ledModule.flush();
    ledModule.flushIncremental();
  }
}

void setup() {
  setupAceSegment();
  ...
}

void loop() {
  flushModule();
  ...
}
```

The configuration of the `Tm1637Module` class for the 6-digit module is slightly
more complicated because the digits are wired to be in the order of `2, 1, 0, 5,
4, 3`. A predefined remap array `kDigitRemapArray6Tm1637` must be given to the
`Tm1637Module` constructor, like this:

```C++
#include <Arduino.h>
#include <AceSegment.h>

using namespace ace_segment;

const uint8_t CLK_PIN = 10;
const uint8_t DIO_PIN = 9;
const uint16_t BIT_DELAY = 100;
const uint8_t NUM_DIGITS = 4;

using WireInterface = SoftWireInterface;
WireInterface wireInterface(CLK_PIN, DIO_PIN, BIT_DELAY);
Tm1637Module<WireInterface, NUM_DIGITS> ledModule(
    wireInterface, kDigitRemapArray6Tm1637);

LedDisplay display(ledModule);

void setupAceSegment() {
  wireInterface.begin();
  ledModule.begin();
}

// Flush to LED module every 50 millis.
void flushModule() {
  static uint16_t prevFlushMillis;
  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 50) {
    prevFlushMillis = nowMillis;
    //ledModule.flush();
    ledModule.flushIncremental();
  }
}

void setup() {
  setupAceSegment();
  ...
}

void loop() {
  flushModule();
  ...
}
```

<a name="Max7219Module"></a>
### Max7219Module

These LED modules use the MAX7219 controller chip which communicate using SPI. A
single chip supports 8 segments of up to 8 digits. Multiple controller chips can
be daisychained to support more than 8 digits. The 8-digit module is readily
available commercially from multiple suppliers on Amazon and eBay, and they look
like this:

![MAX7219 LED Module](docs/max7219/MAX7219_8_digits.png)

The configuration of the `Max7219Module` class for the 8-digit module looks like
this:

```C++
#include <Arduino.h>
#include <AceSegment.h>

using namespace ace_segment;

const uint8_t LATCH_PIN = 10;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t NUM_DIGITS = 8;

using SpiInterface = HardSpiInterface;
SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

Max7219Module<SpiInterface, NUM_DIGITS> ledModule(
    spiInterface, kDigitRemapArray8Max7219);
LedDisplay display(ledModule);

void setupAceSegment() {
  spiInterface.begin();
  ledModule.begin();
}

// Flush to LED module every 100 millis.
void flushModule() {
  static uint16_t prevFlushMillis;
  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;
    ledModule.flush();
  }
}

void setup() {
  setupAceSegment();
  ...
}

void loop() {
  flushModule();
  ...
}
```

<a name="Hc595Module"></a>
### Hc595Module

The 74HC595 shift register is a well-known chip that can be used to control
seven-segment LED modules. Each chip converts 8 serial bits into 8 parallel pins
which can source or sink about 12 mA of current each. With two 74HC595 chips,
one chip can control the segment pins, the other can control the common digit
pins, and the two chips can be daisy chained together. The chips can be
programmed using the straighforward SPI protocol.

Recently (since about Aug 2020?), off-the-shelf 8-digit LED modules using two
74HC595 have become common on Amazon and eBay, in multiple colors. They look
like this:

![HC595 LED Module](docs/hc595/HC595_8_digits.png)

The configuration of the `Hc595Module` class for the 8-digit module looks like
this:

```C++
#include <Arduino.h>
#include <AceSegment.h>

using namespace ace_segment;

const uint8_t LATCH_PIN = 10;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;

const uint8_t NUM_DIGITS = 8;
const uint8_t FRAMES_PER_SECOND = 60;

const uint8_t SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
const uint8_t DIGIT_ON_PATTERN = LedMatrixBase::kActiveHighPattern;
const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

using SpiInterface = HardSpiInterface;
SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

Hc595Module<SpiInterface, NUM_DIGITS> ledModule(
    spiInterface,
    SEGMENT_ON_PATTERN,
    DIGIT_ON_PATTERN,
    FRAMES_PER_SECOND,
    HC595_BYTE_ORDER,
    REMAP_ARRAY
);

LedDisplay display(ledModule);

void setupAceSegment() {
  spiInterface.begin();
  ledModule.begin();
}

// Flush to LED module when ready. Call this as fast as possible, allowing
// the internal counters to figure out when to actually render.
void flushModule() {
  ledModule.renderFieldWhenReady();
}

void setup() {
  setupAceSegment();
  ...
}

void loop() {
  flushModule();
  ...
}
```

<a name="RenderingHc595Module"></a>
#### Rendering the Hc595Module

Unlike the TM1637 and the MAX7219 chips, the 74HC595 does not automatically
multiplex through the digits of the LED module, giving the appearance that all
the digits are on at the same time. The scanning must be performed by the
microcontroller itself. The `Hc595Module::renderFieldWhenReady()` performs that
task. It should be called as quickly as possible, usually faster than `60 *
NUM_DIGITS` times per second, so for a 4-digit LED module, that's 240 times a
second, or every 5 milliseconds.

The rendering the LED module is split into 2 parts:

* a *frame* is one complete rendering of the LED display (4 digits),
* a *field* is a partial rendering of a single frame (a single digit).

A frame rate of about 60Hz will be sufficient to prevent obvious flickering of
the LED. For a 4-digit LED, that requires rendering 240 fields per second. The
`Hc595Module::renderFieldNow()` is meant to be used inside an interrupt service
routine (ISR) which is configured to execute exactly at the requested frequency.
It will immediately render the given field (a single digit) to the 74HC595 shift
registers. When it is called a second time, it will render the next digit. The
`Hc595Module::renderFieldWhenReady()` is designed to give an effective rendering
rate of 240 Hz using a polling method. It should be called as fast as possible
in the global `loop()` function. It keeps an internal timing variable that
remembers the last time that it was called. When the correct amount of time has
passed, it then calls `renderFieldNow()`, and resets the timing variable.

<a name="LedDisplay"></a>
### LedDisplay

The `LedDisplay` object is a thin-wrapper around an `LedModule` object which
provides a unified API to write bit patterns to the LED module at specific
positions. The `LedDisplay` also provides an API to set and clear the decimal
point on the LED module if available, and it provides the ability to control the
brightness of the LED module.

The public methods of class looks like this (not all public methods shown):

```C++
class LedDisplay {
  public:
    explicit LedDisplay(LedModule& ledModule);

    uint8_t getNumDigits() const;
    void writePatternAt(uint8_t pos, uint8_t pattern);
    void writePatternsAt(uint8_t pos, const uint8_t patterns[], uint8_t len);
    void writePatternsAt_P(uint8_t pos, const uint8_t patterns[], uint8_t len);
    void writeDecimalPointAt(uint8_t pos, bool state = true);

    void clear();
    void clearToEnd(uint8_t pos);

    void setBrightness(uint8_t brightness);
};
```

The decimal point is stored as bit 7 (the most significant bit) of the `uint8_t`
byte for a given digit. This bit is cleared by the other `writePatternAt()` or
`writePatternsAt()` functions. So the `writeDecimalPointAt()` should be called
**after** the other write methods are called.

The brightness value of an LED module is determine by the underlying controller
chip, and the range of values are different for each chip:

* The TM1637 chip supports 8 levels from 0 to 7, with 0 turning off the
  display and 7 being the brightest.
* The MAX7219 chip supports 16 levels from 0 to 15, with 0 being the dimmest
  level (which does not turn off the display), and 15 being the brightest.
* The brightness of 74HC595 module is controlled entirely by the
  microcontroller using pulse width modulation (PWM). The range of values could
  theoretically be from 0 to 255, but in practice, it is limited by the speed
  of the SPI protocol to the 74HC595 chip. A brightness range of 0-7 or 0-15
  seems practical for most configurations.

After a specific, hardware-dependent instance of `LedModule` is created, the
`LedDisplay` is created by wrapping around the `LedModule`:

```C++
LedDisplay display(ledModule);
```

Various Writer classes build upon the `LedDisplay` class to provide additional
ways of printing numbers and letters to the LED module.

<a name="NumberWriter"></a>
### NumberWriter

The `NumberWriter` can print integers to the `LedDisplay` using decimal (0-9) or
hexadecimal (0-9A-F) formats. On platforms that support it (ATmega and ESP8266),
the bit mapping table is stored in flash memory to conserve static memory.

The public methods of this class looks something like this:

```C++
class NumberWriter {
  public:
    typedef uint8_t hexchar_t;
    static const hexchar_t kCharSpace = 0x10;
    static const hexchar_t kCharMinus = 0x11;

    explicit NumberWriter(LedDisplay& ledDisplay);

    LedDisplay& display();

    void writeHexCharAt(uint8_t pos, hexchar_t c);
    void writeHexCharsAt(uint8_t pos, hexchar_t [], uint8_t len);

    void writeHexByteAt(uint8_t pos, uint8_t b);
    void writeHexWordAt(uint8_t pos, uint16_t w);

    void writeUnsignedDecimalAt(uint8_t pos, uint16_t num, int8_t boxSize = 0);
    void writeSignedDecimalAt(uint8_t pos, int16_t num, int8_t boxSize = 0);
    void writeUnsignedDecimal2At(uint8_t pos, uint8_t num);

    void clearToEnd(uint8_t pos);
};
```

The `hexchar_t` type semantically represents the character set supported by this
class. It is implemented as an alias for `uint8_t`, which unfortunately means
that the C++ compiler will not warn about mixing this type with another
`uint8_t`. The range of this character set is from `[0,15]` plus 2 additional
symbols, so `[0,17]`:

* `NumberWriter::kCharSpace`
* `NumberWriter::kCharMinus`

A `NumberWriter` consumes about 150 bytes of flash memory on an AVR.

<a name="ClockWriter"></a>
### ClockWriter

There are special, 4 digit,  seven segment LED displays which replace the
decimal point with the colon symbol ":" between the 2 digits on either side so
that it can display a time in the format "hh:mm".

The public methods of this class look like this:

```C++
class ClockWriter {
  public:
    using hexchar_t = NumberWriter::hexchar_t;
    static const hexchar_t kCharSpace = NumberWriter::kCharSpace;
    static const hexchar_t kCharMinus = NumberWriter::kCharMinus;
    static const uint8_t kPatternA = 0b01110111;
    static const uint8_t kPatternP = 0b01110011;

    explicit ClockWriter(LedDisplay& ledDisplay, uint8_t colonDigit = 1);

    LedDisplay& display() const;
    void writeCharAt(uint8_t pos, hexchar_t c);
    void writeChar2At(uint8_t pos, hexchar_t c0, hexchar_t c1);

    void writeBcd2At(uint8_t pos, uint8_t bcd);
    void writeDec2At(uint8_t pos, uint8_t d);
    void writeDec4At(uint8_t pos, uint16_t dd);

    void writeHourMinute(uint8_t hh, uint8_t mm);
    void writeColon(bool state = true);
};
```

A `ClockWriter` consumes about 250 bytes of flash memory on an AVR, which
includes an instance of a `NumberWriter`.

<a name="TemperatureWriter"></a>
### TemperatureWriter

This class supports writing out temperatures in degrees Celcius or Fahrenheit.
The public methods of this class looks something like this:

```C++
class TemperatureWriter {
  public:
    static const uint8_t kPatternDegree = 0b01100011;
    static const uint8_t kPatternC = 0b00111001;
    static const uint8_t kPatternF = 0b01110001;

    explicit TemperatureWriter(LedDisplay& ledDisplay);

    LedDisplay& display();

    uint8_t writeTempAt(uint8_t pos, int16_t temp, boxSize = 0);
    uint8_t writeTempDegAt(uint8_t pos, int16_t temp, boxSize = 0);
    uint8_t writeTempDegCAt(uint8_t pos, int16_t temp, boxSize = 0);
    uint8_t writeTempDegFAt(uint8_t pos, int16_t temp, boxSize = 0);
};
```

A `TemperatureWriter` consumes about 270 bytes of flash memory on an AVR, which
includes an instance of a `NumberWriter`.

<a name="CharWriter"></a>
### CharWriter

It is possible to represent many of the ASCII characters in the range `[0,127]`
on a seven-segment LED display, although some of the characters will necessarily
be crude given the limited number of segments. The `CharWriter` contains a
[mapping of ASCII](https://github.com/dmadison/LED-Segment-ASCII) characters
to seven-segment bit patterns. On platforms that support it (ATmega and
ESP8266), the bit pattern array is stored in flash memory to conserve static
memory.

The public methods of this class looks like thid:

```C++
class CharWriter {
  public:
    static const uint8_t kCharPatterns[];
    static const uint8_t kNumChars = 128;

    explicit CharWriter(
        LedDisplay& ledDisplay,
        const uint8_t charPatterns[] = kCharPatterns,
        uint8_t numChars = kNumChars
    );

    LedDisplay& display();

    void writeCharAt(uint8_t pos, char c);

    uint8_t getNumChars() const;
    uint8_t getPattern(char c) const;
};
```

A `CharWriter` consumes about 250 bytes of flash memory on an AVR.

<a name="StringWriter"></a>
### StringWriter

A `StringWriter` is a class that builds on top of the `CharWriter`. It knows how
to write entirely strings into the LED display. The public methods look like:

```C++
class StringWriter {
  public:
    explicit StringWriter(CharWriter& charWriter);

    LedDisplay& display();

    uint8_t writeStringAt(uint8_t pos, const char* cs, uint8_t numChar = 255);

    uint8_t writeStringAt(uint8_t pos, const __FlashStringHelper* fs,
            uint8_t numChar = 255);

    void clearToEnd(uint8_t pos);
};
```

The implementation of `writeStringAt()` is straightforward except for the
handling of a decimal point. A seven segment LED digit contains a small LED for
the decimal point. Instead of taking up an entire digit for a single '.'
character, we can collapse the '.' character into the decimal point indicator of
the previous character on the left.

The optional `numChar` parameter limits the number of characters in the string
to write. The default value is 255 which is expected to be larger than the
largest LED module that will be used with AceSegment, so the default value will
print the entire string.

The actual number of LED digits written is returned by `writeStringAt()`. For
example, writing `"1.2"` returns 2 because the decimal point was merged into the
previous digit and only 2 digits are written.

The `clearToEnd()` method clears the LED display from the given `pos` to the end
of the display.

The following sequence of calls will write the given string and clear all digits
after the end of the string:

```C++
StringWriter stringWriter(ledDisplay);

uint8_t written = stringWriter.writeStringAt(0, s);
stringWriter.clearToEnd(written);
```

<a name="StringScroller"></a>
### StringScroller

A `StringScroller` is a class that builds on top of the `CharWriter`. It knows
how to write entirely strings into the LED display. The public methods look
like:

```C++
class StringScroller {
  public:
    explicit StringScroller(LedDisplay& ledDisplay);

    LedDisplay& display() const;

    void initScrollLeft(const char* s);
    bool scrollLeft();

    void initScrollRight(const char* s);
    bool scrollRight();
};
```

To scroll a string to the left, first initialize the string, then call
`scrollLeft()` to shift left. Similarly to the right.

<a name="AdvancedUsage"></a>
## Advanced Usage

<a name="DigitalWriteFast"></a>
### DigitalWriteFast on AVR Processors

On the AVR processors (e.g. Arduino Nano, SparkFun Pro Micro), the default
`digitalWrite()`, `digitalRead()` and `pinMode()` functions can be significantly
improved (up to 50X performance, and a lot less flash memory) if the pin number
and output value are known at compile-time. There are at least 2 libraries that
provide the `digitalWriteFast()` variants:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast, or
* https://github.com/NicksonYap/digitalWriteFast

I have written versions of some lower-level classes to take advantage of
`digitalWriteFast()`:

* `scanning/LedMatrixDirectFast4.h`
    * Variant of `LedMatrixDirect` using `digitalWriteFast()`
* `hw/SoftSpiFastInterface.h`
    * Variant of `SoftSpiInterface.h` using  `digitalWriteFast()` for the
      `MOSI`, `SCK` and `LATCH` pins
* `hw/HardSpiFastInterface.h`
    * Variant of `HardSpiInterface.h` using  `digitalWriteFast()` to toggle
      the `LATCH` pin, while the hardware SPI code controls the `MOSI` and `SCK`
      pins
* `hw/SoftWireFastInterface.h`
    * Variant of `SoftWireInterface.h` using `digitalWriteFast()`

Since these header files require an external `digitalWriteFast` library to be
installed, and they are only valid for AVR processors, these header files are
*not* included in the master `<AceSegment.h>` file. If you want to use them, you
need to include these headers manually, like this:

```C++
#include <AceSegment.h> // do this first

#if defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h> // from 3rd party library
  #include <ace_segment/hw/SoftSpiFastInterface.h>
  #include <ace_segment/hw/HardSpiFastInterface.h>
  #include <ace_segment/hw/SoftWireFastInterface.h>
  #include <ace_segment/direct/DirectFast4Module.h>
#endif
```

<a name="ScanningModule"></a>
### Custom Configuration of ScanningModule

The 3 convenience classes (`DirectModule`, `HybridModule`, and `Hc595Module`)
are subclasses of the `ScanningModule` parent class. If you want to know how the
`ScanningModule` is implemented, I have some notes in
[docs/scanning_module.md](docs/scanning_module.md).

<a name="ResourceConsumption"></a>
## Resource Consumption

### Static Memory

Here are the sizes of the various classes on the 8-bit AVR microcontrollers
(Arduino Uno, Nano, etc):

```
sizeof(SoftWireInterface): 4
sizeof(SoftWireFastInterface<4, 5, 100>): 1
sizeof(SoftSpiInterface): 3
sizeof(SoftSpiFastInterface<11, 12, 13>): 1
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface<11, 12, 13>): 1
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 8
sizeof(LedMatrixDualHc595<HardSpiInterface>): 5
sizeof(LedModule): 3
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(DirectModule<4>): 31
sizeof(DirectFast4Module<...>): 25
sizeof(HybridModule<SoftSpiInterface, 4>): 30
sizeof(Hc595Module<SoftSpiInterface, 4>): 27
sizeof(Tm1637Module<SoftWireInterface, 4>): 14
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(LedDisplay): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 2
sizeof(StringWriter): 2
sizeof(StringScroller): 7
```

On 32-bit processors, these numbers look like this:

```
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 12
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 4>): 44
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12
```

### Flash Memory

For the most part, the user pays only for the feature that is being used. For
example, if the `CharWriter` (which consumes 312 bytes of flash) is not used, it
is not loaded into the program.

The full details are given in
[examples/MemoryBenchmark](examples/MemoryBenchmark). Here are the flash and
static memory consumptions for various configurations on an Arduino Nano
(ATmega328):

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   1486/   64 |  1030/   53 |
| DirectFast4Module               |   1250/   94 |   794/   83 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |   1508/   58 |  1052/   47 |
| Hybrid(SoftSpiFast)             |   1400/   56 |   944/   45 |
| Hybrid(HardSpi)                 |   1570/   59 |  1114/   48 |
| Hybrid(HardSpiFast)             |   1498/   57 |  1042/   46 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |   1422/   51 |   966/   40 |
| Hc595(SoftSpiFast)              |   1012/   49 |   556/   38 |
| Hc595(HardSpi)                  |   1496/   52 |  1040/   41 |
| Hc595(HardSpiFast)              |   1404/   50 |   948/   39 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftWire)                |   1582/   39 |  1126/   28 |
| Tm1637(SoftWireFast)            |    924/   36 |   468/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |   1218/   44 |   762/   33 |
| Max7219(SoftSpiFast)            |    778/   42 |   322/   31 |
| Max7219(HardSpi)                |   1298/   45 |   842/   34 |
| Max7219(HardSpiFast)            |   1072/   43 |   616/   32 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |    578/   24 |   122/   13 |
| NumberWriter+Stub               |    682/   28 |   226/   17 |
| ClockWriter+Stub                |    766/   29 |   310/   18 |
| TemperatureWriter+Stub          |    764/   28 |   308/   17 |
| CharWriter+Stub                 |    758/   28 |   302/   17 |
| StringWriter+Stub               |    974/   34 |   518/   23 |
+--------------------------------------------------------------+
```

And here are the memory consumption numbers for an ESP8266:

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 256700/26784 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    | 257772/27260 |  1072/  476 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 | 257860/27244 |  1160/  460 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 | 258964/27252 |  2264/  468 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  | 257728/27248 |  1028/  464 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  | 258928/27256 |  2228/  472 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftWire)                | 257920/27224 |  1220/  440 |
| Tm1637(SoftWireFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                | 257640/27224 |   940/  440 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                | 258856/27232 |  2156/  448 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           | 256876/27200 |   176/  416 |
| NumberWriter+Stub               | 257372/27200 |   672/  416 |
| ClockWriter+Stub                | 257196/27208 |   496/  424 |
| TemperatureWriter+Stub          | 257484/27200 |   784/  416 |
| CharWriter+Stub                 | 257084/27200 |   384/  416 |
| StringWriter+Stub               | 257316/27200 |   616/  416 |
+--------------------------------------------------------------+
```

### CPU Cycles

The CPU benchmark numbers can be seen in
[examples/AutoBenchmark](examples/AutoBenchmark).

Here are the CPU numbers for an AVR processor:

```
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct                                 |    68/   74/   88 |     240 |
| Direct(subfields)                      |     4/   12/   84 |    3840 |
| DirectFast4                            |    24/   28/   36 |     240 |
| DirectFast4(subfields)                 |     4/    8/   40 |    3840 |
|----------------------------------------+-------------------+---------|
| Hybrid(SoftSpi)                        |   156/  159/  180 |     240 |
| Hybrid(SoftSpi,subfields)              |     4/   22/  180 |    3840 |
| Hybrid(SoftSpiFast)                    |    28/   30/   44 |     240 |
| Hybrid(SoftSpiFast,subfields)          |     4/    8/   40 |    3840 |
| Hybrid(HardSpi)                        |    36/   39/   52 |     240 |
| Hybrid(HardSpi,subfields)              |     4/    9/   52 |    3840 |
| Hybrid(HardSpiFast)                    |    24/   27/   44 |     240 |
| Hybrid(HardSpiFast,subfields)          |     4/    7/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| Hc595(SoftSpi)                         |   264/  269/  304 |     240 |
| Hc595(SoftSpi,subfields)               |     4/   35/  304 |    3840 |
| Hc595(SoftSpiFast)                     |    20/   24/   36 |     240 |
| Hc595(SoftSpiFast,subfields)           |     4/    7/   32 |    3840 |
| Hc595(HardSpi)                         |    24/   27/   40 |     240 |
| Hc595(HardSpi,subfields)               |     4/    8/   36 |    3840 |
| Hc595(HardSpiFast)                     |    12/   14/   28 |     240 |
| Hc595(HardSpiFast,subfields)           |     4/    6/   24 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22308/22328/22576 |      20 |
| Tm1637(SoftWireFast)                   | 21060/21073/21212 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |  1320/ 1329/ 1448 |      20 |
| Max7219(SoftSpiFast)                   |   112/  120/  136 |      20 |
| Max7219(HardSpi)                       |   120/  129/  144 |      20 |
| Max7219(HardSpiFast)                   |    56/   63/   72 |      20 |
+----------------------------------------+-------------------+---------+
```

What is amazing is that if you use `digitalWriteFast()`, the software SPI is
just as fast as hardware SPI, **and** consumes 500 bytes of less flash memory
space.

Here are the CPU numbers for an ESP8266:

```
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct                                 |    12/   12/   25 |     240 |
| Direct(subfields)                      |     0/    2/   25 |    3840 |
|----------------------------------------+-------------------+---------|
| Hybrid(SoftSpi)                        |    29/   29/   37 |     240 |
| Hybrid(SoftSpi,subfields)              |     0/    4/   42 |    3840 |
| Hybrid(HardSpi)                        |    11/   11/   19 |     240 |
| Hybrid(HardSpi,subfields)              |     0/    2/   23 |    3840 |
|----------------------------------------+-------------------+---------|
| Hc595(SoftSpi)                         |    50/   50/   63 |     240 |
| Hc595(SoftSpi,subfields)               |     0/    7/   67 |    3840 |
| Hc595(HardSpi)                         |    12/   12/   19 |     240 |
| Hc595(HardSpi,subfields)               |     0/    2/   32 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 21496/21505/21552 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |   254/  256/  271 |      20 |
| Max7219(HardSpi)                       |    61/   61/   70 |      20 |
+----------------------------------------+-------------------+---------+
```

On the ESP8266, the hardware SPI is about 4X after, but it does consume 1200
bytes for flash space. But on the ESP8266 flash memory is usually not a concern,
so it seems to make sense to use hardware SPI on the ESP8266.

If we want to drive a 4 digit LED display at 60 frames per second, using a
subfield modulation of 16 subfields per field, we get a field rate of 3.84 kHz,
or 260 microseconds per field. We see from
[examples/AutoBenchmark](examples/AutoBenchmark) that all hardware platforms are
capable of providing a rendering time between fields of less than 260
microseconds.

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

The library has Tier 1 support on the following boards:

* Arduino Nano (16 MHz ATmega328P)
* SparkFun Pro Micro (16 MHz ATmega32U4)
* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (72 MHz ARM Cortex-M4)

Tier 2 support can be expected on the following boards, mostly because I don't
test these as often:

* ATtiny85 (8 MHz ATtiny85)
* Teensy LC (48 MHz ARM Cortex-M0+)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)

The following boards are **not** supported:

* Any platform using the ArduinoCore-API
  (https://github.com/arduino/ArduinoCore-api). For example, Nano Every,
  MKRZero, and Raspberry Pi Pico RP2040.

<a name="ToolChain"></a>
### Tool Chain

* [Arduino IDE 1.8.13](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.14.0](https://arduino.github.io/arduino-cli)
* [SpenceKonde ATTinyCore 1.5.2](https://github.com/SpenceKonde/ATTinyCore)
* [Arduino AVR Boards 1.8.3](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.1](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 1.9.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 2.7.4](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 1.0.6](https://github.com/espressif/arduino-esp32)
* [Teensydino 1.53](https://www.pjrc.com/teensy/td_download.html)

<a name="OperatingSystem"></a>
### Operating System

I use Ubuntu 20.04 for the vast majority of my development. I expect that the
library will work fine under MacOS and Windows, but I have not explicitly tested
them.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
## Feedback and Support

If you have any questions, comments and other support questions about how to
use this library, use the
[GitHub Discussions](https://github.com/bxparks/AceSegment/discussions)
for this project. If you have bug reports or feature requests, file a ticket in
[GitHub Issues](https://github.com/bxparks/AceSegment/issues). I'd love to hear
about how this software and its documentation can be improved. I can't promise
that I will incorporate everything, but I will give your ideas serious
consideration.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

Created by Brian T. Park (brian@xparks.net).
