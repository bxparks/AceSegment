# AceSegment
An adjustable, configurable, and extensible framework for rendering seven
segment LED displays on Arduino platforms

Version: Work-in-progress (2018-04-06)

## Summary

The AceSegment library provides a number of classes that can display
digits, characters and other patterns on an "seven segment" LED display.
It is called AceSegment because:
* many of its timing parameters are **Adjustable** at runtime
* many of its configurations (e.g. wiring modes) are
  **Configurable** at compile-time by choosing the appropriate classes
* the framework is **Extensible** by writing new versions of the `Driver` class

The framework splits the responsibility of displaying LED digits into two
main parts:
* The `Renderer` is the higher-level class that allows bit patterns of an LED
  digit to be associated with a number of predefined styles (e.g. blinking or
  pulsing). The parameters for these styles can be modified by the end-user.
* The `Driver` knows how to display the bit patterns to a specific
  physical wiring of an LED display. Different versions of the `Driver`
  are provided to cover some of the basic wiring configurations:
    * Resistors on segments
    * Resistors on digits
    * Resistors on digits, with pulse width modulation

Because of power and pin number limitations, the LED display will be multiplexed
to activate only a small part of the LED display. If the multiplexing is
performed quickly enough, the human eye will perceive that the entire LED
display is turned on.

The rendering of an array of bit patterns is split into 2 parts:
* a *frame* is one complete rendering of the LED display
* a *field* is a partial rendering of a single frame

A frame rate of about 60Hz will be sufficient to prevent obvious flickering of
the LED. Depending on the `Driver` subclass, we could reasonably have between 4
and 64 fields per frame (this is partially a user-selectable parameter), giving
us a fields per second rate of 240Hz to 3840Hz.

At the highest fields per second, a single field needs to be written in less
than 260 microseconds. The AceSegment library is able to meet this timing
requirement because the most complex driver option (`useModulatingDriver()`) is
able to render a single field with a maximum CPU time of 124 microseconds on a
16MHz ATmega328P microcontroller (Arduino UNO, Nano, Mini, etc).

When the `useModulatingDriver()` option is enabled, the following features
become enabled on a single digit, independently of the other digits:
* blinking slow
* blinking fast
* pulsing slow
* pulsing fast
* a user-selectable global brightness

The framework supports an LED display which is either directly connected to the
GPIO pins or through a serial-to-parallel chip like the 74HC595.

## Installation

The latest stable release will eventually be available in the Arduino IDE
Library Manager. Search for "AceSegment". Click install. It is not there
yet.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceSegment), checking out the
`develop` branch, then manually copying over the contents to the `./libraries`
directory used by the Arduino IDE. (The result is a directory named
`./libraries/AceSegment`.) The `master` branch contains the stable release.

## LED Wiring

AceSegment library supports the following wiring configurations:

* common cathode
* common anode
* resistors on segments
* resistors on digits

The driver classes assume that the pins are connected directly to the GPIO pins
of the microcontroller. In other words, for a 4 digit x 8 segment LED display,
you would need 12 GPIO pins. This is the cheapest and simplest option if
you have enough pins available because you need nothing else because the
current limiting resistors.

If the project is not able to allocate this many pins, then the usual solution
is to use a Serial to Parallel converter such as the 74HC595 chip. Currently,
the AceSegment library does not support this configuration but it is planned
to be added in the near future.

At first glance, there is not an obvious difference between "resistors on
segments" configuration and "resistors on digits". I recommend using resistors
on segments if at all possible. That's because the LEDs with the resistors are
the ones that can be turned on at the same time, and the ones without the
resistors are multiplexed to give the illusion of a fully lit display. With the
resistors on the segments, all the segments on one digit can be activated at the
same time, and we can use pulse width modulation on the digit line to control
the brightness of a single digit.

## Usage

### Include Header and Namespace

To prevent name clashes with other libraries that the calling code may use, all
classes are defined in the `ace_segment` namespace. To use the code without
prepending the `ace_segment::` prefix, use the `using` directive:

