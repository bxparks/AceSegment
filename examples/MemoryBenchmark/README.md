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
  `ScanningDisplay`.
* Reduce flash by 100-200 bytes on AVR, SAMD21, STM32 and ESP8266 by
  templatizing the `ScanningDisplay` on `NUM_DIGITS` and `NUM_SUBFIELDS`, and
  merging `patterns` and `brightnesses` arrays directly into `ScanningDisplay`.
  Flash usage actually goes up by ~40 bytes on Teensy3.2, but it has enough
  flash memory.
* Reduce flash by 300-350 bytes on AVR (~150 on SAMD, 150-500 bytes on STM32,
  ~250 bytes on ESP8266, 300-600 bytes on ESP32) by templatizing LedMatrix
  and ScanningDisplay on `NUM_DIGITS`, `NUM_SUBFIELDS`, `Hardware` class,
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
  by removing the pointer to `TimingStats` from `ScanningDisplay`. The pointer
  causes the code for the `TimingStats` class to be pulled in, even if it is not
  used.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program that includes the resources needed to perform a
`ScanningDisplay::renderFieldWhenReady()`. This includes:

* `Hardware` (which is opimized away by the compiler)
* `SwSpiAdapter` or `HwSpiAdapter`
* `LedMatrixXxx`
* `ScanningDisplay`

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
| direct                          |   1674/   78 |  1218/   67 |
| single_sw_spi                   |   1680/   72 |  1224/   61 |
| single_hw_spi                   |   1742/   73 |  1286/   62 |
| dual_sw_spi                     |   1564/   63 |  1108/   52 |
| dual_hw_spi                     |   1638/   64 |  1182/   53 |
|---------------------------------+--------------+-------------|
| direct_fast                     |   1410/  106 |   954/   95 |
| single_sw_fast                  |   1572/   70 |  1116/   59 |
| dual_sw_fast                    |   1172/   61 |   716/   50 |
|---------------------------------+--------------+-------------|
| NumberWriter                    |    696/   34 |   240/   23 |
| ClockWriter                     |    804/   35 |   348/   24 |
| CharWriter                      |    778/   34 |   322/   23 |
| StringWriter                    |    932/   48 |   476/   37 |
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
| direct                          |   4668/  218 |  1196/   67 |
| single_sw_spi                   |   4676/  212 |  1204/   61 |
| single_hw_spi                   |   4738/  213 |  1266/   62 |
| dual_sw_spi                     |   4560/  203 |  1088/   52 |
| dual_hw_spi                     |   4634/  204 |  1162/   53 |
|---------------------------------+--------------+-------------|
| direct_fast                     |   4290/  246 |   818/   95 |
| single_sw_fast                  |   4568/  210 |  1096/   59 |
| dual_sw_fast                    |   4052/  201 |   580/   50 |
|---------------------------------+--------------+-------------|
| NumberWriter                    |   3652/  174 |   180/   23 |
| ClockWriter                     |   3760/  175 |   288/   24 |
| CharWriter                      |   3734/  174 |   262/   23 |
| StringWriter                    |   3888/  188 |   416/   37 |
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
| direct                          |  10912/    0 |   848/    0 |
| single_sw_spi                   |  10968/    0 |   904/    0 |
| single_hw_spi                   |  11416/    0 |  1352/    0 |
| dual_sw_spi                     |  10856/    0 |   792/    0 |
| dual_hw_spi                     |  11376/    0 |  1312/    0 |
|---------------------------------+--------------+-------------|
| direct_fast                     |     -1/   -1 |    -1/   -1 |
| single_sw_fast                  |     -1/   -1 |    -1/   -1 |
| dual_sw_fast                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| NumberWriter                    |  10512/    0 |   448/    0 |
| ClockWriter                     |  10592/    0 |   528/    0 |
| CharWriter                      |  10536/    0 |   472/    0 |
| StringWriter                    |  10696/    0 |   632/    0 |
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
| direct                          |  21644/ 4016 |  2508/  228 |
| single_sw_spi                   |  21708/ 4020 |  2572/  232 |
| single_hw_spi                   |  23448/ 4020 |  4312/  232 |
| dual_sw_spi                     |  21604/ 4012 |  2468/  224 |
| dual_hw_spi                     |  23388/ 4012 |  4252/  224 |
|---------------------------------+--------------+-------------|
| direct_fast                     |     -1/   -1 |    -1/   -1 |
| single_sw_fast                  |     -1/   -1 |    -1/   -1 |
| dual_sw_fast                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| NumberWriter                    |  19504/ 3952 |   368/  164 |
| ClockWriter                     |  19560/ 3956 |   424/  168 |
| CharWriter                      |  19524/ 3952 |   388/  164 |
| StringWriter                    |  19672/ 3956 |   536/  168 |
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
| direct                          | 257924/26860 |  1224/   76 |
| single_sw_spi                   | 258012/26868 |  1312/   84 |
| single_hw_spi                   | 259116/26876 |  2416/   92 |
| dual_sw_spi                     | 257864/26848 |  1164/   64 |
| dual_hw_spi                     | 259064/26856 |  2364/   72 |
|---------------------------------+--------------+-------------|
| direct_fast                     |     -1/   -1 |    -1/   -1 |
| single_sw_fast                  |     -1/   -1 |    -1/   -1 |
| dual_sw_fast                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| NumberWriter                    | 257124/26792 |   424/    8 |
| ClockWriter                     | 257252/26800 |   552/   16 |
| CharWriter                      | 257108/26792 |   408/    8 |
| StringWriter                    | 257312/26812 |   612/   28 |
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
| direct                          | 200820/13404 |  3090/  304 |
| single_sw_spi                   | 200880/13404 |  3150/  304 |
| single_hw_spi                   | 203172/13452 |  5442/  352 |
| dual_sw_spi                     | 200748/13396 |  3018/  296 |
| dual_hw_spi                     | 203112/13444 |  5382/  344 |
|---------------------------------+--------------+-------------|
| direct_fast                     |     -1/   -1 |    -1/   -1 |
| single_sw_fast                  |     -1/   -1 |    -1/   -1 |
| dual_sw_fast                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| NumberWriter                    | 199650/13184 |  1920/   84 |
| ClockWriter                     | 199726/13184 |  1996/   84 |
| CharWriter                      | 199650/13184 |  1920/   84 |
| StringWriter                    | 199798/13184 |  2068/   84 |
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
| direct                          |  12112/ 4204 |  4488/ 1156 |
| single_sw_spi                   |  12164/ 4208 |  4540/ 1160 |
| single_hw_spi                   |  13404/ 4264 |  5780/ 1216 |
| dual_sw_spi                     |  12088/ 4200 |  4464/ 1152 |
| dual_hw_spi                     |  13292/ 4256 |  5668/ 1208 |
|---------------------------------+--------------+-------------|
| direct_fast                     |     -1/   -1 |    -1/   -1 |
| single_sw_fast                  |     -1/   -1 |    -1/   -1 |
| dual_sw_fast                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| NumberWriter                    |  11132/ 4160 |  3508/ 1112 |
| ClockWriter                     |  11176/ 4164 |  3552/ 1116 |
| CharWriter                      |  11112/ 4160 |  3488/ 1112 |
| StringWriter                    |  11284/ 4164 |  3660/ 1116 |
+--------------------------------------------------------------+

```

