# AutoBenchmark

This program creates instances of `ScanningDisplay` using different
configurations of the `LedMatrix` class:

* `direct`: group and segment pins directly connected to MCU
* `single_sw_spi`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through software SPI (`SwSpiAdapter`)
* `single_hw_spi`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through hardware SPI (`HwSpiAdapter`)
* `dual_sw_spi`: group pins and segment pins connected to 74HC595 accessed
  through software SPI (`SwSpiAdapter`)
* `dual_hw_spi`: group pins and segment pins connected to 74HC595 accessed
  through hardware SPI (`HwSpiAdapter`)

It measures the time taken by `ScanningDisplay::renderFieldNow()` which
renders a single digit (multiple fields make up a frame, a frame is the
rendering of all digits on the display module).

**Version**: AceSegment v0.4

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

v0.4: Huge refactoring of core AceSegment classes. Rewrote AutoBenchmark
to match similar programs in the AceButton, AceCrc and AceTime libraries.

v0.5: Add benchmarks for `Tm1637Display`. The CPU time is mostly determined by
the calls to `delayMicroseconds()`, which is required to meet the electrical
characteristics of the LED module.

## Results

The following tables show the number of microseconds taken by:

* `ScanningDisplay::renderFieldNow()`
    * renders the 8 segments of a single LED digit. If the LED module has 4
      digits, then `renderFieldNow()` must be called 4 times to render the light
      pattern of the entire LED module. The entire rendering is then called a
      frame.
* `Tm1637Display::flush()`
    * sends all digits in the buffer to the TM1637 LED module using the I2C-like
      protocol
    * a bitDelay of 100 microseconds is used

Most people can no longer see flickering of the display at about 60 frames a
second. To achieve that, the `renderFieldNow()` method must be called 240
times a second for a module with 4 digits, or every 4.17 milliseconds. The
results below show that every processor, even the slowest AVR processor, is able
to meet this threshhold.

* For the `LedMatrixDirect` type, this involves turning off the previous digit,
  sending the 8 bits for the current digit's 8 LED segments in a loop, then
  turning on the current digit. The `digitalWrite()` function is called 10
  times.
* For the `LedMatrixSingleShiftRegister` type, the 8 LED segment bits are sent
  using software SPI or hardware SPI. (Software SPI uses the `shiftOut()`
  method, which is implemented using a loop of `digitalWrite()`.
* For the `LedMatrixDualShiftRegister` type, the LED digit pins and the LED
  segment pins are using conceptually a single SPI transaction. For software
  SPI, this is implemented using 2 `shiftOut()` operations. For hardware SPI,
  this uses a single `transfer16()` command.

On AVR processors, the "fast" options are available using the
[digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast) library whose
`digitalWriteFast()` functions can be up to 50X faster if the `pin` number and
`value` parameters are compile-time constants.

The `digitalWriteFast` library is useful to create the `Tm1637DriverFast` class,
because it consumes 600-700 fewer bytes of flash memory.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Hardware): 1
sizeof(SwSpiAdapter): 3
sizeof(SwSpiAdapterFast<1,2,3>): 1
sizeof(HwSpiAdapter): 3
sizeof(LedMatrixDirect<Hardware>): 11
sizeof(LedMatrixDirectFast<0..3, 0..7>): 3
sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): 10
sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): 5
sizeof(LedDisplay): 3
sizeof(ScanningDisplay<Hardware, LedMatrixBase, 4, 1>): 25
sizeof(Tm1637Display<Tm1637Driver, 4>): 12
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(CharWriter): 2
sizeof(StringWriter): 2

