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

**Version**: AceSegment v0.3

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

v0.3: Initial MemoryBenchmark using the old v.0.3 implementation from 2018,
before substantional refactoring in 2021.

Reduce library size from 4.0-4.4kB by about 200-500 bytes on AVR by simplifying
`LedMatrix` class hierarchy by extracting out the `SpiAdapter` class to handle
both hardware and software SPI, instead of calling `shiftOut()` directly.

Reduce library size from 3.8-4.2kB down 800-1000 bytes on AVR by simplifying the
`Driver` class hierarchy into a single `Renderer` class, by making the
`LedMatrix` class into a better abstraction and unifying the API into a single
`draw(group, elementPattern)` method.

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
| direct                          |   3298/  283 |  2842/  272 |
| split_sw_spi                    |   3456/  291 |  3000/  280 |
| split_hw_spi                    |   3678/  294 |  3222/  283 |
| merged_sw_spi                   |   3178/  280 |  2722/  269 |
| merged_hw_spi                   |   3400/  283 |  2944/  272 |
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
| direct                          |   5466/  248 |  1994/   97 |
| split_sw_spi                    |   5626/  256 |  2154/  105 |
| split_hw_spi                    |   5848/  259 |  2376/  108 |
| merged_sw_spi                   |   5348/  245 |  1876/   94 |
| merged_hw_spi                   |   5570/  248 |  2098/   97 |
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
| direct                          |  11416/    0 |  1352/    0 |
| split_sw_spi                    |  11576/    0 |  1512/    0 |
| split_hw_spi                    |  12160/    0 |  2096/    0 |
| merged_sw_spi                   |  11376/    0 |  1312/    0 |
| merged_hw_spi                   |  11952/    0 |  1888/    0 |
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
| direct                          |  22076/ 4048 |  2940/  260 |
| split_sw_spi                    |  22256/ 4056 |  3120/  268 |
| split_hw_spi                    |  24460/ 4056 |  5324/  268 |
| merged_sw_spi                   |  22072/ 4048 |  2936/  260 |
| merged_hw_spi                   |  24276/ 4048 |  5140/  260 |
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
| direct                          | 261736/27004 |  5036/  220 |
| split_sw_spi                    | 261984/27012 |  5284/  228 |
| split_hw_spi                    | 263328/27020 |  6628/  236 |
| merged_sw_spi                   | 261676/26992 |  4976/  208 |
| merged_hw_spi                   | 263020/27000 |  6320/  216 |
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
| direct                          | 206916/13596 |  9186/  496 |
| split_sw_spi                    | 207184/13604 |  9454/  504 |
| split_hw_spi                    | 210788/13652 | 13058/  552 |
| merged_sw_spi                   | 206856/13596 |  9126/  496 |
| merged_hw_spi                   | 210460/13644 | 12730/  544 |
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
| direct                          |  13180/ 4236 |  5556/ 1188 |
| split_sw_spi                    |  13160/ 4244 |  5536/ 1196 |
| split_hw_spi                    |  14304/ 4300 |  6680/ 1252 |
| merged_sw_spi                   |  12608/ 4236 |  4984/ 1188 |
| merged_hw_spi                   |  13748/ 4292 |  6124/ 1244 |
+--------------------------------------------------------------+

```

