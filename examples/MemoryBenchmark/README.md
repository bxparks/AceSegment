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
| direct                          |   4850/  301 |  4394/  290 |
| split_sw_spi                    |   4876/  293 |  4420/  282 |
| split_hw_spi                    |   4896/  293 |  4440/  282 |
| merged_sw_spi                   |   4508/  285 |  4052/  274 |
| merged_hw_spi                   |   4514/  285 |  4058/  274 |
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
| direct                          |   7018/  266 |  3546/  115 |
| split_sw_spi                    |   7044/  258 |  3572/  107 |
| split_hw_spi                    |   7064/  258 |  3592/  107 |
| merged_sw_spi                   |   6676/  250 |  3204/   99 |
| merged_hw_spi                   |   6682/  250 |  3210/   99 |
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
| direct                          |  13000/    0 |  2936/    0 |
| split_sw_spi                    |  13008/    0 |  2944/    0 |
| split_hw_spi                    |  13040/    0 |  2976/    0 |
| merged_sw_spi                   |  12816/    0 |  2752/    0 |
| merged_hw_spi                   |  12840/    0 |  2776/    0 |
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
| direct                          |  24924/ 3980 |  5788/  192 |
| split_sw_spi                    |  24920/ 3980 |  5784/  192 |
| split_hw_spi                    |  24952/ 3980 |  5816/  192 |
| merged_sw_spi                   |  24744/ 3980 |  5608/  192 |
| merged_hw_spi                   |  24764/ 3980 |  5628/  192 |
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
| direct                          | 264196/26948 |  7496/  164 |
| split_sw_spi                    | 264188/26948 |  7488/  164 |
| split_hw_spi                    | 264252/26948 |  7552/  164 |
| merged_sw_spi                   | 263928/26936 |  7228/  152 |
| merged_hw_spi                   | 263976/26936 |  7276/  152 |
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
| direct                          | 213348/13612 | 15618/  512 |
| split_sw_spi                    | 213344/13612 | 15614/  512 |
| split_hw_spi                    | 213380/13612 | 15650/  512 |
| merged_sw_spi                   | 213104/13612 | 15374/  512 |
| merged_hw_spi                   | 213124/13612 | 15394/  512 |
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
| direct                          |  14804/ 4224 |  7180/ 1176 |
| split_sw_spi                    |  14856/ 4224 |  7232/ 1176 |
| split_hw_spi                    |  15152/ 4224 |  7528/ 1176 |
| merged_sw_spi                   |  14488/ 4224 |  6864/ 1176 |
| merged_hw_spi                   |  14756/ 4224 |  7132/ 1176 |
+--------------------------------------------------------------+

```

