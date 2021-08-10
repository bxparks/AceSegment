# AceSegment Scanning Module

The easiest way to integrate with a seven-segment LED display is to get one of
the off-the-shelf LED modules based on the TM1637, MAX7219 or 74HC595 controller
chip. Creating your own custom LED display using raw components is possible, but
it can be tedious process due to the number of wires that you need to connect.
However, it is a great learning experience and I originally created the
AceSegment library to enable my own custom LED modules.

This document explains some of the inner workings of `ScanningModule` and its
associated the `LedMatrix` classes. These low-level classes are mostly hidden
from public view, but they form the building blocks for implementing the
`Hc595Module`, `HybridModule` and `DirectModule` classes.

**Warning**: This document is hard to keep up-to-date. There may be a number of
bugs or missing pieces in the code fragments below.

## Table of Contents

* [LED Wiring](#LedWiring)
    * [Resistors and Transistors](#ResistorsAndTransistors)
* [LedMatrix](#LedMatrix)
    * [LedMatrixClasses](#LedMatrixClasses)
    * [Choosing the LedMatrix](#ChoosingLedMatrix)
        * [Pins Wired Directly, Common Cathode](#LedMatrixDirectCommonCathode)
        * [Pins Wired Directly, Common Anode](#LedMatrixDirectCommonAnode)
        * [Segments On Single 74HC595](#LedMatrixSingleHc595)
        * [Digits and Segments On Dual 74Hc595](#LedMatrixDualHc595)
* [Scanning Module](#ScanningModule)
    * [Scanning Module Classes](#ScanningModuleClasses)
    * [Setting Up the Scanning Module](#SettingUpScanningModule)
    * [Using the ScanningModule](#UsingScanningModule)
        * [Writing the Digit Bit Patterns](#DigitBitPatterns)
        * [Global Brightness](#GlobalBrightness)
        * [Frames and Fields](#FramesAndFields)
        * [Rendering by Polling](#RenderingByPolling)
        * [Rendering using Interrupts](#RenderingUsingInterrupts)

<a name="LedWiring"></a>
## LED Wiring

Raw LED modules come in 2 flavors:

* common cathode
* common anode

The AceSegment classes support both through an bit-flipping XOR mechanism
in the `LedModuleBase` class.

Current limiting resistors are required to prevent burning out the LED segments.
There are two options:

* resistors on segments
* resistors on digits (not supported)

At first glance, there is not an obvious difference between "resistors on
segments" configuration and "resistors on digits". I recommend using resistors
on segments. That's because the LEDs with the resistors are the ones that can be
turned on at the same time, and the ones without the resistors are multiplexed
to give the illusion of a fully lit display. With the resistors on the segments,
all the segments on one digit can be activated at the same time, and we can use
pulse width modulation on the digit line to control the brightness of a single
digit. AceSegment supports only the `resistors on segments` configuration for
simplicity. (Code that supports "resistors on digits" has been moved into the
`archived/` directory).

Multiple LED segments are connected to a single pin. If the microcontroller
cannot supply the current required by all the segments on a particular pin, a
transistor must be used to handle the larger current. This usually means that
the logical level of the pin becomes inverted. The `LedMatrix` classes can
handle driver transistors using the same bit-flipping XOR mechanism used to
handle common cathode and common anode configurations.

<a name="ResistorsAndTransistors"></a>
### Resistors and Transistors

In the following circuit diagrams, `R` represents the current limiting resistors
and `T` represents the driving transistor. The resistor protects each LED
segment, and the transistor prevents overloading the GPIO pin of the
microcontroller.

On an Arduino Nano for example, each GPIO pin can handle 40mA of current. If the
value of `R` is high enough that each segment only drew only 5mA of current, the
transistor could be omitted since 8 x 5mA is 40mA, which is within the limit of
a single GPIO pin.

If the `R` value is increased so that each LED segment is pushed to its rated
current limit (probably 10-15mA), then 8 segments could potentially push
80-120mA into the single digit pin of the Nano, which would exceed the maximum
rating. A driver transistor would be needed on the digit pin to handle this
current.

If driver transistors are used on the digits, then it is likely that the logic
levels need to be inverted software. In the example below, using common cathode
display, with the resistors on the segments, an NPN transistor on the digit,
the digit pin on the microcontroller (D12) needs to be HIGH to turn on the LED
segment. In contrast, if the D12 pin was connected directly to the LED, the
digit pin would need to be set LOW to turn on the LED.

```
MCU                      LED display (Common Cathode)
+-----+                  +------------------------+
|  D08|------ R ---------|a -------.              |
|  D09|------ R ---------|b -------|--------.     |
|  D10|------ R ---------|c        |        |     |
|  D11|------ R ---------|d      -----    -----   |
|  D12|------ R ---------|e       \ /      \ /    |
|  D13|------ R ---------|f      --v--    --v--   |
|  D14|------ R ---------|g        |        |     |
|  D15|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|     |              +---|D1 ------+--------'     |
|     |             /    |                        |
|     |            /     +------------------------+
|  D04|----- R ---| NPN
|     |            \
+-----+             v
                    |
                   GND
```

If the LED display was using common anode, instead of common cathode as shown
above, then a PNP transistor would be used, with the emitter tied to Vcc. Again,
the logic level on the D12 pin will become inverted compared to wiring the digit
pin directly to the microcontroller.

<a name="LedMatrix"></a>
## LedMatrix

<a name="LedMatrixClasses"></a>
### LedMatrix Classes

Several `LedMatrix` classes capture the wiring of the matrix of LEDs.

* `LedMatrixDirect`
    * Group pins and element pins are directly accessed through the
        microcontroller pins.
* `LedMatrixDirectFast4`
    * Same as `LedMatrixDirect` but using `digitalWriteFast()` on AVR
        processors
* `LedMatrixSingleHc595`
    * Group pins are access directly, but element pins are access through an
        74HC595 chip through SPI using one of SpiInterface classes
* `LedMatrixDualHc595`
    * Both group and element pions are access through two 74HC595 chips
        through SPI using one of the SpiInterface classes

<a name="ChoosingLedMatrix"></a>
### Choosing the LedMatrix

The `LedMatrix` captures the wiring information about the LED module.
The best way to show how to use these methods is probably through
examples.

<a name="LedMatrixDirectCommonCathode"></a>
#### Pins Wired Directly, Common Cathode

The wiring for this configuration looks like this:
```
MCU                      LED display (Common Cathode)
+-----+                  +------------------------+
|  D08|------ R ---------|a -------.              |
|  D09|------ R ---------|b -------|--------.     |
|  D10|------ R ---------|c        |        |     |
|  D11|------ R ---------|d      -----    -----   |
|  D12|------ R ---------|e       \ /      \ /    |
|  D13|------ R ---------|f      --v--    --v--   |
|  D14|------ R ---------|g        |        |     |
|  D15|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|  D04|---- R - T -------|D1 ------'--------'     |
|  D05|---- R - T -------|D2                      |
|  D06|---- R - T -------|D3                      |
|  D07|---- R - T -------|D4                      |
+-----+                  +------------------------+
```

The `LedMatrixDirect` constructor is:

```C++
const uint8_t NUM_SEGMENTS = 8;
const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 11, 12, 13, 14, 15};
const uint8_t NUM_DIGITS = 4;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
const uint16_t FRAMES_PER_SECOND = 60;

using LedMatrix = LedMatrixDirect<>;
LedMatrix ledMatrix(
    kActiveHighPattern /*elementOnPattern*/,
    kActiveHighPattern /*groupOnPattern*/,
    NUM_SEGMENTS,
    SEGMENT_PINS,
    NUM_DIGITS,
    DIGIT_PINS);
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
...

void setupScanningModule() {
  ledMatrix.begin();
  scanningModule.begin();
}
```

<a name="LedMatrixDirectCommonAnode"></a>
#### Pins Wired Directly, Common Anode

The wiring for this configuration looks like this:
```
MCU                      LED display (Common Anode)
+-----+                  +------------------------+
|  D08|------ R ---------|a -------.              |
|  D09|------ R ---------|b -------|--------.     |
|  D10|------ R ---------|c        |        |     |
|  D11|------ R ---------|d      --^--    --^--   |
|  D12|------ R ---------|e       / \      / \    |
|  D13|------ R ---------|f      -----    -----   |
|  D14|------ R ---------|g        |        |     |
|  D15|------ R ---------|h        |        |     |
|     |                  |         |        |     |
|  D04|---- R - T -------|D1 ------'--------'     |
|  D05|---- R - T -------|D2                      |
|  D06|---- R - T -------|D3                      |
|  D07|---- R - T -------|D4                      |
+-----+                  +------------------------+
```

The `LedMatrixDirect` configuration is *exactly* the same as the Common Cathode
case above, except that `kActiveHighPattern` is replaced with
`kActiveLowPattern`.

<a name="LedMatrixSingleHc595"></a>
#### Segments on Single 74HC595

The segment pins can be placed on a 74HC595 shift register chip that can be
accessed through SPI. The caveat is that this chip can supply only 6 mA per pin
(as opposed to 40 mA for an Arduino Nano for example). But it may be enough in
some applications.

We assume here that the registors are on the segment pins, and that we are using
a common cathode LED display.

```
MCU          74HC595             LED display (Common Cathode)
+--------+   +---------+         +------------------------+
|        |   |       Q0|--- R ---|a -------.              |
|        |   |       Q1|--- R ---|b -------|--------.     |
|     D10|---|ST_CP  Q2|--- R ---|c        |        |     |
|MOSI/D11|---|DS     Q3|--- R ---|d      -----    -----   |
| SCK/D13|---|SH_CP  Q4|--- R ---|e       \ /      \ /    |
|        |   |       Q5|--- R ---|f      --v--    --v--   |
|        |   |       Q6|--- R ---|g        |        |     |
|        |   |       Q7|--- R ---|h        |        |     |
|        |   +---------+         |         |        |     |
|        |                       |         |        |     |
|     D04|------ R - T ----------|D1 ------'--------'     |
|     D05|------ R - T ----------|D2                      |
|     D06|------ R - T ----------|D3                      |
|     D07|------ R - T ----------|D4                      |
+--------+                       +------------------------+
```

The `LedMatrixSingleHc595` configuration using software SPI is:

```C++
const uint8_t NUM_DIGITS = 4;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = 11; // DS on 74HC595
const uint8_t CLOCK_PIN = 13; // SH_CP on 74HC595
const uint16_t FRAMES_PER_SECOND = 60;

// Common Cathode, with transistors on Group pins
SimpleSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
using LedMatrix = LedMatrixSingleHc595<SimpleSpiInterface>;
LedMatrix ledMatrix(
    spiInterface,
    kActiveHighPattern /*elementOnPattern*/,
    kActiveHighPattern /*groupOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS):
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
...

void setupScanningModule() {
  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
}
```

The `LedMatrixSingleHc595` configuration using hardware SPI is *exactly*
the same as above but with `HardSpiInterface` replacing `SimpleSpiInterface`.

<a name="LedMatrixDualHc595"></a>
#### Digits and Segments on Dual 74HC595

In this wiring, both the segment pins and the digit pins are wired to
two 74HC595 chips so that both sets of pins are set through SPI.

```
MCU                 74HC595             LED display (Common Cathode)
+--------+          +---------+         +------------------------+
|        |          |       Q0|--- R ---|a -------.              |
|        |          |       Q1|--- R ---|b -------|--------.     |
|     D10|--+-------|ST_CP  Q2|--- R ---|c        |        |     |
|MOSI/D11|--|-------|DS     Q3|--- R ---|d      -----    -----   |
| SCK/D13|--|---+---|SH_CP  Q4|--- R ---|e       \ /      \ /    |
|        |  |   |   |       Q5|--- R ---|f      --v--    --v--   |
|        |  |   |   |       Q6|--- R ---|g        |        |     |
|        |  |   | +-|Q7'    Q7|--- R ---|h        |        |     |
|        |  |   | | +---------+         |         |        |     |
|        |  |   | |                     |         |        |     |
|        |  |   | | 74HC595             |         |        |     |
|        |  |   | | +---------+         |         |        |     |
|        |  |   | | |       Q0|- R - T -|D1 ------+--------+     |
|        |  |   | | |       Q1|- R - T -|D2                      |
|        |  +---|-|-|ST_CP  Q2|- R - T -|D3                      |
|        |      | +-|DS     Q3|- R - T -|D4                      |
|        |      +---|SH_CP  Q4|         |                        |
|        |          |       Q5|         |                        |
|        |          |       Q6|         |                        |
+--------+          |       Q7|         +------------------------+
                    +---------+
```

The `LedMatrixDualHc595` configuration is the following. Let's use
`HardSpiInterface` this time:

```C++
const uint8_t NUM_DIGITS = 4;
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = 11; // DS on 74HC595
const uint8_t CLOCK_PIN = 13; // SH_CP on 74HC595
const uint16_t FRAMES_PER_SECOND = 60;

HardSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
using LedMatrix = LedMatrixSingleHc595<HardSpiInterface>;
LedMatrix ledMatrix(
    spiInterface,
    kActiveHighPattern /*elementOnPattern*/,
    kActiveHighPattern /*groupOnPattern*/);
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
...

void setupScanningModule() {
  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
}
```

<a name="ScanningModule"></a>
## ScanningModule

The `ScanningModule` is the base class of 4 convenience classes:

* `DirectModule`
* `DirectModuleFast4`
* `HybridModule`
* `Hc595Module`

The class hierarchy diagram looks like this:

```
                    ScanningModule
                         ^
                         |
               +---------+------------+
               |         |            |
      DirectModule  HybridModule   Hc595Module
 DirectFast4Module       |                \
           /             |                 \
          /              |                  \
         v               v                   v
  LedMatrixDirect   LedMatrixSingleHc595  LedMatrixDualHc595
LedMatrixDirectFast4              \             /
                                   \           /
                                    v         v
                                   SimpleSpiInterface
                                   SimpleSpiFastInterface
                                   HardSpiInterface
                                   HardSpiFastInterface
```

`ScanningModule` class is designed for LED modules which do not have a hardware
controller that performs the multiplexing, so must be manually multiplexed by
the host microcontroller

Instead of using the convenience classes, it is possible to use the
`ScanningModule` class directly with one of its associated `LedMatrixXxx`
classes.

<a name="SettingUpScanningModule"></a>
### Setting Up the Scanning Module

A series of resources must be built up to finally create an instance of
`ScanningModule`. For an LED module using direct scanning, the `ScanningModule`
is used, and the resource creation occurs in roughly 4 stages, with the objects
in the later stages depending on the objects created in the earlier stage:

1. Create the `SpiInterface` object that determines whether software SPI or
   hardware SPI is used. Needed only by `LedMatrixSingleHc595` and
   `LedMatrixDualHc595` classes.
1. Create the LedMatrix object that determines how the LEDs are wired and how to
   communicate to the LED segments.
1. Create the `ScanningModule` object that represents the actual LED segment
   module using direct scanning, or scanning through an 74HC595 shift register
   chip.
1. (Optional) Create the various Writer classes from the
   [AceSegmentWriter](https://github.com/bxparks/AceSegmentWriter) library which
   can write numbers, characters, and strings to the LED module.

A typical resource creation code looks like this:

```C++
#include <AceSegment.h>
#include <AceSegmentWriter.h> // NumberWriter (optional)

using namespace ace_segment;

const uint8_t NUM_SEGMENTS = 8;
const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 11, 12, 13, 14, 15};
const uint8_t NUM_DIGITS = 4;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
const uint16_t FRAMES_PER_SECOND = 60;

// The chain of resources.
using LedMatrix = LedMatrixDirect<>;
LedMatrix ledMatrix(
    kActiveLowPattern /*elementOnPattern*/,
    kActiveLowPattern /*groupOnPattern*/,
    NUM_SEGMENTS,
    SEGMENT_PINS,
    NUM_DIGITS,
    DIGIT_PINS);
ScanningModule<LedMatrix, NUM_DIGITS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);

// NumberWriter from AceSegmentWriter library
NumberWriter<LedModule> numberWriter(scanningModule);

void setupAceSegment() {
  ledMatrix.begin();
  scanningModule.begin();
}

void setup() {
  delay(1000);

  setupAceSegment();
  ...
}
```

<a name="UsingScanningModule"></a>
### Using the ScanningModule

<a name="DigitBitPatterns"></a>
#### Writing Digit Bit Patterns

The `ScanningModule` contains a number of methods to write the bit patterns of
the seven segment display:

* `void writePatternAt(uint8_t pos, uint8_t pattern)`
* `void writeDecimalPointAt(uint8_t pos, bool state = true)`
* `void setBrightnessAt(uint8_t pos, uint8_t brightness)`

The `writeDecimalPointAt()` is a special method that sets the bit corresponding
to the decimal point ('h', bit 7), no matter what previous pattern was there in
initially. The `state` variable controls whether the decimal point should
be turned on (default) or off (false).

The `brightness` is an integer associated with the digit. The range of
brightness is controlled by `NUM_SUBFIELDS`. It requires the `ScanningModule`
object to be configured to support PWM on the digit pins. Otherwise, the
brightness is ignored.

<a name="GlobalBrightness"></a>
#### Global Brightness

If the `ScanningModule` supports it, we can control the global brightness of
the entire LED display using:

```
scanningModule.setBrightness(value);
```

Note that the `value` is an integer from `[0, NUM_SUBFIELDS]`, and represents
the brightness of the display, where 0 means OFF and `NUM_SUBFIELDS` means 100%
ON.

The global brightness is enabled only if the `NUM_SUBFIELDS` template parameter
of the `ScanningModule` was set to be `> 1`. By default, this is set to 1. For
example, this creates a `ScanningModule` using hardware SPI, setting the
`NUM_SUBFIELDS` to be 16:

```
const uint8_t NUM_DIGITS = 4;
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = 11; // DS on 74HC595
const uint8_t CLOCK_PIN = 13; // SH_CP on 74HC595
const uint16_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;

HardSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
using LedMatrix = LedMatrixSingleHc595<HardSpiInterface>;
LedMatrix ledMatrix(
    spiInterface,
    kActiveHighPattern /*elementOnPattern*/,
    kActiveHighPattern /*groupOnPattern*/);
ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
    ledMatrix, FRAMES_PER_SECOND);
```

Each digit is rendered 16 times within a single field, and modulated using pulse
width modulation to control the width of that signal. The given digit will be
on only a fraction of the full interval of the entire rendering of the  field
and will appear dimmer to the human eye.

<a name="FramesAndFields"></a>
#### Frames and Fields

To understand how to the `ScanningModule` supports brightness, we first need to
explain a couple of terms that we borrowed from the field of
[video processing](https://en.wikipedia.org/wiki/Field_(video)):

* **Frame**: A frame is a complete rendering of all digits of the seven segment
  display. A frame is intended to be a single, conceptually static image of the
  LED display. Any changes in bit patterns or brightness of the digits happens
  through the rendering of multiple frames.
* **Field**: A field is a partial rendering of a frame. If the current limiting
  resistors are on the segments (recommended), then the `ScanningModule`
  multiplexes through the digits. Each rendering of the digit is a *field* and
  for a 4-digit display, there are 4 fields per frame.

A *frame* rate of about 60Hz is recommended to eliminate obvious visual
flickering. If the LED display has 4 digits, and we use "resistors on segments"
configuration, then we need to have a *field* rate of 240Hz. We will see later
that if we want dimmable digits using PWM, then we need about 8-16 subfields
within a field, giving a total *field* rate of about 2000-4000Hz. That's abaout
250-500 microseconds per field, which is surprisingly doable using an 8-bit
processor like an Arduino UNO or Nano on an ATmega328 running at 16MHz.

The primary unit of rendering in `ScanningModule` is a single field,
implemented in `ScanningModule::renderFieldNow()`. The `ScanningModule` class
keeps track of the current digit, the current frame, and the current field, and
each successive call to `renderFieldNow()` sends the appropriate bit pattern to
the LED module.

For a given requested frame rate, are 2 ways to render the fields at the correct
time, and they are explained below:

* Polling
* Interrupts

<a name="RenderingByPolling"></a>
#### Rendering By Polling

The `ScanningModule::renderFieldWhenReady()` is meant to be called at a
frequency somewhat higher than that needed to sustain the actual frame rate. It
keeps an internal variable containing the time (in `micros()`) of the previous
rendering of the field. When the time is up, it calls `renderFieldNow()` and
updates the internal clock to be ready for the next call.

The code looks like this:

```
void loop() {
  scanningModule.renderFieldWhenReady();
}
```

The problem with using this method is that it's difficult to get much else done
in the `loop()` method. If those other side things take up too much time, then
the refreshing rate of the LED module will wander, and the eyes will notice
flickering of the LED module.

Using this polling method is the easiest way to get AceSegment working. But many
non-trivial project will want to use the timer interrupt method to avoid the
flickering problem.

<a name="RenderingUsingInterrupts"></a>
#### Rendering Using Interrupts

The calling code sets up an interrupt service routine (ISR) which calls
`ScanningModule::renderFieldNow()` at exactly the periodic frequency needed to
achieve the desired frames per second and fields per second.

Unfortunately, timer interrupts are not part of the Arduino API (probably
because every microcontroller handles interrupts in a slightly different way).
An easy way to run an ISR periodically is to use one of the TimerXxx libraries:

* TimerOne (https://github.com/PaulStoffregen/TimerOne)
* TimerThree (https://github.com/PaulStoffregen/TimerThree)
* TimerFour (https://github.com/VincentLim/TimerFour)
* TimerFour (https://github.com/VincentLim/TimerFour)
* TimerFive (https://github.com/VincentLim/TimerFive)

(I'm not sure why there is no TimerTwo library.)

An example code is given in
[examples/Hc595InterruptDemo](../examples/Hc595InterruptDemo), and looks like
this:

```C++
#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // Hc595Module
#include <TimerOne.h> // Timer1

using namespace ace_segment;

const uint8_t NUM_DIGITS = 4;
const uint8_t LATCH_PIN = 10;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;
const uint8_t FRAMES_PER_SECOND = 60;

const uint8_t NUM_SUBFIELDS = 8;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 9, 5, 2
};

using SpiInterface = SimpleSpiInterface;
SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
    spiInterface,
    SEGMENT_ON_PATTERN,
    DIGIT_ON_PATTERN,
    FRAMES_PER_SECOND,
    HC595_BYTE_ORDER,
    REMAP_ARRAY
);

void setupAceSegment() {
  spiInterface.begin();
  ledModule.begin();
}

void flushModule() {
  ledModule.renderFieldNow();
}

void setupTimer() {
  Timer1.initialize(ledModule.getMicrosPerField());
  Timer1.attachInterrupt(flushModule);
}

void updateDisplay() {
  ...
}

void setup() {
  ...
  setupAceSegment();
  setupTimer();
  ...
}

void loop() {
  updateDisplay();
}
```