```
#include <AceSegment.h>
using namespace ace_segment;
```

### Classes

Here are the classes in the library which will be most useful to the
end-users, listed roughly from low-level to higher-level classes which often
depend on the lower-level classes:

* `Hardware`: A class that hold hardware dependent methods (such as
  `digitalWrite()`).
* `DimmingDigit`: A class that represents one digit of the 7-segment display
  and its brightness. An array of these will be created, one for each digit.
* `Driver`: A class that knows how to display bit patterns of a `DimmingDigit`
  to the seven segment leds. Different subclasses implement different types of
  wiring, but the user does not need to aware of the various subclasses. That
  complexity is managed by the `DriverBuilder` class.
* `StyledDigit`: A class that represents one digit which can have certain style
  attributes (e.g. blinking, or pulsing). A `StyledDigit` is converted into
  a `DimmingDigit`.
* `Renderer`: A class that knows how to convert a `StyledDigit` into the
  `DimmingDigit` that a `Driver` knows how to diplay. A `Renderer` also
  knows how to modulate the brightness of a `DimmingDigit` to achieve
  the style indicated by `StyledDigit`.
* `CharWriter`: A class that convert an ASCII character
  represented by a `char` (code 0-127) to a bit pattern used by the `Renderer`
  class. Not all ASCII characters can be rendered on a seven segment display
  legibly but the `CharWriter` tries its best.
* `StringWriter`: A class that can print strings of `char` to a `CharWriter`.
  It tries to be smart about collapsing decimal point `.` characters into
  the native decimal point on a seven segment LED display.

Not all `Driver`s and not all wiring configurations will support the brightness.
See the *Modulating Driver* for details on which configuration supports this
feature.

