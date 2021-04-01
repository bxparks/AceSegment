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
* Reduce flash by 300-350 bytes on AVR (~150 on SAMD, 150-500 bytes on STM32,
  ~250 bytes on ESP8266, 300-600 bytes on ESP32) by templatizing LedMatrix
  and SegmentDisplay on `NUM_DIGITS`, `NUM_SUBFIELDS`, `Hardware` class,
  `SwSpiAdapter` and `HwSpiAdapter`.
* Reduce flash by flattening the `LedMatrix` hierarchy into templatized
  classes, and removing virtual methods. Saves 250-300 bytes on AVR, 150-200 on
  SAMD, 150-300 on STM32, 200-300 on ESP8266, 300-1300 bytes on ESP32, 800-1300
  bytes on Teensy 3.2.
* Reduce flash by 250-400 bytes on AVR by providing ability to use
  `digitalWriteFast()` (https://github.com/NicksonYap/digitalWriteFast) using
  the `fast/LedMatrixDirectFast.h` and `fast/SwSpiAdapterFast.h` classes.
* Total flash size saved is around 2kB for AVR, from (4 to 4.4) kB to (2 to 2.5)
  kB.

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
| direct                          |   2566/  252 |  2110/  241 |
| partial_sw_spi                  |   2572/  246 |  2116/  235 |
| partial_hw_spi                  |   2634/  247 |  2178/  236 |
| full_sw_spi                     |   2448/  237 |  1992/  226 |
| full_hw_spi                     |   2522/  238 |  2066/  227 |
| direct_fast                     |   2296/  280 |  1840/  269 |
| partial_sw_fast                 |   2464/  244 |  2008/  233 |
| full_sw_fast                    |   2056/  235 |  1600/  224 |
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
| direct                          |   4732/  217 |  1260/   66 |
| partial_sw_spi                  |   4740/  211 |  1268/   60 |
| partial_hw_spi                  |   4802/  212 |  1330/   61 |
| full_sw_spi                     |   4616/  202 |  1144/   51 |
| full_hw_spi                     |   4690/  203 |  1218/   52 |
| direct_fast                     |   4348/  245 |   876/   94 |
| partial_sw_fast                 |   4632/  209 |  1160/   58 |
| full_sw_fast                    |   4108/  200 |   636/   49 |
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
| direct                          |  10928/    0 |   864/    0 |
| partial_sw_spi                  |  10984/    0 |   920/    0 |
| partial_hw_spi                  |  11432/    0 |  1368/    0 |
| full_sw_spi                     |  10872/    0 |   808/    0 |
| full_hw_spi                     |  11392/    0 |  1328/    0 |
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
| direct                          |  21648/ 4020 |  2512/  232 |
| partial_sw_spi                  |  21716/ 4024 |  2580/  236 |
| partial_hw_spi                  |  23456/ 4024 |  4320/  236 |
| full_sw_spi                     |  21616/ 4016 |  2480/  228 |
| full_hw_spi                     |  23404/ 4016 |  4268/  228 |
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
| direct                          | 260920/26972 |  4220/  188 |
| partial_sw_spi                  | 261008/26980 |  4308/  196 |
| partial_hw_spi                  | 262112/26988 |  5412/  204 |
| full_sw_spi                     | 260876/26960 |  4176/  176 |
| full_hw_spi                     | 262060/26968 |  5360/  184 |
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
| direct                          | 205976/13556 |  8246/  456 |
| partial_sw_spi                  | 206016/13556 |  8286/  456 |
| partial_hw_spi                  | 208308/13604 | 10578/  504 |
| full_sw_spi                     | 205884/13548 |  8154/  448 |
| full_hw_spi                     | 208252/13596 | 10522/  496 |
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
| direct                          |  12216/ 4208 |  4592/ 1160 |
| partial_sw_spi                  |  12248/ 4212 |  4624/ 1164 |
| partial_hw_spi                  |  13508/ 4268 |  5884/ 1220 |
| full_sw_spi                     |  12172/ 4204 |  4548/ 1156 |
| full_hw_spi                     |  13368/ 4260 |  5744/ 1212 |
+--------------------------------------------------------------+

```

