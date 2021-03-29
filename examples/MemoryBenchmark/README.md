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
| direct                          |   4284/  284 |  3828/  273 |
| split_sw_spi                    |   4424/  288 |  3968/  277 |
| split_hw_spi                    |   4646/  291 |  4190/  280 |
| merged_sw_spi                   |   4060/  282 |  3604/  271 |
| merged_hw_spi                   |   4282/  285 |  3826/  274 |
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
| direct                          |   6450/  249 |  2978/   98 |
| split_sw_spi                    |   6592/  253 |  3120/  102 |
| split_hw_spi                    |   6814/  256 |  3342/  105 |
| merged_sw_spi                   |   6228/  247 |  2756/   96 |
| merged_hw_spi                   |   6450/  250 |  2978/   99 |
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
| direct                          |  12200/    0 |  2136/    0 |
| split_sw_spi                    |  12360/    0 |  2296/    0 |
| split_hw_spi                    |  12936/    0 |  2872/    0 |
| merged_sw_spi                   |  12136/    0 |  2072/    0 |
| merged_hw_spi                   |  12720/    0 |  2656/    0 |
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
| direct                          |  22468/ 3980 |  3332/  192 |
| split_sw_spi                    |  22644/ 3980 |  3508/  192 |
| split_hw_spi                    |  24848/ 3980 |  5712/  192 |
| merged_sw_spi                   |  22440/ 3980 |  3304/  192 |
| merged_hw_spi                   |  24644/ 3980 |  5508/  192 |
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
| direct                          | 262468/26940 |  5768/  156 |
| split_sw_spi                    | 262716/26940 |  6016/  156 |
| split_hw_spi                    | 264060/26948 |  7360/  164 |
| merged_sw_spi                   | 262408/26928 |  5708/  144 |
| merged_hw_spi                   | 263752/26936 |  7052/  152 |
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
| direct                          | 208780/13524 | 11050/  424 |
| split_sw_spi                    | 209052/13524 | 11322/  424 |
| split_hw_spi                    | 212656/13572 | 14926/  472 |
| merged_sw_spi                   | 208720/13524 | 10990/  424 |
| merged_hw_spi                   | 212324/13572 | 14594/  472 |
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
| direct                          |  13424/ 4168 |  5800/ 1120 |
| split_sw_spi                    |  13472/ 4168 |  5848/ 1120 |
| split_hw_spi                    |  14604/ 4224 |  6980/ 1176 |
| merged_sw_spi                   |  12972/ 4168 |  5348/ 1120 |
| merged_hw_spi                   |  14104/ 4224 |  6480/ 1176 |
+--------------------------------------------------------------+

```