The [Doxygen docs for these classes](https://bxparks.github.io/AceSegment/html)
are published through GitHub Pages.

### Setting Up the Resources

With the exception of `Driver`, all of the classes in the previous section will
most likely be created statically in the global memory area. The `Driver` object
will be created by the `DriverBuilder` class on the heap and a pointer to the
object will be returned. The client code will not normally need to delete this
heap object so for all practical purposes, it can be treated as if it was
created statically.

The resource creation occurs in roughly 4 stages, with the objects in the
later stages depending on the objects created in the earlier stage:

1. Low level buffers and hardware interface.
1. The `Driver` object through the `DriverBuilder`.
1. The `Renderer` which knows how to send bit patterns to the `Driver`.
1. The various `XxxWriter` classes which translate higher level characters and
   strings into bit patterns.

A typical resource creation code looks like this:
```
const uint16_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_DIGITS = 4;

Hardware hardware;
DimmingDigit dimmingDigits[NUM_DIGITS];
StyledDigit styledDigits[NUM_DIGITS];

Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setXxx()
    .useXxx()
    ...
    .build();

Renderer renderer(&hardware, driver, styledDigits, NUM_DIGITS);
CharWriter charWriter(&renderer);
StringWriter stringWriter(&charWriter);
```

Then in the `setup()` method, the `driver` and the `renderer` need to be
configured, like this:
```
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  driver->configure();
  renderer
      .setFramesPerSecond(FRAMES_PER_SECOND)
      .setXxx()
      .configure();
  ...
```

### Configuring the Driver

The `Driver` is created indirectly through a helper class called the
`DriverBuilder`. The helper class exists to hide the enormous complexity of
configuring and adjusting the various subclasses and internal helper classes
which work together to make a `Driver` object operate correctly for a given
LED display configuration.

To use the `DriverBuilder`, create an instance of it, call the various
`setXxx()` or `useXxx()` methods to tell the object how the LED display is
wired, then finally call the `build()` method which returns an instance of a
`Driver` (more accurately, one of its implementation subclasses) which was
created on the heap. Normally, this heap object will never need to be
deleted, so you don't have to worry about memory management. All of these
methods on `DriverBuilder` return a reference to `*this`, so they can
be chained together in one statement. This makes it very easy to use
the `DriverBuilder` during the static initialization phase of the various
resources.

The following methods are available in `DriverBuilder`:

* `DriverBuilder& setHardware(Hardware* hardware);`
* `DriverBuilder& setNumDigits(uint8_t numDigits);`
* `DriverBuilder& setNumSegments(uint8_t numSegments);`
* `DriverBuilder& setCommonAnode();`
* `DriverBuilder& setCommonCathode();`
* `DriverBuilder& setResistorsOnDigits();`
* `DriverBuilder& setResistorsOnSegments();`
* `DriverBuilder& setDigitPins(const uint8_t* digitPins);`
* `DriverBuilder& setSegmentDirectPins(const uint8_t* segmentPins);`
* `DriverBuilder& setSegmentSerialPins(uint8_t latchPin, uint8_t dataPin,
   uint8_t clockPin);`
* `DriverBuilder& useSpi();`
* `DriverBuilder& setDimmingDigits(DimmingDigit* dimmingDigits);`
* `DriverBuilder& useModulatingDriver();`
* `DriverBuilder& setNumSubFields(uint8_t numSubFields);`

The best way to show how to use these methods is probably through
examples.

#### Pins Wired Directly, Resistors on Segments, Common Cathode

The wiring for this configuration looks like this:
```
MCU                     LED display
+-----+                  +------------------------+
|  D04|------ R ---------|a -------.              |
|  D05|------ R ---------|b -------|--------.     |
|  D06|------ R ---------|c        |        |     |
|  D07|------ R ---------|d      -----    -----   |
|  D08|------ R ---------|e       \ /      \ /    |
|  D09|------ R ---------|f      --v--    --v--   |
|  D10|------ R ---------|g        |        |     |
|  D11|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|  D12|------------------|D1 ------'--------'     |
|  D14|------------------|D2                      |
|  D15|------------------|D3                      |
|  D16|------------------|D4                      |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();
```

#### Pins Wired Directly, Resistors on Segments, Common Anode

The wiring for this configuration looks like this:
```
MCU                    LED display
+-----+                  +------------------------+
|  D04|------ R ---------|a -------.              |
|  D05|------ R ---------|b -------|--------.     |
|  D06|------ R ---------|c        |        |     |
|  D07|------ R ---------|d      --^--    --^--   |
|  D08|------ R ---------|e       / \      / \    |
|  D09|------ R ---------|f      -----    -----   |
|  D10|------ R ---------|g        |        |     |
|  D11|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|  D12|------------------|D1 ------'--------'     |
|  D14|------------------|D2                      |
|  D15|------------------|D3                      |
|  D16|------------------|D4                      |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonAnode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();
```

#### Pins Wired Directly, Resistors on Digits, Common Cathode

The wiring for this configuration looks like this:
```
MCU                     LED display
+-----+                  +------------------------+
|  D04|------------------|a -------.              |
|  D05|------------------|b -------|--------.     |
|  D06|------------------|c        |        |     |
|  D07|------------------|d      -----    -----   |
|  D08|------------------|e       \ /      \ /    |
|  D09|------------------|f      --v--    --v--   |
|  D10|------------------|g        |        |     |
|  D11|------------------|h        |        |     |
|     |                  |         |        |     |
|  D12|------ R---------| 1 ------'--------'      |
|  D14|------ R---------| 2                       |
|  D15|------ R---------| 3                       |
|  D16|------ R---------| 4                       |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonAnode()
    .setResistorsOnDigits()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();
```

#### Pins Wired Directly, Resistors on Digits, Common Anode

The wiring for this configuration looks like this:
```
MCU                     LED display
+-----+                  +------------------------+
|  D04|------------------|a -------.              |
|  D05|------------------|b -------|--------.     |
|  D06|------------------|c        |        |     |
|  D07|------------------|d      --^--    --^--   |
|  D08|------------------|e       / \      / \    |
|  D09|------------------|f      -----    -----   |
|  D10|------------------|g        |        |     |
|  D11|------------------|h        |        |     |
|     |                  |         |        |     |
|  D12|------- R --------|D1 ------'--------'     |
|  D14|------- R --------|D2                      |
|  D15|------- R --------|D3                      |
|  D16|------- R --------|D4                      |
+-----+                  +------------------------ +
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonAnode()
    .setResistorsOnDigits()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();
```

#### Pins Wired Directly, Resistors on Segments, Common Cathode, PWM

If the resistors are on the segments, then we have to option of
using pulse width modulation (PWM) to get features like brightness
and pulsing of digits. The wiring for this configuration looks like before:

```
MCU                     LED display
+-----+                  +------------------------+
|  D04|------ R ---------|a -------.              |
|  D05|------ R ---------|b -------|--------.     |
|  D06|------ R ---------|c        |        |     |
|  D07|------ R ---------|d      -----    -----   |
|  D08|------ R ---------|e       \ /      \ /    |
|  D09|------ R ---------|f      --v--    --v--   |
|  D10|------ R ---------|g        |        |     |
|  D11|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|  D12|------------------|D1 ------'--------'     |
|  D14|------------------|D2                      |
|  D15|------------------|D3                      |
|  D16|------------------|D4                      |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is similar to before but we add
a `useModulatingDriver()` option along with the number of
subfields to use with the `setNumSubFields()` method:
```
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};

Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .useModulatingDriver()
    .setNumSubFields(NUM_SUBFIELDS)
    .build();