CPU:
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |    72/   76/   88 |     240 |
| Scanning(direct,subfields)             |     4/   12/   84 |    3840 |
| Scanning(single_sw_spi)                |   156/  159/  180 |     240 |
| Scanning(single_sw_spi,subfields)      |     4/   20/  180 |    3840 |
| Scanning(single_hw_spi)                |    36/   39/   52 |     240 |
| Scanning(single_hw_spi,subfields)      |     4/    8/   56 |    3840 |
| Scanning(dual_sw_spi)                  |   264/  269/  300 |     240 |
| Scanning(dual_sw_spi,subfields)        |     4/   30/  296 |    3840 |
| Scanning(dual_hw_spi)                  |    24/   26/   40 |     240 |
| Scanning(dual_hw_spi,subfields)        |     4/    7/   40 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(direct_fast)                  |    28/   29/   36 |     240 |
| Scanning(direct_fast,subfields)        |     4/    8/   44 |    3840 |
| Scanning(single_sw_spi_fast)           |    28/   31/   40 |     240 |
| Scanning(single_sw_spi_fast,subfields) |     4/    8/   40 |    3840 |
| Scanning(dual_sw_spi_fast)             |    20/   24/   32 |     240 |
| Scanning(dual_sw_spi_fast,subfields)   |     4/    7/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 22308/22326/22596 |      20 |
| Tm1637(Fast)                           | 21056/21069/21224 |      20 |
+----------------------------------------+-------------------+---------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Hardware): 1
sizeof(SwSpiAdapter): 3
sizeof(SwSpiAdapterFast<1,2,3>): 1
sizeof(HwSpiAdapter): 3
sizeof(LedMatrixDirect<Hardware>): 11
sizeof(LedMatrixDirectFast<0..3, 0..7>): 3
sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): 10
sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): 5
sizeof(LedDisplay): 3
sizeof(ScanningDisplay<Hardware, LedMatrixBase, 4, 1>): 25
sizeof(Tm1637Display<Tm1637Driver, 4>): 12
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(CharWriter): 2
sizeof(StringWriter): 2

CPU:
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |    72/   78/   88 |     240 |
| Scanning(direct,subfields)             |     4/   14/   92 |    3840 |
| Scanning(single_sw_spi)                |   148/  152/  164 |     240 |
| Scanning(single_sw_spi,subfields)      |     4/   23/  164 |    3840 |
| Scanning(single_hw_spi)                |    36/   40/   52 |     240 |
| Scanning(single_hw_spi,subfields)      |     4/    9/   52 |    3840 |
| Scanning(dual_sw_spi)                  |   248/  253/  264 |     240 |
| Scanning(dual_sw_spi,subfields)        |     4/   37/  264 |    3840 |
| Scanning(dual_hw_spi)                  |    24/   27/   36 |     240 |
| Scanning(dual_hw_spi,subfields)        |     4/    8/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(direct_fast)                  |    28/   28/   36 |     240 |
| Scanning(direct_fast,subfields)        |     4/    8/   36 |    3840 |
| Scanning(single_sw_spi_fast)           |    28/   31/   40 |     240 |
| Scanning(single_sw_spi_fast,subfields) |     4/    8/   40 |    3840 |
| Scanning(dual_sw_spi_fast)             |    20/   23/   32 |     240 |
| Scanning(dual_sw_spi_fast,subfields)   |     4/    8/   32 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 22424/22436/22448 |      20 |
| Tm1637(Fast)                           | 21164/21172/21180 |      20 |
+----------------------------------------+-------------------+---------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
Sizes of Objects:
sizeof(Hardware): 1
sizeof(SwSpiAdapter): 3
sizeof(HwSpiAdapter): 3
sizeof(LedMatrixDirect<Hardware>): 20
sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): 20
sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): 12
sizeof(LedDisplay): 8
sizeof(ScanningDisplay<Hardware, LedMatrixBase, 4, 1>): 36
sizeof(Tm1637Display<Tm1637Driver, 4>): 20
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |    24/   24/   30 |     240 |
| Scanning(direct,subfields)             |     3/    5/   30 |    3840 |
| Scanning(single_sw_spi)                |    51/   52/   58 |     240 |
| Scanning(single_sw_spi,subfields)      |     3/    7/   59 |    3840 |
| Scanning(single_hw_spi)                |    24/   24/   30 |     240 |
| Scanning(single_hw_spi,subfields)      |     3/    5/   30 |    3840 |
| Scanning(dual_sw_spi)                  |    87/   87/   93 |     240 |
| Scanning(dual_sw_spi,subfields)        |     3/   11/   94 |    3840 |
| Scanning(dual_hw_spi)                  |    22/   22/   28 |     240 |
| Scanning(dual_hw_spi,subfields)        |     3/    5/   28 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 22202/22205/22213 |      20 |
+----------------------------------------+-------------------+---------+

