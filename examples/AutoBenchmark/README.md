# AutoBenchmark

This program creates instances of various subclasses `LedModule` using different
configurations:

* `BareModule`: group and segment pins directly connected to MCU
* `BareFast4Module`: same as `BareModule` but using `digitalWriteFast` library
* `SingleHc595Module`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through SPI
* `DualHc595Module`: group pins and segment pins connected to two 74HC595
  which are accessed through SPI
* `Tm1637Module`: an LED module using a TM1637 driver chip, accessed through
  a two-wire protocol similar to I2C
* `Max7219Module`: an LED module using a MAX7219 driver chip, accessed through
  SPI

We then measure the time taken by the methods used to render the digit to the
LED module:

* Subclasses of `ScanningModule`
    * Measure the time taken by `ScanningModule::renderFieldNow()` which renders
      a single digit (multiple fields make up a frame (a single frame is the
      complete rendering of all digits on the display module).
* `Tm1637Module::flush()
    * Sends out the buffered digits over the custom two-wire protocol.
* `Max7219Module::flush()`
    * Sends out the buffered digits using SPI.

**Version**: AceSegment v0.4+

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceCommon](https://github.com/bxparks/AceCommon)
* [AceSegment](https://github.com/bxparks/AceButton)

On AVR processors, the following library is required to run the
`digitalWriteFast()` versions of the low-level drivers:

* [digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast)

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
```

Fortunately, we no longer need to run `generate_table.awk` for each `*.txt`
file. The process has been automated using the `generate_readme.py` script which
will be invoked by the following command:
```
$ make README.md
```

The CPU times below are given in microseconds. The "samples" column is the
number of `TimingStats::update()` calls that were made.

## CPU Time Changes

**v0.4:**
* Huge refactoring of core AceSegment classes. Rewrote AutoBenchmark
  to match similar programs in the AceButton, AceCrc and AceTime libraries.

**v0.4+:**

* Add benchmarks for `Tm1637Module`. The CPU time is mostly determined by
  the calls to `delayMicroseconds()`, which is must be about 100 microseconds
  due to the unusually large capacitors (20 nF) installed on the DIO and CLK
  lines. They should have been about 100X smaller (200 pF).
* Add benchmarks for `Max7219Module`.
* Add benchmarks for `BareModule`, `BareFast4Module`, `SingleHc595Module`, and
  `DualHc595Module`.
* Upgrade from ESP32 Core v1.0.4 to v1.0.6.

## Results

The following tables show the number of microseconds taken by:

* `ScanningModule::renderFieldNow()`
    * renders the 8 segments of a single LED digit. If the LED module has 4
      digits, then `renderFieldNow()` must be called 4 times to render the light
      pattern of the entire LED module. The entire rendering is then called a
      frame.
* `Tm1637Module::flush()`
    * sends all digits in the buffer to the TM1637 LED module using the I2C-like
      protocol
    * a bitDelay of 100 microseconds is used
* `Max7219Module::flush()`
    * sends all digits in the buffer to the MAX7219 LED module using standard
      software or hardware SPI

Most people can no longer see flickering of the display at about 60 frames a
second. To achieve that, the `renderFieldNow()` method must be called 240
times a second for a module with 4 digits, or every 4.17 milliseconds. The
results below show that every processor, even the slowest AVR processor, is able
to meet this threshhold.

* For the `BaseModule` and `BareFast4Module`, this involves turning off the
  previous digit, sending the 8 bits for the current digit's 8 LED segments in a
  loop, then turning on the current digit. The `digitalWrite()` function is
  called 10 times.
* For the `SingleHc595Module` type, the 8 LED segment bits are sent
  using software SPI or hardware SPI. (Software SPI uses the `shiftOut()`
  method, which is implemented using a loop of `digitalWrite()`.
* For the `DualHc595Module` type, the LED digit pins and the LED
  segment pins are using conceptually a single SPI transaction. For software
  SPI, this is implemented using 2 `shiftOut()` operations. For hardware SPI,
  this uses a single `SPI::transfer16()` command.

On AVR processors, the "fast" options are available using the
[digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast) library whose
`digitalWriteFast()` functions can be up to 50X faster if the `pin` number and
`value` parameters are compile-time constants. In addition, the
`digitalWriteFast` functions reduce flash memory consumption by 600-700 bytes
for `SoftWireFastInterface`, `SoftSpiFastInterface`, and `HardSpiFastInterface`
compared to their non-fast equivalents.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftWireFastInterface<4, 5, 100>): 1
sizeof(SoftSpiInterface): 3
sizeof(SoftSpiFastInterface<11, 12, 13>): 1
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface<11, 12, 13>): 1
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 8
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 5
sizeof(LedModule): 3
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(BareModule<4>): 31
sizeof(BareFast4Module<...>): 25
sizeof(SingleHc595Module<SoftSpiInterface, 4>): 30
sizeof(DualHc595Module<SoftSpiInterface, 4>): 27
sizeof(Tm1637Module<SoftWireInterface, 4>): 14
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(LedDisplay): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 2
sizeof(StringWriter): 2
sizeof(StringScroller): 7

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| BareModule                             |    68/   74/   88 |     240 |
| BareModule(subfields)                  |     4/   12/   84 |    3840 |
| BareFast4Module                        |    24/   28/   36 |     240 |
| BareFast4Module(subfields)             |     4/    8/   40 |    3840 |
|----------------------------------------+-------------------+---------|
| SingleHc595(SoftSpi)                   |   156/  159/  180 |     240 |
| SingleHc595(SoftSpi,subfields)         |     4/   22/  180 |    3840 |
| SingleHc595(SoftSpiFast)               |    28/   30/   44 |     240 |
| SingleHc595(SoftSpiFast,subfields)     |     4/    8/   40 |    3840 |
| SingleHc595(HardSpi)                   |    36/   39/   52 |     240 |
| SingleHc595(HardSpi,subfields)         |     4/    9/   52 |    3840 |
| SingleHc595(HardSpiFast)               |    24/   27/   44 |     240 |
| SingleHc595(HardSpiFast,subfields)     |     4/    7/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| DualHc595(SoftSpi)                     |   264/  269/  304 |     240 |
| DualHc595(SoftSpi,subfields)           |     4/   35/  304 |    3840 |
| DualHc595(SoftSpiFast)                 |    20/   24/   36 |     240 |
| DualHc595(SoftSpiFast,subfields)       |     4/    7/   32 |    3840 |
| DualHc595(HardSpi)                     |    24/   27/   40 |     240 |
| DualHc595(HardSpi,subfields)           |     4/    8/   36 |    3840 |
| DualHc595(HardSpiFast)                 |    12/   14/   28 |     240 |
| DualHc595(HardSpiFast,subfields)       |     4/    6/   24 |    3840 |
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

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftWireFastInterface<4, 5, 100>): 1
sizeof(SoftSpiInterface): 3
sizeof(SoftSpiFastInterface<11, 12, 13>): 1
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface<11, 12, 13>): 1
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 8
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 5
sizeof(LedModule): 3
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(BareModule<4>): 31
sizeof(BareFast4Module<...>): 25
sizeof(SingleHc595Module<SoftSpiInterface, 4>): 30
sizeof(DualHc595Module<SoftSpiInterface, 4>): 27
sizeof(Tm1637Module<SoftWireInterface, 4>): 14
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(LedDisplay): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 2
sizeof(StringWriter): 2
sizeof(StringScroller): 7

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| BareModule                             |    72/   76/   84 |     240 |
| BareModule(subfields)                  |     4/   14/   92 |    3840 |
| BareFast4Module                        |    24/   28/   36 |     240 |
| BareFast4Module(subfields)             |     4/    8/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| SingleHc595(SoftSpi)                   |   148/  153/  164 |     240 |
| SingleHc595(SoftSpi,subfields)         |     4/   23/  164 |    3840 |
| SingleHc595(SoftSpiFast)               |    28/   31/   40 |     240 |
| SingleHc595(SoftSpiFast,subfields)     |     4/    8/   48 |    3840 |
| SingleHc595(HardSpi)                   |    36/   41/   52 |     240 |
| SingleHc595(HardSpi,subfields)         |     4/    9/   52 |    3840 |
| SingleHc595(HardSpiFast)               |    24/   27/   44 |     240 |
| SingleHc595(HardSpiFast,subfields)     |     4/    8/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| DualHc595(SoftSpi)                     |   248/  253/  264 |     240 |
| DualHc595(SoftSpi,subfields)           |     4/   37/  268 |    3840 |
| DualHc595(SoftSpiFast)                 |    20/   23/   32 |     240 |
| DualHc595(SoftSpiFast,subfields)       |     4/    8/   32 |    3840 |
| DualHc595(HardSpi)                     |    24/   28/   36 |     240 |
| DualHc595(HardSpi,subfields)           |     4/    8/   40 |    3840 |
| DualHc595(HardSpiFast)                 |    12/   14/   20 |     240 |
| DualHc595(HardSpiFast,subfields)       |     4/    6/   20 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22428/22440/22452 |      20 |
| Tm1637(SoftWireFast)                   | 21168/21176/21188 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |  1244/ 1247/ 1252 |      20 |
| Max7219(SoftSpiFast)                   |   112/  115/  120 |      20 |
| Max7219(HardSpi)                       |   124/  129/  140 |      20 |
| Max7219(HardSpiFast)                   |    56/   58/   68 |      20 |
+----------------------------------------+-------------------+---------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(BareModule<4>): 48
sizeof(SingleHc595Module<SoftSpiInterface, 4>): 48
sizeof(DualHc595Module<SoftSpiInterface, 4>): 44
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| BareModule                             |    24/   24/   29 |     240 |
| BareModule(subfields)                  |     2/    5/   29 |    3840 |
|----------------------------------------+-------------------+---------|
| SingleHc595(SoftSpi)                   |    52/   53/   58 |     240 |
| SingleHc595(SoftSpi,subfields)         |     2/    8/   58 |    3840 |
| SingleHc595(HardSpi)                   |    24/   24/   28 |     240 |
| SingleHc595(HardSpi,subfields)         |     2/    5/   29 |    3840 |
|----------------------------------------+-------------------+---------|
| DualHc595(SoftSpi)                     |    89/   90/   95 |     240 |
| DualHc595(SoftSpi,subfields)           |     2/   13/   95 |    3840 |
| DualHc595(HardSpi)                     |    22/   22/   26 |     240 |
| DualHc595(HardSpi,subfields)           |     2/    5/   26 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22225/22228/22237 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |   448/  451/  455 |      20 |
| Max7219(HardSpi)                       |   107/  107/  113 |      20 |
+----------------------------------------+-------------------+---------+