```

#### Segment Pins Wired through Serial-to-Parallel Converter

There is experimental support for using an 74HC595 serial-to-parallel converter
chip. The caveat is that this chip can supply only 6 mA per pin (as opposed to
40 mA for an Arduino Nano for example). But it may be enough in some
applications.

We assume here that the registors are on the segment pins, and that we are using
a common cathode LED display.

```
MCU       74HC595             LED display
+-----+   +---------+         +------------------------+
|     |   |       Q0|--- R ---|a -------.              |
|     |   |       Q1|--- R ---|b -------|--------.     |
|  D10|---|ST_CP  Q2|--- R ---|c        |        |     |
|  D11|---|DS     Q3|--- R ---|d      -----    -----   |
|  D13|---|SH_CP  Q4|--- R ---|e       \ /      \ /    |
|     |   |       Q5|--- R ---|f      --v--    --v--   |
|     |   |       Q6|--- R ---|g        |        |     |
|     |   |       Q7|--- R ---|h        |        |     |
|     |   +---------+         |         |        |     |
|  D04|-----------------------|D1 ------'--------'     |
|  D05|-----------------------|D2                      |
|  D06|-----------------------|D3                      |
|  D07|-----------------------|D4                      |
+-----+                       +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t latchPin = 10; // ST_CP on 74HC595
const uint8_t dataPin = 11; // DS on 74HC595
const uint8_t clockPin = 13; // SH_CP on 74HC595

Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentSerialPins(latchPin, dataPin, clockPin)
    .setDimmingDigits(dimmingDigits)
    .build();