```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
Sizes of Objects:
sizeof(Hardware): 1
sizeof(SwSpiAdapter): 3
sizeof(HwSpiAdapter): 3
sizeof(LedMatrixDirect<Hardware>): 20
sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): 20
sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): 12
sizeof(LedDisplay): 8
sizeof(ScanningDisplay<Hardware, LedMatrixBase, 4, 1>): 36
sizeof(Tm1637Display<Tm1637Driver, 4>): 20
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |    13/   13/   19 |     240 |
| Scanning(direct,subfields)             |     1/    3/   37 |    3840 |
| Scanning(single_sw_spi)                |    29/   30/   35 |     240 |
| Scanning(single_sw_spi,subfields)      |     1/    4/   52 |    3840 |
| Scanning(single_hw_spi)                |    40/   40/   45 |     240 |
| Scanning(single_hw_spi,subfields)      |     1/    6/   63 |    3840 |
| Scanning(dual_sw_spi)                  |    51/   51/   56 |     240 |
| Scanning(dual_sw_spi,subfields)        |     1/    7/   74 |    3840 |
| Scanning(dual_hw_spi)                  |    40/   40/   45 |     240 |
| Scanning(dual_hw_spi,subfields)        |     1/    5/   50 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 22389/22393/22399 |      20 |
+----------------------------------------+-------------------+---------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(Hardware): 1
sizeof(SwSpiAdapter): 3
sizeof(HwSpiAdapter): 3
sizeof(LedMatrixDirect<Hardware>): 20
sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): 20
sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): 12
sizeof(LedDisplay): 8
sizeof(ScanningDisplay<Hardware, LedMatrixBase, 4, 1>): 36
sizeof(Tm1637Display<Tm1637Driver, 4>): 20
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |    12/   12/   48 |     240 |
| Scanning(direct,subfields)             |     0/    2/   24 |    3840 |
| Scanning(single_sw_spi)                |    29/   29/   33 |     240 |
| Scanning(single_sw_spi,subfields)      |     0/    4/   43 |    3840 |
| Scanning(single_hw_spi)                |    11/   11/   23 |     240 |
| Scanning(single_hw_spi,subfields)      |     0/    2/   24 |    3840 |
| Scanning(dual_sw_spi)                  |    50/   50/   58 |     240 |
| Scanning(dual_sw_spi,subfields)        |     1/    7/   67 |    3840 |
| Scanning(dual_hw_spi)                  |    12/   12/   24 |     240 |
| Scanning(dual_hw_spi,subfields)        |     1/    2/   28 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 21494/21500/21539 |      20 |
+----------------------------------------+-------------------+---------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
Sizes of Objects:
sizeof(Hardware): 1
sizeof(SwSpiAdapter): 3
sizeof(HwSpiAdapter): 3
sizeof(LedMatrixDirect<Hardware>): 20
sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): 20
sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): 12
sizeof(LedDisplay): 8
sizeof(ScanningDisplay<Hardware, LedMatrixBase, 4, 1>): 36
sizeof(Tm1637Display<Tm1637Driver, 4>): 20
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |     2/    2/   10 |     240 |
| Scanning(direct,subfields)             |     0/    1/    9 |    3840 |
| Scanning(single_sw_spi)                |     4/    4/   12 |     240 |
| Scanning(single_sw_spi,subfields)      |     0/    1/   13 |    3840 |
| Scanning(single_hw_spi)                |     9/    9/   18 |     240 |
| Scanning(single_hw_spi,subfields)      |     0/    1/   18 |    3840 |
| Scanning(dual_sw_spi)                  |     7/    7/   15 |     240 |
| Scanning(dual_sw_spi,subfields)        |     0/    1/   16 |    3840 |
| Scanning(dual_hw_spi)                  |     9/    9/   18 |     240 |
| Scanning(dual_hw_spi,subfields)        |     0/    1/   18 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 21226/21239/21246 |      20 |
+----------------------------------------+-------------------+---------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(Hardware): 1
sizeof(SwSpiAdapter): 3
sizeof(HwSpiAdapter): 3
sizeof(LedMatrixDirect<Hardware>): 20
sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): 20
sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): 12
sizeof(LedDisplay): 8
sizeof(ScanningDisplay<Hardware, LedMatrixBase, 4, 1>): 36
sizeof(Tm1637Display<Tm1637Driver, 4>): 20
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+----------------------------------------+-------------------+---------+
| LedDisplay Operation                   |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(direct)                       |     5/    5/   10 |     240 |
| Scanning(direct,subfields)             |     0/    1/    8 |    3840 |
| Scanning(single_sw_spi)                |    10/   10/   13 |     240 |
| Scanning(single_sw_spi,subfields)      |     0/    1/   13 |    3840 |
| Scanning(single_hw_spi)                |     4/    4/    6 |     240 |
| Scanning(single_hw_spi,subfields)      |     0/    1/    5 |    3840 |
| Scanning(dual_sw_spi)                  |    16/   16/   20 |     240 |
| Scanning(dual_sw_spi,subfields)        |     0/    2/   21 |    3840 |
| Scanning(dual_hw_spi)                  |     3/    3/    4 |     240 |
| Scanning(dual_hw_spi,subfields)        |     0/    1/    7 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(Normal)                         | 21152/21153/21158 |      20 |
+----------------------------------------+-------------------+---------+

```

