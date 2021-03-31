# Memory Benchmark

The `MemoryBenchmark.ino` compiles example code snippets using the various
CRC algorithms. The `FEATURE` macro flag controls which feature is
compiled. The `collect.sh` edits this `FEATURE` flag programmatically, then runs
the Arduino IDE compiler on the program, and extracts the flash and static
memory usage into a text file (e.g. `nano.txt`).

The numbers shown below should be considered to be rough estimates. It is often
difficult to separate out the code size of the library from the overhead imposed
by the runtime environment of the processor. For example, it often seems like
the ESP8266 allocates flash memory in blocks of a certain quantity, so the
calculated flash size can jump around in unexpected ways.

**Version**: AceSegment v0.4

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules for several microcontrollers:

```
$ make benchmarks
```
produces the following files:

```
nano.txt
micro.txt
samd.txt
stm32.txt
esp8266.txt
esp32.txt
teensy32.txt
```

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

## Algorithms

* 0 `baseline`: program does (almost) nothing
* 1 `direct`: segment and digit pins are wired directly
* 2 `split_sw_spi`: segment pins wired directly, digit pins through SW SPI
* 3 `split_hw_spi`: segment pins wired directly, digit pins through HW SPI
* 4 `merged_sw_spi`: segment and digit pins both controlled through SW SPI
* 5 `merged_hw_spi`: segment and digit pins both controlled through HW SPI

## Library Size Changes

**v0.3**

* Initial MemoryBenchmark using the old v0.3 implementation from 2018,
before substantional refactoring in 2021.

**v0.4**

* Reduce flash size from 4.0-4.4kB by about 200-500 bytes on AVR by
  simplifying `LedMatrix` class hierarchy by extracting out the `SpiAdapter`
  class to handle both hardware and software SPI, instead of calling
  `shiftOut()` directly.
* Reduce flash size from 3.8-4.2kB down 800-1000 bytes on AVR by
  simplifying the `Driver` class hierarchy into a single `Renderer` class, by
  making the `LedMatrix` class into a better abstraction and unifying the API
  into a single `draw(group, elementPattern)` method.
* Reduce flash by 20-50 bytes on AVR by merging `Renderer` into
  `SegmentDisplay`.
* Reduce flash by 100-200 bytes on AVR, SAMD21, STM32 and ESP8266 by
  templatizing the `SegmentDisplay` on `NUM_DIGITS` and `NUM_SUBFIELDS`, and
  merging `patterns` and `brightnesses` arrays directly into `SegmentDisplay`.
  Flash usage actually goes up by ~40 bytes on Teensy3.2, but it has enough
  flash memory.

## Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |   3088/  285 |  2632/  274 |
| split_sw_spi                    |   3246/  293 |  2790/  282 |
| split_hw_spi                    |   3468/  296 |  3012/  285 |
| merged_sw_spi                   |   2948/  282 |  2492/  271 |
| merged_hw_spi                   |   3170/  285 |  2714/  274 |
+--------------------------------------------------------------+

```

## Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   3472/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |   5258/  250 |  1786/   99 |
| split_sw_spi                    |   5418/  258 |  1946/  107 |
| split_hw_spi                    |   5640/  261 |  2168/  110 |
| merged_sw_spi                   |   5120/  247 |  1648/   96 |
| merged_hw_spi                   |   5342/  250 |  1870/   99 |
+--------------------------------------------------------------+

```

## SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  10064/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |  11240/    0 |  1176/    0 |
| split_sw_spi                    |  11400/    0 |  1336/    0 |
| split_hw_spi                    |  11984/    0 |  1920/    0 |
| merged_sw_spi                   |  11192/    0 |  1128/    0 |
| merged_hw_spi                   |  11776/    0 |  1712/    0 |
+--------------------------------------------------------------+

```

## STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  19136/ 3788 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |  21904/ 4024 |  2768/  236 |
| split_sw_spi                    |  22088/ 4032 |  2952/  244 |
| split_hw_spi                    |  24292/ 4032 |  5156/  244 |
| merged_sw_spi                   |  21904/ 4024 |  2768/  236 |
| merged_hw_spi                   |  24108/ 4024 |  4972/  236 |
+--------------------------------------------------------------+

```

## ESP8266

* NodeMCU 1.0, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 256700/26784 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          | 261448/26980 |  4748/  196 |
| split_sw_spi                    | 261696/26988 |  4996/  204 |
| split_hw_spi                    | 263040/26996 |  6340/  212 |
| merged_sw_spi                   | 261388/26968 |  4688/  184 |
| merged_hw_spi                   | 262732/26976 |  6032/  192 |
+--------------------------------------------------------------+

```

## ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 197730/13100 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          | 206636/13556 |  8906/  456 |
| split_sw_spi                    | 206904/13564 |  9174/  464 |
| split_hw_spi                    | 210508/13612 | 12778/  512 |
| merged_sw_spi                   | 206576/13556 |  8846/  456 |
| merged_hw_spi                   | 210180/13604 | 12450/  504 |
+--------------------------------------------------------------+

```

## Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   7624/ 3048 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |  13152/ 4212 |  5528/ 1164 |
| split_sw_spi                    |  13124/ 4220 |  5500/ 1172 |
| split_hw_spi                    |  14268/ 4276 |  6644/ 1228 |
| merged_sw_spi                   |  12580/ 4212 |  4956/ 1164 |
| merged_hw_spi                   |  13720/ 4268 |  6096/ 1220 |
+--------------------------------------------------------------+

```