```

#### Segment Pins Wired through Serial-to-Parallel Converter using Hardware SPI

With the same configuration as above, we can use the hardware
[SPI](https://www.arduino.cc/en/Reference/SPI) instead of the
software implementation given by
[shiftOut()](https://www.arduino.cc/reference/en/language/functions/advanced-io/shiftout/).


```
MCU          74HC595             LED display
+--------+   +---------+         +------------------------+
|        |   |       Q0|--- R ---|a -------.              |
|        |   |       Q1|--- R ---|b -------|--------.     |
|  SS/D10|---|ST_CP  Q2|--- R ---|c        |        |     |
|MOSI/D11|---|DS     Q3|--- R ---|d      -----    -----   |
| SCK/D13|---|SH_CP  Q4|--- R ---|e       \ /      \ /    |
|        |   |       Q5|--- R ---|f      --v--    --v--   |
|        |   |       Q6|--- R ---|g        |        |     |
|        |   |       Q7|--- R ---|h        |        |     |
|        |   +---------+         |         |        |     |
|     D04|-----------------------|D1 ------'--------'     |
|     D05|-----------------------|D2                      |
|     D06|-----------------------|D3                      |
|     D07|-----------------------|D4                      |
+--------+                       +------------------------+
```

The `DriverBuilder` configuration is the same as before, but we
just add the `useSpi()` configuration option:
```
const uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t latchPin = 10; // ST_CP on 74HC595
const uint8_t dataPin = 11; // DS on 74HC595
const uint8_t clockPin = 13; // SH_CP on 74HC595

Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentSerialPins(latchPin, dataPin, clockPin)
    .useSpi()
    .setDimmingDigits(dimmingDigits)
    .build();
```

#### Modulating Driver

As indicated above, some driver options will support brightness control using
pulse width modulation. There are some restrictions. First, the resistors must
be on the segments. Second, the driver must be fast enough to do pulse width
modulation. Currently, this means that the following `DriverBuilder` options
(see below) must be used:
* `useSpi()` - hardware API
* `setSegmentDirectPins()` - direct wiring to the segment pins

The following options:
* `useSerial()` - software SPI

is too slow, and `useModulatingDriver()` will not work for this option.


### Configuring the Renderer

The following parameters are configuration in `Renderer`:
* `void setFramesPerSecond(uint8_t framesPerSecond);` (required)
* `void setBrightness(uint8_t brightness);`
* `void setBlinkFastDuration(uint16_t durationMillis);`
* `void setBlinkSlowDuration(uint16_t durationMillis);`
* `void setPulseFastDuration(uint16_t durationMillis);`
* `void setPulseSlowDuration(uint16_t durationMillis);`
* `void setStatsResetInterval(uint16_t framesPerStatsReset);`

Only the `setFramesPerSecond()` method is required. The others have reasonable
defaults.

And example of configuring the `Renderer` is:
```
renderer.setFramesPerSecond(FRAMES_PER_SECOND);
renderer.configure();
```

### Writing Digit Bit Patterns

The `Renderer` contains a number of methods to write the bit patterns of
the seven segment display:
* `void writePatternAt(uint8_t digit, uint8_t pattern, uint8_t style)`
* `void writePatternAt(uint8_t digit, uint8_t pattern)`
* `void writeStyleAt(uint8_t digit, uint8_t style)`
* `void writeDecimalPointAt(uint8_t digit, bool state = true)`

The `digit` is the index into the `StyledDigit` array, from `0` to
`NUM_DIGITS-1`. The `pattern` is an 8-bit integer which maps the the
seven-segment digit bit to the bit using following the usual convention for
a seven segment LED ('a' is the least significant bit 0, decimal point 'dp'
is the most seignificant bit 7):
```
7-segment map:
      aaa       000
     f   b     5   1
     f   b     5   1
      ggg       666
     e   c     4   2
     e   c     4   2
      ddd  dp   333  77

Segment: dp g f e d c b a
   Bits: 7  6 5 4 3 2 1 0
