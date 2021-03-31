# AutoBenchmark

This program creates instances of `SegmentDisplay` using different
configurations of the `LedMatrix` class:

* `direct`: group and segment pins directly connected to MCU
* `partial_sw_spi`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through software SPI (`SwSpiAdapter`)
* `partial_hw_spi`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through hardware SPI (`HwSpiAdapter`)
* `full_sw_spi`: group pins and segment pins connected to 74HC595 accessed
  through software SPI (`SwSpiAdapter`)
* `full_hw_spi`: group pins and segment pins connected to 74HC595 accessed
  through hardware SPI (`HwSpiAdapter`)

**Version**: AceSegment v0.4

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceCommon](https://github.com/bxparks/AceCommon)
* [AceSegment](https://github.com/bxparks/AceButton)

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

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Hardware): 2
sizeof(SwSpiAdapter): 5
sizeof(HwSpiAdapter): 5
sizeof(LedMatrixDirect): 13
sizeof(LedMatrixPartialSpi): 12
sizeof(LedMatrixFullSpi): 7
sizeof(LedDisplay): 3
sizeof(SegmentDisplay<4, 1>): 28
sizeof(HexWriter): 2
sizeof(ClockWriter): 3
sizeof(CharWriter): 2
sizeof(StringWriter): 2

CPU:
+---------------------------+-------------+---------+
| LedMatrix type            | min/avg/max | samples |
|---------------------------+-------------+---------|
| direct                    |  72/ 77/ 92 | 1200    |
| partial_sw_spi            | 132/135/156 | 1200    |
| partial_hw_spi            |  36/ 40/ 52 | 1200    |
| full_sw_spi               | 212/217/244 | 1200    |
| full_hw_spi               |  24/ 24/ 40 | 1200    |
+---------------------------+-------------+---------+

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(Hardware): 2
sizeof(SwSpiAdapter): 5
sizeof(HwSpiAdapter): 5
sizeof(LedMatrixDirect): 13
sizeof(LedMatrixPartialSpi): 12
sizeof(LedMatrixFullSpi): 7
sizeof(LedDisplay): 3
sizeof(SegmentDisplay<4, 1>): 28
sizeof(HexWriter): 2
sizeof(ClockWriter): 3
sizeof(CharWriter): 2
sizeof(StringWriter): 2

CPU:
+---------------------------+-------------+---------+
| LedMatrix type            | min/avg/max | samples |
|---------------------------+-------------+---------|
| direct                    |  76/ 82/ 96 | 1200    |
| partial_sw_spi            | 132/137/152 | 1200    |
| partial_hw_spi            |  40/ 43/ 56 | 1200    |
| full_sw_spi               | 212/215/228 | 1200    |
| full_hw_spi               |  24/ 26/ 36 | 1200    |
+---------------------------+-------------+---------+

```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
Sizes of Objects:
sizeof(Hardware): 4
sizeof(SwSpiAdapter): 8
sizeof(HwSpiAdapter): 8
sizeof(LedMatrixDirect): 24
sizeof(LedMatrixPartialSpi): 24
sizeof(LedMatrixFullSpi): 16
sizeof(LedDisplay): 8
sizeof(SegmentDisplay<4, 1>): 40
sizeof(HexWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+---------------------------+-------------+---------+
| LedMatrix type            | min/avg/max | samples |
|---------------------------+-------------+---------|
| direct                    |  31/ 31/ 36 | 1200    |
| partial_sw_spi            |  55/ 55/ 60 | 1200    |
| partial_hw_spi            |  26/ 27/ 31 | 1200    |
| full_sw_spi               |  88/ 89/ 94 | 1200    |
| full_hw_spi               |  22/ 23/ 27 | 1200    |
+---------------------------+-------------+---------+

```

## STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
Sizes of Objects:
sizeof(Hardware): 4
sizeof(SwSpiAdapter): 8
sizeof(HwSpiAdapter): 8
sizeof(LedMatrixDirect): 24
sizeof(LedMatrixPartialSpi): 24
sizeof(LedMatrixFullSpi): 16
sizeof(LedDisplay): 8
sizeof(SegmentDisplay<4, 1>): 40
sizeof(HexWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+---------------------------+-------------+---------+
| LedMatrix type            | min/avg/max | samples |
|---------------------------+-------------+---------|
| direct                    |  17/ 17/ 22 | 1200    |
| partial_sw_spi            |  31/ 32/ 37 | 1200    |
| partial_hw_spi            |  42/ 42/ 47 | 1200    |
| full_sw_spi               |  52/ 52/ 57 | 1200    |
| full_hw_spi               |  40/ 40/ 45 | 1200    |
+---------------------------+-------------+---------+

```

## ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(Hardware): 4
sizeof(SwSpiAdapter): 8
sizeof(HwSpiAdapter): 8
sizeof(LedMatrixDirect): 24
sizeof(LedMatrixPartialSpi): 24
sizeof(LedMatrixFullSpi): 16
sizeof(LedDisplay): 8
sizeof(SegmentDisplay<4, 1>): 40
sizeof(HexWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+---------------------------+-------------+---------+
| LedMatrix type            | min/avg/max | samples |
|---------------------------+-------------+---------|
| direct                    |  15/ 16/ 36 | 1200    |
| partial_sw_spi            |  30/ 31/ 34 | 1200    |
| partial_hw_spi            |  13/ 13/ 14 | 1200    |
| full_sw_spi               |  50/ 50/ 65 | 1200    |
| full_hw_spi               |  12/ 12/ 16 | 1200    |
+---------------------------+-------------+---------+

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
Sizes of Objects:
sizeof(Hardware): 4
sizeof(SwSpiAdapter): 8
sizeof(HwSpiAdapter): 8
sizeof(LedMatrixDirect): 24
sizeof(LedMatrixPartialSpi): 24
sizeof(LedMatrixFullSpi): 16
sizeof(LedDisplay): 8
sizeof(SegmentDisplay<4, 1>): 40
sizeof(HexWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+---------------------------+-------------+---------+
| LedMatrix type            | min/avg/max | samples |
|---------------------------+-------------+---------|
| direct                    |   3/  3/ 12 | 1200    |
| partial_sw_spi            |   4/  5/ 14 | 1200    |
| partial_hw_spi            |   9/  9/ 18 | 1200    |
| full_sw_spi               |   7/  7/ 16 | 1200    |
| full_hw_spi               |   9/  9/ 18 | 1200    |
+---------------------------+-------------+---------+

```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(Hardware): 4
sizeof(SwSpiAdapter): 8
sizeof(HwSpiAdapter): 8
sizeof(LedMatrixDirect): 24
sizeof(LedMatrixPartialSpi): 24
sizeof(LedMatrixFullSpi): 16
sizeof(LedDisplay): 8
sizeof(SegmentDisplay<4, 1>): 40
sizeof(HexWriter): 4
sizeof(ClockWriter): 8
sizeof(CharWriter): 4
sizeof(StringWriter): 4

CPU:
+---------------------------+-------------+---------+
| LedMatrix type            | min/avg/max | samples |
|---------------------------+-------------+---------|
| direct                    |   8/  8/ 13 | 1200    |
| partial_sw_spi            |  12/ 12/ 16 | 1200    |
| partial_hw_spi            |   6/  6/ 10 | 1200    |
| full_sw_spi               |  17/ 17/ 22 | 1200    |
| full_hw_spi               |   4/  4/  8 | 1200    |
+---------------------------+-------------+---------+

```

