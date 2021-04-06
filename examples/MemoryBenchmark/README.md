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
* Reduce flash size by 828 bytes on AVR, 3kB on ESP8266, 5kB on ESP32 in commit
  c5da272 which simplified the test classes under `src/ace_segment/testing/` so
  that they no longer inherit from `TestOnce` classes in the `AUnit` library.
  Apparently, just making a reference to AUnit causes the `Serial` instance of
  the `HardwareSerial` class to be pulled in. The compiler/linker is not able to
  detect that it is actually never used, so it keeps around the code for the
  HardwareSerial class. (I will make a fix to AUnit so that the `HardwareSerial`
  will not be pulled in by other libraries in the future.)
* Reduce flash size by ~130 bytes on AVR and 70-80 bytes on 32-bit processors
  by removing the pointer to `TimingStats` from `SegmentDisplay`. The pointer
  causes the code for the `TimingStats` class to be pulled in, even if it is not
  used.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program that includes the resources needed to perform a
`SegmentDisplay::renderFieldWhenReady()`. This includes:

* `Hardware` (which is opimized away by the compiler)
* `SwSpiAdapter` or `HwSpiAdapter`
* `LedMatrixXxx`
* `SegmentDisplay`

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |   1596/   74 |  1140/   63 |
| single_sw_spi                   |   1602/   68 |  1146/   57 |
| single_hw_spi                   |   1664/   69 |  1208/   58 |
| dual_sw_spi                     |   1486/   59 |  1030/   48 |
| dual_hw_spi                     |   1560/   60 |  1104/   49 |
|---------------------------------+--------------+-------------|
| direct_fast                     |   1332/  102 |   876/   91 |
| single_sw_fast                  |   1494/   66 |  1038/   55 |
| dual_sw_fast                    |   1094/   57 |   638/   46 |
+--------------------------------------------------------------+

```

### Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   3472/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |   4590/  214 |  1118/   63 |
| single_sw_spi                   |   4598/  208 |  1126/   57 |
| single_hw_spi                   |   4660/  209 |  1188/   58 |
| dual_sw_spi                     |   4482/  199 |  1010/   48 |
| dual_hw_spi                     |   4556/  200 |  1084/   49 |
|---------------------------------+--------------+-------------|
| direct_fast                     |   4212/  242 |   740/   91 |
| single_sw_fast                  |   4490/  206 |  1018/   55 |
| dual_sw_fast                    |   3974/  197 |   502/   46 |
+--------------------------------------------------------------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  10064/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |  10848/    0 |   784/    0 |
| single_sw_spi                   |  10904/    0 |   840/    0 |
| single_hw_spi                   |  11344/    0 |  1280/    0 |
| dual_sw_spi                     |  10784/    0 |   720/    0 |
| dual_hw_spi                     |  11304/    0 |  1240/    0 |
+--------------------------------------------------------------+

```

### STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  19136/ 3788 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          |  21564/ 4016 |  2428/  228 |
| single_sw_spi                   |  21628/ 4020 |  2492/  232 |
| single_hw_spi                   |  23368/ 4020 |  4232/  232 |
| dual_sw_spi                     |  21524/ 4012 |  2388/  224 |
| dual_hw_spi                     |  23308/ 4012 |  4172/  224 |
+--------------------------------------------------------------+

```

### ESP8266

* NodeMCU 1.0, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 256700/26784 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          | 257828/26860 |  1128/   76 |
| single_sw_spi                   | 257900/26868 |  1200/   84 |
| single_hw_spi                   | 259004/26876 |  2304/   92 |
| dual_sw_spi                     | 257768/26848 |  1068/   64 |
| dual_hw_spi                     | 258968/26856 |  2268/   72 |
+--------------------------------------------------------------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 197730/13100 |     0/    0 |
|---------------------------------+--------------+-------------|
| direct                          | 200640/13412 |  2910/  312 |
| single_sw_spi                   | 200688/13412 |  2958/  312 |
| single_hw_spi                   | 202980/13460 |  5250/  360 |
| dual_sw_spi                     | 200556/13404 |  2826/  304 |
| dual_hw_spi                     | 202920/13452 |  5190/  352 |
+--------------------------------------------------------------+

```

### Teensy 3.2

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
| direct                          |  12016/ 4204 |  4392/ 1156 |
| single_sw_spi                   |  12068/ 4208 |  4444/ 1160 |
| single_hw_spi                   |  13308/ 4264 |  5684/ 1216 |
| dual_sw_spi                     |  11992/ 4200 |  4368/ 1152 |
| dual_hw_spi                     |  13196/ 4256 |  5572/ 1208 |
+--------------------------------------------------------------+

```