```
(Sometimes, the decimal point `dp` is labeled as an `h`).

The `style` is a constant given by the constants in the `StyledDigit` class:
```
static const StyleType kStyleNormal = 0;
static const StyleType kStyleBlinkSlow = 1;
static const StyleType kStyleBlinkFast = 2;
static const StyleType kStylePulseSlow = 3;
static const StyleType kStylePulseFast = 4;
```

The `writeDecimalPointAt()` is a special method that sets the bit corresponding
to the decimal point ('h', bit 7), no matter what previous pattern was there in
initially. The `state` variable controls whether the decimal point should
be turned on (default) or off (false).

As indicated above, each digit of the LED display can be assigned a
specific style:
* `kStyleNormal`: plain
* `kStyleBlinkSlow`: blinks a slow rate of 800 milliseconds per blink which can
  be changed by `Renderer::setBlinkSlowDuration()`
* `kStyleBlinkFast`: blinks a slow rate of 400 milliseconds per blink which can
  be changed by `Renderer::setBlinkFastDuration()`
* `kStylePulseSlow`: pulses (dims and brightens) at a slow rate of 3000
  milliseconds per cycle which can be changed by
  `Renderer::setPulseSlowDuration()`
* `kStylePulseFast`: pulses (dims and brightens) at a fast rate of 1000
  milliseconds per cycle which can be changed by
  `Renderer::setPulseFastDuration()`

The `useModulatingDriver()` option is required for the two pulsing modes,
because the driver needs to be able to support intermediate brightness of the
LED digits, which can currently be achieved using pulse width modulation.


### Global Brightness

If the `Driver` supports it, we can control the global brightness of the
entire LED display using:

```
renderer.setBrightness(value);
```

Note that the `value` is a fraction (0.0 - 1.0) represented in units of
1/256. In other words, 3 means (3/256) and 255 means (255/256).

### Frames and Fields

To understand how to use the `Renderer`, we first need to explain a couple of
terms that we
[borrowed from video processing](https://en.wikipedia.org/wiki/Field_(video)):
* **Frame**: A frame is a complete rendering of all digits of the seven segment
  display. A frame is intended to be a single, conceptually static image of the
  LED display. Any changes in bit patterns or brightness of the digits happens
  through the rendering of multiple frames.
* **Field**: A field is a partial rendering of a frame. If the current limiting
  resistors are on the segments (recommended), then the `DigitDriver`
  multiplexes through the digits. Each rendering of the digit is a *field* and
  for a 4-digit display, there are 4 fields per frame.

  If the current limiting resistors are on the digits (not recommended unless
  absolutely necessary given the constraints), then the `SegmentDriver`
  multiplexes through the 8 segments (7 plus the decimal point). Each segment
  will light across multiple digits, and there are 8 fields per frame.

A *frame* rate of about 60Hz is recommended to eliminate obvious visual
flickering. If the LED display has 4 digits, and we use "resistors on segments"
configuration, then we need to have a *field* rate of 240Hz. We will see later
that if we want dimmable digits using PWM, then we need about 8-16 subfields
within a field, giving a total *field* rate of about 2000-4000Hz. That's abaout
250-500 microseconds per field, which is surprisingly doable using an 8-bit
processor like an Arduino UNO or Nano on an ATmega328 running at 16MHz.

The `Driver` and its subclasses do not know about *frames*, they only know about
*fields*. The `Renderer` on the other hand, cares only about frames and does not
know much of anything about fields. The only thing that the `Renderer` knows is
how many fields there are in a frame and this information comes from the
`Driver::getFieldsPerFrame()` method from the `Driver`.

### Use Modulating Driver Option

When the `useModulatingDriver()` option is configured in `DriverBuilder` with,
say, 16 subfields within a single field, then each digit is rendered 16 times
within a single field, but modulated using pulse width modulation to control the
width of that signal. The given digit will be "on" only a fraction of the full
interval of the single field rendering and will appear dimmer to the human eye.

Since PWM is used to control the brightness of the digit, the
`isBrightnessSupported()` method returns `true` only if the
`useModulatingDriver()` is selected, which notifies the `Renderer` that
brightness is supported. This allows the `Renderer` to avoid performing certain
time consuming calculatings related to pulsing, which saves CPU cycles.

### Rendering

With the distinction between *frames* and *fields* explained, we can now explain
how `Renderer::renderField()` works. The `Renderer` keeps an internal counter,
and if the call occurs at a frame boundary, the `Renderer` calculates the
`DimmingDigit` buffer in the `Driver` from the `StyledDigit` in the `Renderer`,
and applies any changes to the digit bit patterns necessary to support the
various digit styles (overall brightness, pulsing or blinking). Then the
`Renderer` passes along the call to the `Driver` which will draw the resulting
bit pattern on the LED display.

If the call to `renderField()` occurs in the middle of a frame (i.e. in a
field), then the `Renderer` simply passed along the call to the `Driver`, which
will update the bit patterns as rendered to it by the `Renderer` (supporting
blinking and pulsing). Any pulse width modulation to support a specific
brightness level happens at the `Driver` level, not at the `Renderer` level. The
`Renderer` does not care how the brightness is achieved, it leaves that decision
up to the `Driver`.

One more interesting property is that neither the `Renderer` or the `Driver`
is actually aware of the real clock (millis or micros). The only thing that
marks the passage of time for these objects is the *frame* counter and the
*field* counter. The AceSegment library leaves it up to the calling code to call
`renderField()` at exactly the right time.

There are 2 methods to achieve this:

* Polling
* Interrupts

### Rendering By Polling

For convenience, we provided one method, `Renderer::renderFieldWhenReady()` that
actually knows the real clock, and can be polled repeated to generated the calls
to `renderField()` at the right time. This is the easiest way to see something
on the LED segments and will work at the early stages of a project. But any
non-trivial project will want to use the interrupt method (explained below).

The code looks like this:

```
void loop() {
  renderer.renderFieldWhenReady();
}
```

The problem with using this method is that it's difficult to get much else done
in the `loop()` method. We noted above that to get dimmable digits using PWM,
then we need a field rate of 2000-4000 Hz or 250-500 microseconds per frame. If
the `loop()` method executes anything else that affects the timing requirements,
then the user will notice this problem as flickering of the LED segments.

### Rendering Using Interrupts

This is the recommended way of drawing the bit patterns to the LED display.

The calling code sets up an interrupt service
routine which calls `Renderer::renderField()` at exactly the periodic
frequency needed to achieve the desired frames per second and fields
per second.

Unfortunately, timer interrupts are not part of the Arduino API (probably
because every microcontroller does interrupts in a slightly different way). For
example, an ATmega328 (e.g. Arduino UNO, Nano, Mini), using an 8-bit timer on
Timer 2 looks like this:
```
ISR(TIMER2_COMPA_vect) {
  renderer.renderField();
}

