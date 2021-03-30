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
| direct                          |   3324/  285 |  2868/  274 |
| split_sw_spi                    |   3482/  293 |  3026/  282 |
| split_hw_spi                    |   3704/  296 |  3248/  285 |
| merged_sw_spi                   |   3222/  282 |  2766/  271 |
| merged_hw_spi                   |   3444/  285 |  2988/  274 |
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
| direct                          |   5492/  250 |  2020/   99 |
| split_sw_spi                    |   5652/  258 |  2180/  107 |
| split_hw_spi                    |   5874/  261 |  2402/  110 |
| merged_sw_spi                   |   5392/  247 |  1920/   96 |
| merged_hw_spi                   |   5614/  250 |  2142/   99 |
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
| direct                          |  11408/    0 |  1344/    0 |
| split_sw_spi                    |  11568/    0 |  1504/    0 |
| split_hw_spi                    |  12152/    0 |  2088/    0 |
| merged_sw_spi                   |  11408/    0 |  1344/    0 |
| merged_hw_spi                   |  11984/    0 |  1920/    0 |
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
| direct                          |  22076/ 4056 |  2940/  268 |
| split_sw_spi                    |  22256/ 4064 |  3120/  276 |
| split_hw_spi                    |  24460/ 4064 |  5324/  276 |
| merged_sw_spi                   |  22104/ 4052 |  2968/  264 |
| merged_hw_spi                   |  24308/ 4052 |  5172/  264 |
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
| direct                          | 261736/27012 |  5036/  228 |
| split_sw_spi                    | 261984/27020 |  5284/  236 |
| split_hw_spi                    | 263328/27028 |  6628/  244 |
| merged_sw_spi                   | 261708/27000 |  5008/  216 |
| merged_hw_spi                   | 263052/27008 |  6352/  224 |
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
| direct                          | 206700/13588 |  8970/  488 |
| split_sw_spi                    | 206972/13596 |  9242/  496 |
| split_hw_spi                    | 210576/13644 | 12846/  544 |
| merged_sw_spi                   | 206692/13588 |  8962/  488 |
| merged_hw_spi                   | 210296/13636 | 12566/  536 |
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
| direct                          |  13140/ 4244 |  5516/ 1196 |
| split_sw_spi                    |  13124/ 4252 |  5500/ 1204 |
| split_hw_spi                    |  14264/ 4308 |  6640/ 1260 |
| merged_sw_spi                   |  12660/ 4240 |  5036/ 1192 |
| merged_hw_spi                   |  13800/ 4296 |  6176/ 1248 |
+--------------------------------------------------------------+

```

