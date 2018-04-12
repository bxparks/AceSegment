# AceSegment
An adjustable, configurable, and extensible framework for rendering seven
segment LED displays on Arduino platforms

Version: 0.1.0 (2018-04-10)

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

A Python script in the `./tools` directory extends the framework to implement
C++ code generation to create subclasses of `Driver` that uses
[digitalWriteFast()](https://github.com/NicksonYap/digitalWriteFast) routines
which are 10-20X faster than the default `digitalWrite()` methods in Arduino.
The generated code makes `Driver::renderField()` consume 25% to 65% fewer CPU
cycles (i.e. 1.3 to 2.7 times faster).

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
you would need 12 GPIO pins. This is the cheapest and simplest option if you
have enough pins available because you need nothing else because the current
limiting resistors. If the project is not able to allocate this many pins, then
the usual solution is to use a Serial to Parallel converter such as the 74HC595
chip.

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
* `DriverBuilder`: A class that knows how to select and configure the
  appropriate subclass of `Driver`.
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
See the *Modulating Driver* section for details on which configuration supports
this feature.

The [Doxygen docs for these classes](https://bxparks.github.io/AceSegment/html)
are published through GitHub Pages.

### Setting Up the Resources

The instances of the various classes mentioned above are recommended to
be created in the `setup()` method in the heap using the `new` operator, instead
of creating them statically. There are 2 reasons:
1. The `DriverBuilder` does not know which subclass of `Driver` to build until
   it is given its build parameters, so it must create the `Driver` object
   on the heap. For consistency, it's easier to create all the other objects
   on the heap as well.
1. It's sometimes necessary to debug the construction of some of these objects,
   and the `Serial.print()` method does not work if used during static
   initialization. It must be called after the `Serial` object is initialized
   in the `setup()` method.

The client code will not normally need to delete these heap objects so for all
practical purposes, they can be treated as if they were created statically.

The resource creation occurs in roughly 5 stages, with the objects in the
later stages depending on the objects created in the earlier stage:

1. Low level buffers which are created statically.
1. The `Hardware` object which provides access to the various pins.
1. The `Driver` object created through the `DriverBuilder`.
1. The `Renderer` object created through the `RendererBuilder`.
1. The various `XxxWriter` classes which translate higher level characters and
   strings into bit patterns used by `Renderer`.

A typical resource creation code looks like this:
```
const uint16_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_DIGITS = 4;
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};

DimmingDigit dimmingDigits[NUM_DIGITS];
StyledDigit styledDigits[NUM_DIGITS];

// The chain of resources.
Hardware* hardware;
Driver* driver;
Renderer* renderer;
CharWriter* charWriter;
StringWriter* stringWriter;
...

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  // Create an instance of Hardware that will be shared.
  hardware = new Hardware();

  // Create and configure the Driver.
  driver = DriverBuilder(hardware)
      .setNumDigits(NUM_DIGITS)
      .setCommonCathode()
      .setResistorsOnSegments()
      .setDigitPins(digitPins)
      .setSegmentDirectPins(segmentPins)
      ...
      .build();
  driver->configure();

  // Create and configure the Renderer.
  renderer = RendererBuilder(hardware, driver, styledDigits, NUM_DIGITS)
      .setFramesPerSecond(FRAMES_PER_SECOND)
      ...
      .build();
  renderer->configure();

  // Create higher level Writers.
  charWriter = new CharWriter(renderer);
  stringWriter = new StringWriter(charWriter);

  ...

  Serial.println(F("setup(): end"));
}

```

#### Heap Objects versus Static Objects

Usually in embedded environments, resources (objects) are created statically
instead of the heap. That's usually because resources are created once and
never deleted for the life of the application. In the `Driver` and `Renderer`
objects, I chose to create them on the heap through the `DriverBuilder` and
`RendererBuilder` objects (respectively). For `DriverBuilder`, it was a
necessity since it creates different versions (subclasses) of `Driver` and an
internal class called `LedMatrix`, and it's not possible to statically create
those subclasses. Then for consistency, I chose to do the same for
`RendererBuilder` even though that was not strictly necessary.

In the example shown above, I chose to use the heap for all resources for
consistency of style. In a normal program, all of those heap objects are created
once and never deleted, so the static memory usage will be about the same as if
they were created statically.

If the user prefer, it *is* possible to create most of these objects statically,
like this:
```
const uint16_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_DIGITS = 4;
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};

DimmingDigit dimmingDigits[NUM_DIGITS];
StyledDigit styledDigits[NUM_DIGITS];

// The chain of resources.
Hardware hardware;
Driver* driver = DriverBuilder(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    ...
    .build();
Renderer* renderer = RendererBuilder(
        &hardware, driver, styledDigits, NUM_DIGITS)
    .setFramesPerSecond(FRAMES_PER_SECOND)
    ...
    .build();
CharWriter charWriter(renderer);
StringWriter stringWriter(&charWriter);
...

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  driver->configure();
  renderer->configure();
  ...

  Serial.println(F("setup(): end"));

```

The disadvantages are:
* there is mixing style, using objects by value and some objects by pointer
* debugging the `DriverBuilder::build()` and `RendererBuilder::build()` is
  more difficult because we cannot insert a `Serial.print()` statement in there,
  `Serial` cannot be used until it is configured in the `setup()` method

In unit tests, multiple test objects are created and deleted with various
configuration parameters, so creating these objects on the heap makes the test
code cleaner and more robust. Once I got used to creating them on the heap, then
it seemed easier sense to use the same style within an application. As I point
out above, these heap objects are created just once in an application, so
there's little difference in the overall static memory usage.

The biggest disadvantage of using the `DriverBuilder` helper object is not the
static memory consumption, but *flash* memory consumption. The `DriverBuilder`
contains the ability to dynamically create various subclasses of `Driver` and an
internal helper class called `LedMatrix`. Even though the wiring and other
configuration options are known at compile-time, the code for creating all the
other configuration options are loaded into flash and *not removed*, even after
the `DriverBuilder` instance is created, used, and destroyed.

Therefore, I've made it possible to bypass the `DriverBuilder` and
`RendererBuilder` and create *all* resources statically. You just need to follow
the logic of `DriverBuilder` and `RendererBuilder`, create the intermediary
`LedMatrix` object, and call the constructors of the various subclasses of
`Driver` and the constructor of `Renderer` manually. Send me an email if you
need help with this. (If there is sufficient demand, I will write up more
detailed instructions.)

However, I suspect that if flash memory consumption is an issue, then you will
probably also be concerned about CPU cycles, and there's an even better way to
reduce both. See the section __Code Generation to Use DigitalWriteFast__ section
below.

### Configuring the Driver

The `Driver` is created indirectly through a helper class called the
`DriverBuilder` which hides the complexity of configuring and adjusting the
various Driver options. To use the `DriverBuilder`, create a temporary instance
of it (on the stack), call the various `setXxx()` or `useXxx()` methods to tell
the object how the LED display is wired, then finally call the `build()` method
which returns an instance of a `Driver` (more accurately, one of its
implementation subclasses) which was created on the heap.

Normally, the `Driver` object will never need to be deleted, so you don't have
to worry about memory management. All of these methods on `DriverBuilder` return
a reference to `*this`, so they can be chained together in one statement.

The following methods are available in `DriverBuilder`:

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
* `DriverBuilder& setSegmentSpiPins(uint8_t latchPin, uint8_t dataPin,
   uint8_t clockPin);`
* `DriverBuilder& setDimmingDigits(DimmingDigit* dimmingDigits);`
* `DriverBuilder& useModulatingDriver();`
* `DriverBuilder& setNumSubFields(uint8_t numSubFields);`

The best way to show how to use these methods is probably through
examples.

In the following circuit diagrams, `R` represents the current limiting resistors
and `T` represents the driving transistor. The resistor protects each LED
sement, and the transistor prevents overloading the GPIO pin of the
microcontroller. On an Arduino Nano for example, each GPIO pin can handle 40mA
of current. But if all 8 segments are turned on at the same time, then 320mA of
current would flow into the digit pin, which exceeds the maximum current
significantly. If the value of `R` is high enough that each segment only drew
only 5mA of current, the transistor could be omitted since 8 x 5mA is 40mA,
which is within the limit of a single GPIO pin. The LED will probably be
significantly dim, but this configuration may be good enough for testing
purposes.

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
|  D12|------ T ---------|D1 ------'--------'     |
|  D14|------ T ---------|D2                      |
|  D15|------ T ---------|D3                      |
|  D16|------ T ---------|D4                      |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder(hardware)
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
|  D12|------ T ---------|D1 ------'--------'     |
|  D14|------ T ---------|D2                      |
|  D15|------ T ---------|D3                      |
|  D16|------ T ---------|D4                      |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder(hardware)
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
|  D04|------ T ---------|a -------.              |
|  D05|------ T ---------|b -------|--------.     |
|  D06|------ T ---------|c        |        |     |
|  D07|------ T ---------|d      -----    -----   |
|  D08|------ T ---------|e       \ /      \ /    |
|  D09|------ T ---------|f      --v--    --v--   |
|  D10|------ T ---------|g        |        |     |
|  D11|------ T ---------|h        |        |     |
|     |                  |         |        |     |
|  D12|------ R ---------|D1 ------'--------'     |
|  D14|------ R ---------|D2                      |
|  D15|------ R ---------|D3                      |
|  D16|------ R ---------|D4                      |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder(hardware)
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
|  D04|------- T --------|a -------.              |
|  D05|------- T --------|b -------|--------.     |
|  D06|------- T --------|c        |        |     |
|  D07|------- T --------|d      --^--    --^--   |
|  D08|------- T --------|e       / \      / \    |
|  D09|------- T --------|f      -----    -----   |
|  D10|------- T --------|g        |        |     |
|  D11|------- T --------|h        |        |     |
|     |                  |         |        |     |
|  D12|------- R --------|D1 ------'--------'     |
|  D14|------- R --------|D2                      |
|  D15|------- R --------|D3                      |
|  D16|------- R --------|D4                      |
+-----+                  +----------------------- +
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
Driver* driver = DriverBuilder(hardware)
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
|  D12|------ T ---------|D1 ------'--------'     |
|  D14|------ T ---------|D2                      |
|  D15|------ T ---------|D3                      |
|  D16|------ T ---------|D4                      |
+-----+                  +------------------------+
```

The `DriverBuilder` configuration is similar to before but we add
a `useModulatingDriver()` option along with the number of
subfields to use with the `setNumSubFields()` method:
```
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};

Driver* driver = DriverBuilder(hardware)
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
|  D04|------- T -------------|D1 ------'--------'     |
|  D05|------- T -------------|D2                      |
|  D06|------- T -------------|D3                      |
|  D07|------- T -------------|D4                      |
+-----+                       +------------------------+
```

The `DriverBuilder` configuration is:
```
const uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t latchPin = 10; // ST_CP on 74HC595
const uint8_t dataPin = 11; // DS on 74HC595
const uint8_t clockPin = 13; // SH_CP on 74HC595

Driver* driver = DriverBuilder(hardware)
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
|     D04|------- T -------------|D1 ------'--------'     |
|     D05|------- T -------------|D2                      |
|     D06|------- T -------------|D3                      |
|     D07|------- T -------------|D4                      |
+--------+                       +------------------------+
```

The `DriverBuilder` configuration is similar to before but we use the
`setSegmentSpiPins()` method instead:

```
const uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t latchPin = 10; // ST_CP on 74HC595
const uint8_t dataPin = 11; // DS on 74HC595
const uint8_t clockPin = 13; // SH_CP on 74HC595

Driver* driver = DriverBuilder(hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentSpiPins(latchPin, dataPin, clockPin)
    .setDimmingDigits(dimmingDigits)
    .build();

driver->configure();
```

#### Modulating Driver

As indicated above, some driver options will support brightness control using
pulse width modulation. There are some restrictions. First, the resistors must
be on the segments. Second, the driver must be fast enough to do pulse width
modulation. The modulating driver is activated using the `useModulatingDriver()`
option and it requires the `setNumSubFields()` to be specified:

```
Driver* driver = DriverBuilder()
    ...
    .useModulatingDriver()
    .setNumSubFields(NUM_SUBFIELDS)
    ...
```

#### Feature Matrix

Here is a table that summarizes the various combinations which are supported by
`DriverBuilder`. As you can see, PWM is only available if
"Resistors-on-Segments" are used, because the modulation happens on a per-digit
basis.

The "Resistors" options are selected by:
* `setResistorsOnSegments()`
* `setResistorsOnDigits()`

The "Wiring" options are selected by:
* `setSegmentDirectPins(segmentPins)`
* `setSegmentSerialPins(latch, data, clock)`
* `setSegmentSpiPins(latch, data, clock)`

The "PWM" options are selected by:
* `useModulatingDriver()`
* `setNumSubFields(NUM_SUBFIELDS)

```
Resistors | Wiring | PWM | Available? |
----------+--------+-----+------------|
Segments  | Direct | -   | y          |
Segments  | Direct | On  | y          |
Segments  | Serial | -   | y          |
Segments  | Serial | On  | y          |
Segments  | SPI    | -   | y          |
Segments  | SPI    | On  | y          |
Digits    | Direct | -   | y          |
Digits    | Direct | On  | -          |
Digits    | Serial | -   | y          |
Digits    | Serial | On  | -          |
Digits    | SPI    | -   | y          |
Digits    | SPI    | On  | -          |
----------+--------+-----+------------|
```

### Configuring the Renderer

The `Renderer` is dependent on the following resources, and these required
parameters are given in the constructor of `RendererBuilder`:
* `Hardware`
* `Driver`
* an array of `StyledDigit`

The following optional parameters can be given to `RendererBuilder` to override
the defaults. Each of these methods returns a reference to `*this` so they can
be chained (see below):
* `setFramesPerSecond(uint8_t framesPerSecond);` (default: 60)
* `setStatsResetInterval(uint16_t framesPerStatsReset);` (default: 120)
* `setBlinkSlowDuration(uint16_t durationMillis);` (default: 800)
* `setBlinkFastDuration(uint16_t durationMillis);` (default: 400)
* `setPulseSlowDuration(uint16_t durationMillis);` (default: 3000)
* `setPulseFastDuration(uint16_t durationMillis);` (default: 1000)

The `build()` method creates an instance of `Renderer` with the given
parameters. An example of configuring the `Renderer` is:
```
const uint8_t NUM_DIGITS = 4;
StyledDigit styledDigits[NUM_DIGITS];
const int FRAMES_PER_SECOND = 90;
const int BLINK_FAST_DURATION_MILLIS = 500;

void setup() {
  ...
  Hardware* hardware = ...;
  Driver* driver = ...;

  Renderer* renderer =
      RendererBuilder(hardware, driver, styledDigits, NUM_DIGITS)
      .setFramesPerSecond(FRAMES_PER_SECOND)
      .setBlinkFastDuration(BLINK_FAST_DURATION_MILLIS)
      .build();
  renderer->configure();
  ...
}
```

### Using the Renderer

#### Writing Digit Bit Patterns

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


#### Global Brightness

If the `Driver` supports it, we can control the global brightness of the
entire LED display using:

```
renderer->writeBrightness(value);
```

Note that the `value` is a fraction (0.0 - 1.0) represented in units of
1/256. In other words, 3 means (3/256) and 255 means (255/256).

The global brightness is enabled only if the `useModulatingDriver()` option was
configured in `DriverBuilder`. If the `setNumSubFields()` was set to 16, then
each digit is rendered 16 times within a single field, but modulated using pulse
width modulation to control the width of that signal. The given digit will be
"on" only a fraction of the full interval of the single field rendering and will
appear dimmer to the human eye.

#### Frames and Fields

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

#### Rendering By Polling

For convenience, we provided one method, `Renderer::renderFieldWhenReady()` that
actually knows the real clock, and can be polled repeated to generated the calls
to `renderField()` at the right time. This is the easiest way to see something
on the LED segments and will work at the early stages of a project. But any
non-trivial project will want to use the interrupt method (explained below).

The code looks like this:

```
void loop() {
  renderer->renderFieldWhenReady();
}
```

The problem with using this method is that it's difficult to get much else done
in the `loop()` method. We noted above that to get dimmable digits using PWM,
then we need a field rate of 2000-4000 Hz or 250-500 microseconds per frame. If
the `loop()` method executes anything else that affects the timing requirements,
then the user will notice this problem as flickering of the LED segments.

#### Rendering Using Interrupts

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
  renderer->renderField();
}

void setup() {
  ...
  // set up Timer 2
  uint8_t timerCompareValue =
      (long) F_CPU / 1024 / renderer->getFieldsPerSecond() - 1;
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
`CharWriter` contains a
[mapping of ASCII](https://github.com/dmadison/LED-Segment-ASCII)
characters (0-127) to seven-segment bit patterns. On platforms that support it
(ATmega and ESP8266), the bit mapping table is stored in flash memory to
conserve static memory.

The class supports the following methods:
* `void writeCharAt(uint8_t digit, char c)`
* `void writeCharAt(uint8_t digit, char c, StyledDigit::StyleType style)`
* `void writeStyleAt(uint8_t digit, StyledDigit::StyleType style)`
* `void writeDecimalPointAt(uint8_t digit, bool state = true)`

A `CharWriter` consumes significant amount of flash storage, about 1800 bytes.

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

### Code Generation to Use DigitalWriteFast

Looking at the CPU cycles in the __Resource Consumption__ section below, the
time taken for a single `Renderer::renderField()` can range from 80 to 204
microseconds. A significant contribution to the CPU cycles is the slow
implementation of
[digitalWrite() in Arduino](https://forum.arduino.cc/index.php?topic=46896.0).
A faster version called
[digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast)
is available on GitHub based on the Forum discussions.

I wrote a Python script called `./tools/fast_driver.py` which generates C++ code
for a subclass of `Driver` that uses the `digitalWriteFast()`. The
script is called like this:
```
$ ./tools/fast_driver.py --digit_pins 12 14 15 16 \
        --segment_direct_pins 4 5 6 7 8 9 10 11 \
        --class_name FastDirectDriver --output_files
```
which generates two files in the current directory:
```
FastDirectDriver.h
FastDirectDriver.cpp
```

These generated classes will replace the class generated by `DriverBuilder`:
```
Driver* driver = DriverBuilder(hardware)
    .setDimmingDigits(dimmingDigits)
    .setNumDigits(NUM_DIGITS)
    .useModulatingDriver()
    .setNumSubFields(NUM_SUBFIELDS)
    ...
    .build();
```
with just
```
Driver* driver = new FastDirectDriver(dimmingDigits, NUM_DIGITS, NUM_SUBFIELDS);
```

The generated code has no dependency to `Hardware`, it writes directly to the
`digitalWriteFast()` method (which is actually implemented as macros). The
resulting generated code in all configurations runs `renderField()` between
76-84 microseconds, which is 1.3 to 2.7 times faster than the `Driver` versions
created by `DriverBuilder`.

For "production" code that uses the AceSegment library, the code generation is
the recommended procedure. See the example code in `examples/AceSegmentDemo/'
for more details.

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

Here are the flash and static memory consumptions for various options.
Tested on `examples/AceSegmentDemo`:

```
Configuration    | flash/static | Incremental Size |
-----------------+--------------+------------------|
No AceSegment    | 2562/218     | 0/0              |
ModDigit/Direct  |              |                  |
 (no CharWriter) | 7468/419     | 4906/201         |
ModDigit/Direct  | 9266/432     | 6704/214         |
ModDigit/Serial  | 9266/424     | 6704/206         |
ModDigit/SPI     | 9266/424     | 6704/206         |
Segment/Direct   | 9266/427     | 6704/209         |
FastDirectDriver | 8570/416     | 6008/198         |
FastSerialDriver | 8410/376     | 5848/158         |
FastSpiDriver    | 8450/377     | 5888/159         |
-----------------+--------------+------------------|
```

**CPU cycles:**

The `Renderer` has a number of statistics variables which track the minimum,
average, and maximum amount of time taken by a single call to `renderField()`
method, over the last 300 frames (default which is adjustable). Here are the
numbers for a 16MHz ATmega328P, using a frame rate of 60Hz, with 16 subfields
per field for the `useModulatingDriver()` option:

* DigitDriver w/ LedMatrixDirect
    * min: 16us; avg: 80us; max: 108us
* DigitDriver w/ LedMatrixSerial
    * min: 16us; avg: 84-139us; max: 204us
* DigitDriver w/ LedMatrixSpi
    * min: 16us; avg: 30-55us; max: 80us
* ModulatingDigitDriver w/ LedMatrixDirect
    * min: 8us; avg: 12-20us; max: 140us
* ModulatingDigitDriver w/ LedMatrixSerial
    * min: 8us; avg: 12-20us; max: 212us
* ModulatingDigitDriver w/ LedMatrixSpi
    * min: 8us; avg: 12-20us; max: 100us
* SegmentDriver w/ LedMatrixDirect
    * min: 28us; avg: 36-76; max: 96us
* fast_driver.py w/ segment_direct_pins
    * min: 8us; avg: 13us; max: 84us
* fast_driver.py, segment_serial_pins
    * min: 8us; avg: 13us; max: 80us
* fast_driver.py, segment_spi_pins
    * min: 8us; avg: 13us; max: 76us


If we want to drive a 4 digit LED display at 60 frames per second, using a
subfield modulation of 16 subfields per field, we get a field rate of 3.84 kHz,
or 260 microseconds per field. All of the options had a maximum time
of less than 260 microseconds.

The `fast_driver.py` generates code that is fast enough to drive a 4-digit LED
display on an 8MHz ATmega328P microcontroller at a frame rate of 60Hz, with 16
subfields per field for pulse width modulation.


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