void setup() {
  ...
  // set up Timer 2
  uint8_t timerCompareValue =
      (long) F_CPU / 1024 / renderer.getFieldsPerSecond() - 1;
  noInterrupts();
  TCNT2  = 0;	// Initialize counter value to 0
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= bit(WGM21); // CTC
  TCCR2B |= bit(CS22) | bit(CS21) | bit(CS20); // prescale 1024
  TIMSK2 |= bit(OCIE2A); // interrupt on Compare A Match
  OCR2A =  timerCompareValue;
  interrupts();
  ...
}

void loop() {
 ...do other stuff here...
}
```

### CharWriter

While it is exciting to be able to write any bit patterns to the LED display,
it is often easier to write ASCII characters represented by the `char` type. The
`CharWriter` contains a mapping of all ASCII characters (0-127) to seven-segment
bit patterns. On platforms that support it (ATmega and ESP8266), the bit mapping
table is stored in flash memory to conserve static memory.

The class supports the following methods:
* `void writeCharAt(uint8_t digit, char c)`
* `void writeCharAt(uint8_t digit, char c, StyledDigit::StyleType style)`
* `void writeStyleAt(uint8_t digit, StyledDigit::StyleType style)`
* `void writeDecimalPointAt(uint8_t digit, bool state = true)`

### StringWriter

A `StringWriter` is a class that builds on top of the `CharWriter`. It knows how
to write entirely strings into the LED display. It provides the following
method:

* `void writeStringAt(uint8_t digit, const char* s, bool padRight = false)`

The implementation of this method is straightforward except for the handling of
a decimal point. A seven segment LED digit contains a small LED for the decimal
point. Instead of taking up an entire digit for a single '.' character, we can
collapse the '.' character into the decimal point indicator of the previous
character on the left.

The `padRight` flag tells the method to pad spaces to the right if we run out of
characters before getting to the end of the digits on the LED display.

Scrolling can be achieved by writing success string fragments into digit 0, with
a scrolling timing interval:
```
void scrollString(const char* s) {
  static uint8_t i = 0;

  if (i >= strlen(s)) i = 0;
  stringWriter.writeStringAt(0, &s[i], true /* padRight */);
  i++;
}
```

(TODO: Maybe move this code fragment into the StringWriter class. I'm not sure
that we can push this down to the Renderer class because the Renderer not know
how to translate a `char` into the bit patterns of `StyledDigit`. We could have
the StringWriter present a complete array of translated `StyledDigit` to the
Renderer, but that seems like a waste of memory, since we don't need to
precalcuate the bit pattern translation of the entire string. We only need to
translate as many characters as will fit into the number of digits in the LED
display. Also, it turns out the precalcuted strings won't really work, because
the exact `StyledDigit` of the first digit depends on the scroll position. In
other words, a period '.' character will occupy an entire digit on the first LED
digit, but will be collapsed into the previous character at other positions.)

### NumberWriter

TBD

## Resource Consumption

Here are the sizes of the various classes on the 8-bit AVR microcontrollers
(Arduino Uno, Nano, etc):

* sizeof(Hardware): 2
* sizeof(LedMatrixDirect): 14
* sizeof(LedMatrixSerial): 15
* sizeof(LedMatrixSpi): 15
* sizeof(Driver): 7
* sizeof(SegmentDriver): 10
* sizeof(DigitDriver): 11
* sizeof(ModulatingDigitDriver): 14
* sizeof(Renderer): 62
* sizeof(CharWriter): 2
* sizeof(StringWriter): 2

**Program size:**

TBD

**CPU cycles:**

The `Renderer` has a number of statistics variables which track the minimum,
average, and maximum amount of time taken by a single call to `renderField()`
method, over the last 300 frames (default which is adjustable). Here
are the numbers for a 16MHz ATmega328P:

* DigitDriver w/ LedMatrixDirect
    * min: 16us; avg: 47us; max: 96us
* DigitDriver w/ LedMatrixSerial
    * min: 16us; avg: 82us; max: 196us
* ModulatingDigitDriver w/ LedMatrixDirect
    * min: 8us; avg: 12-20us; max: 192us
* ModulatingDigitDriver w/ LedMatrixDirect w/ calcPulseFractionForFrameUsingInverse():
    * min: 8us; avg: 12-20us; max: 124us
* ModulatingDigitDriver w/ LedMatrixSpi
    * min: 8us; avg: 16us; max: 92us
* SegmentDriver w/ LedMatrixDirect
    * min: 28us; avg: 36-59us; max: 80us

If we want to drive a 4 digit LED display at 60 frames per second, using a
subfield modulation of 16 subfields per field, we get a field rate of 3.84 kHz,
or 260 microseconds per field. The worst case elapsed time for
`Renderer::renderField()` using the `useModulatingDriver()` option was observed
to be 124 microseconds. That means that we would have slightly less than 136
microseconds of spare CPU cyles to do any other work besides rendering the LED
display.

## System Requirements

This library was developed and tested using:
* [Arduino IDE 1.8.5](https://www.arduino.cc/en/Main/Software)
* [Teensyduino 1.41](https://www.pjrc.com/teensy/td_download.html)

I used MacOS 10.13.3 and Ubuntu Linux 17.10 for most of my development.

The library has been verified to work on the following hardware:

* Arduino Nano clone (16 MHz ATmega328P)
* Arduino Pro Mini clone (16 MHz ATmega328P, 5V)
* Teensy LC (48 MHz ARM Cortex-M0+)

The unit tests require [AUnit](https://github.com/bxparks/AUnit)
to be installed.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## License

[MIT License](https://opensource.org/licenses/MIT)

## Authors

Created by Brian T. Park (brian@xparks.net).
