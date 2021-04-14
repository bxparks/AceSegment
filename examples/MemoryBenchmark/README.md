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

**v0.5**

* Slight increase in memory usage (20-30 bytes) on some processors (AVR,
  ESP8266, ESP8266), but slight decrease on others (STM32, Teensy), I think the
  changes are due to some removal/addition of some methods in `LedDisplay`.
* Add memory usage for `Tm1637Display`. Seems to consume something in between
  similar to the `ScanningDisplay` w/ SW SPI and `ScanningDisplay` with HW SPI.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program that includes the resources needed to perform a
`ScanningDisplay::renderFieldWhenReady()`. This includes:

* `Hardware` (which is opimized away by the compiler)
* `SwSpiAdapter` or `HwSpiAdapter`
* `LedMatrixXxx`
* `ScanningDisplay`
* `NumberWriter`
* `ClockWriter`
* `CharWriter`
* `StringWriter`

The `StubDisplay` is a dummy subclass of `LedDisplay` needed to create the
various Writers. To get a better flash consumption of the Writer classes, this
stub class should be subtracted from the numbers below. (Ideally, the
`generate_table.awk` script should do this automatically, but I'm trying to keep
that script more general to avoid maintenance overhead when it is copied into
other `MemoryBenchmark` programs.)

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
| ScanningDisplay(direct)         |   1700/   76 |  1244/   65 |
| ScanningDisplay(single_sw_spi)  |   1724/   70 |  1268/   59 |
| ScanningDisplay(single_hw_spi)  |   1786/   71 |  1330/   60 |
| ScanningDisplay(dual_sw_spi)    |   1626/   61 |  1170/   50 |
| ScanningDisplay(dual_hw_spi)    |   1700/   62 |  1244/   51 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |   1460/  104 |  1004/   93 |
| ScanningDisplay(single_sw_fast) |   1616/   68 |  1160/   57 |
| ScanningDisplay(dual_sw_fast)   |   1226/   59 |   770/   48 |
|---------------------------------+--------------+-------------|
| Tm1637Display                   |   1672/   41 |  1216/   30 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |    538/   11 |    82/    0 |
| NumberWriter+Stub               |    692/   32 |   236/   21 |
| ClockWriter+Stub                |    792/   33 |   336/   22 |
| CharWriter+Stub                 |    792/   32 |   336/   21 |
| StringWriter+Stub               |    940/   40 |   484/   29 |
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
| ScanningDisplay(direct)         |   4694/  216 |  1222/   65 |
| ScanningDisplay(single_sw_spi)  |   4720/  210 |  1248/   59 |
| ScanningDisplay(single_hw_spi)  |   4782/  211 |  1310/   60 |
| ScanningDisplay(dual_sw_spi)    |   4622/  201 |  1150/   50 |
| ScanningDisplay(dual_hw_spi)    |   4696/  202 |  1224/   51 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |   4340/  244 |   868/   93 |
| ScanningDisplay(single_sw_fast) |   4612/  208 |  1140/   57 |
| ScanningDisplay(dual_sw_fast)   |   4106/  199 |   634/   48 |
|---------------------------------+--------------+-------------|
| Tm1637Display                   |   4742/  181 |  1270/   30 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |   3552/  151 |    80/    0 |
| NumberWriter+Stub               |   3648/  172 |   176/   21 |
| ClockWriter+Stub                |   3748/  173 |   276/   22 |
| CharWriter+Stub                 |   3748/  172 |   276/   21 |
| StringWriter+Stub               |   3896/  180 |   424/   29 |
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
| ScanningDisplay(direct)         |  10928/    0 |   864/    0 |
| ScanningDisplay(single_sw_spi)  |  10984/    0 |   920/    0 |
| ScanningDisplay(single_hw_spi)  |  11432/    0 |  1368/    0 |
| ScanningDisplay(dual_sw_spi)    |  10872/    0 |   808/    0 |
| ScanningDisplay(dual_hw_spi)    |  11392/    0 |  1328/    0 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display                   |  10952/    0 |   888/    0 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |  10336/    0 |   272/    0 |
| NumberWriter+Stub               |  10664/    0 |   600/    0 |
| ClockWriter+Stub                |  10448/    0 |   384/    0 |
| CharWriter+Stub                 |  10520/    0 |   456/    0 |
| StringWriter+Stub               |  10664/    0 |   600/    0 |
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
| ScanningDisplay(direct)         |  21632/ 4016 |  2496/  228 |
| ScanningDisplay(single_sw_spi)  |  21696/ 4020 |  2560/  232 |
| ScanningDisplay(single_hw_spi)  |  23436/ 4020 |  4300/  232 |
| ScanningDisplay(dual_sw_spi)    |  21596/ 4012 |  2460/  224 |
| ScanningDisplay(dual_hw_spi)    |  23380/ 4012 |  4244/  224 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display                   |  21756/ 3976 |  2620/  188 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |  19336/ 3948 |   200/  160 |
| NumberWriter+Stub               |  19612/ 3952 |   476/  164 |
| ClockWriter+Stub                |  19464/ 3956 |   328/  168 |
| CharWriter+Stub                 |  19508/ 3952 |   372/  164 |
| StringWriter+Stub               |  19652/ 3956 |   516/  168 |
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
| ScanningDisplay(direct)         | 257972/26860 |  1272/   76 |
| ScanningDisplay(single_sw_spi)  | 258044/26868 |  1344/   84 |
| ScanningDisplay(single_hw_spi)  | 259148/26876 |  2448/   92 |
| ScanningDisplay(dual_sw_spi)    | 257928/26848 |  1228/   64 |
| ScanningDisplay(dual_hw_spi)    | 259128/26856 |  2428/   72 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display                   | 258200/26816 |  1500/   32 |
|---------------------------------+--------------+-------------|
| StubDisplay                     | 256884/26792 |   184/    8 |
| NumberWriter+Stub               | 257380/26792 |   680/    8 |
| ClockWriter+Stub                | 257140/26800 |   440/   16 |
| CharWriter+Stub                 | 257092/26792 |   392/    8 |
| StringWriter+Stub               | 257292/26816 |   592/   32 |
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
| ScanningDisplay(direct)         | 200660/13404 |  2930/  304 |
| ScanningDisplay(single_sw_spi)  | 200720/13404 |  2990/  304 |
| ScanningDisplay(single_hw_spi)  | 203012/13452 |  5282/  352 |
| ScanningDisplay(dual_sw_spi)    | 200568/13396 |  2838/  296 |
| ScanningDisplay(dual_hw_spi)    | 202936/13444 |  5206/  344 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display                   | 200940/13364 |  3210/  264 |
|---------------------------------+--------------+-------------|
| StubDisplay                     | 199298/13176 |  1568/   76 |
| NumberWriter+Stub               | 199674/13184 |  1944/   84 |
| ClockWriter+Stub                | 199538/13184 |  1808/   84 |
| CharWriter+Stub                 | 199506/13184 |  1776/   84 |
| StringWriter+Stub               | 199670/13184 |  1940/   84 |
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
| ScanningDisplay(direct)         |  12004/ 4204 |  4380/ 1156 |
| ScanningDisplay(single_sw_spi)  |  12052/ 4208 |  4428/ 1160 |
| ScanningDisplay(single_hw_spi)  |  13128/ 4264 |  5504/ 1216 |
| ScanningDisplay(dual_sw_spi)    |  11980/ 4200 |  4356/ 1152 |
| ScanningDisplay(dual_hw_spi)    |  13040/ 4256 |  5416/ 1208 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display                   |  12672/ 4164 |  5048/ 1116 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |  10924/ 4156 |  3300/ 1108 |
| NumberWriter+Stub               |  11328/ 4160 |  3704/ 1112 |
| ClockWriter+Stub                |  11052/ 4164 |  3428/ 1116 |
| CharWriter+Stub                 |  11096/ 4160 |  3472/ 1112 |
| StringWriter+Stub               |  11264/ 4164 |  3640/ 1116 |
+--------------------------------------------------------------+

```