```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(BareModule<4>): 48
sizeof(SingleHc595Module<SoftSpiInterface, 4>): 48
sizeof(DualHc595Module<SoftSpiInterface, 4>): 44
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| BareModule                             |    13/   14/   19 |     240 |
| BareModule(subfields)                  |     1/    3/   28 |    3840 |
|----------------------------------------+-------------------+---------|
| SingleHc595(SoftSpi)                   |    30/   31/   36 |     240 |
| SingleHc595(SoftSpi,subfields)         |     1/    4/   41 |    3840 |
| SingleHc595(HardSpi)                   |    40/   40/   45 |     240 |
| SingleHc595(HardSpi,subfields)         |     1/    5/   50 |    3840 |
|----------------------------------------+-------------------+---------|
| DualHc595(SoftSpi)                     |    53/   53/   58 |     240 |
| DualHc595(SoftSpi,subfields)           |     1/    6/   77 |    3840 |
| DualHc595(HardSpi)                     |    39/   40/   45 |     240 |
| DualHc595(HardSpi,subfields)           |     1/    6/   63 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22399/22403/22411 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |   267/  268/  272 |      20 |
| Max7219(HardSpi)                       |   199/  200/  204 |      20 |
+----------------------------------------+-------------------+---------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(BareModule<4>): 48
sizeof(SingleHc595Module<SoftSpiInterface, 4>): 48
sizeof(DualHc595Module<SoftSpiInterface, 4>): 44
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| BareModule                             |    12/   12/   25 |     240 |
| BareModule(subfields)                  |     0/    2/   25 |    3840 |
|----------------------------------------+-------------------+---------|
| SingleHc595(SoftSpi)                   |    29/   29/   37 |     240 |
| SingleHc595(SoftSpi,subfields)         |     0/    4/   42 |    3840 |
| SingleHc595(HardSpi)                   |    11/   11/   19 |     240 |
| SingleHc595(HardSpi,subfields)         |     0/    2/   23 |    3840 |
|----------------------------------------+-------------------+---------|
| DualHc595(SoftSpi)                     |    50/   50/   63 |     240 |
| DualHc595(SoftSpi,subfields)           |     0/    7/   67 |    3840 |
| DualHc595(HardSpi)                     |    12/   12/   19 |     240 |
| DualHc595(HardSpi,subfields)           |     0/    2/   32 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 21496/21505/21552 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |   254/  256/  271 |      20 |
| Max7219(HardSpi)                       |    61/   61/   70 |      20 |
+----------------------------------------+-------------------+---------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(BareModule<4>): 48
sizeof(SingleHc595Module<SoftSpiInterface, 4>): 48
sizeof(DualHc595Module<SoftSpiInterface, 4>): 44
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| BareModule                             |     2/    2/   10 |     240 |
| BareModule(subfields)                  |     0/    1/    9 |    3840 |
|----------------------------------------+-------------------+---------|
| SingleHc595(SoftSpi)                   |     4/    4/    7 |     240 |
| SingleHc595(SoftSpi,subfields)         |     0/    1/   13 |    3840 |
| SingleHc595(HardSpi)                   |     9/    9/   17 |     240 |
| SingleHc595(HardSpi,subfields)         |     0/    1/   18 |    3840 |
|----------------------------------------+-------------------+---------|
| DualHc595(SoftSpi)                     |     7/    7/   16 |     240 |
| DualHc595(SoftSpi,subfields)           |     0/    1/   15 |    3840 |
| DualHc595(HardSpi)                     |     9/    9/   16 |     240 |
| DualHc595(HardSpi,subfields)           |     0/    1/   14 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 21226/21239/21247 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |    33/   34/   41 |      20 |
| Max7219(HardSpi)                       |    44/   44/   51 |      20 |
+----------------------------------------+-------------------+---------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(BareModule<4>): 48
sizeof(SingleHc595Module<SoftSpiInterface, 4>): 48
sizeof(DualHc595Module<SoftSpiInterface, 4>): 44
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| BareModule                             |     5/    5/   10 |     240 |
| BareModule(subfields)                  |     0/    1/    7 |    3840 |
|----------------------------------------+-------------------+---------|
| SingleHc595(SoftSpi)                   |     9/   10/   13 |     240 |
| SingleHc595(SoftSpi,subfields)         |     0/    1/   17 |    3840 |
| SingleHc595(HardSpi)                   |     3/    3/    4 |     240 |
| SingleHc595(HardSpi,subfields)         |     0/    1/    6 |    3840 |
|----------------------------------------+-------------------+---------|
| DualHc595(SoftSpi)                     |    16/   16/   20 |     240 |
| DualHc595(SoftSpi,subfields)           |     0/    2/   19 |    3840 |
| DualHc595(HardSpi)                     |     3/    3/    4 |     240 |
| DualHc595(HardSpi,subfields)           |     0/    1/    6 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 21160/21161/21166 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |    83/   83/   85 |      20 |
| Max7219(HardSpi)                       |    16/   16/   21 |      20 |
+----------------------------------------+-------------------+---------+

```

