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

**Version**: AceSegment v0.4+

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
  simplifying `LedMatrix` class hierarchy by extracting out the `SpiInterface`
  class to handle both hardware and software SPI, instead of calling
  `shiftOut()` directly.
* Reduce flash size from 3.8-4.2kB down 800-1000 bytes on AVR by
  simplifying the `Driver` class hierarchy into a single `Renderer` class, by
  making the `LedMatrix` class into a better abstraction and unifying the API
  into a single `draw(group, elementPattern)` method.
* Reduce flash by 20-50 bytes on AVR by merging `Renderer` into
  `ScanningModule`.
* Reduce flash by 100-200 bytes on AVR, SAMD21, STM32 and ESP8266 by
  templatizing the `ScanningModule` on `NUM_DIGITS` and `NUM_SUBFIELDS`, and
  merging `patterns` and `brightnesses` arrays directly into `ScanningModule`.
  Flash usage actually goes up by ~40 bytes on Teensy3.2, but it has enough
  flash memory.
* Reduce flash by 300-350 bytes on AVR (~150 on SAMD, 150-500 bytes on STM32,
  ~250 bytes on ESP8266, 300-600 bytes on ESP32) by templatizing LedMatrix
  and ScanningModule on `NUM_DIGITS`, `NUM_SUBFIELDS`, `SwSpiInterface` and
  `HwSpiInterface`.
* Reduce flash by flattening the `LedMatrix` hierarchy into templatized
  classes, and removing virtual methods. Saves 250-300 bytes on AVR, 150-200 on
  SAMD, 150-300 on STM32, 200-300 on ESP8266, 300-1300 bytes on ESP32, 800-1300
  bytes on Teensy 3.2.
* Reduce flash by 250-400 bytes on AVR by providing ability to use
  `digitalWriteFast()` (https://github.com/NicksonYap/digitalWriteFast) using
  the `fast/LedMatrixDirectFast.h` and `fast/FastSwSpiInterface.h` classes.
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
  by removing the pointer to `TimingStats` from `ScanningModule`. The pointer
  causes the code for the `TimingStats` class to be pulled in, even if it is not
  used.

**v0.5**

* Slight increase in memory usage (20-30 bytes) on some processors (AVR,
  ESP8266, ESP8266), but slight decrease on others (STM32, Teensy), I think the
  changes are due to some removal/addition of some methods in `LedDisplay`.
* Add memory usage for `Tm1637Module`. Seems to consume something in between
  similar to the `ScanningModule` w/ SW SPI and `ScanningModule` with HW SPI.
* Add memory usage for `Tm1637Module` using `FastSwWireInterface` which uses
  `digitalWriteFast` library for AVR processors. Saves 662 - 776 bytes of flash
  on AVR processors compared to `Tm1637Module` using normal `SwWireInterface`.
* Save 150-200 bytes of flash on AVR processors by lifting all of the
  `LedDisplay::writePatternAt()` type of methods to `LedDisplay`, making them
  non-virtual, then funneling these methods through just 2 lower-level virtual
  methods: `setPatternAt()` and `getPatternAt()`. It also made the
  implementation of `Tm1637Module` position remapping easier.
* Extracting `LedModule` from `LedDisplay` saves 10-40 bytes on AVR for
  `ScanningModule` and `Tm1637Module`, but add about that many bytes for various
  Writer classes (probably because they have to go though one additional layer
  of indirect, through the `LedModule`). So overall, I think it's a wash.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program that includes the resources needed to perform a
`ScanningModule::renderFieldWhenReady()`. This includes:

* `ClockInterface`, `GpioInterface` (which are opimized away by the compiler)
* `SwSpiInterface` or `HwSpiInterface`
* `LedMatrixXxx`
* `ScanningModule`
* `NumberWriter`
* `ClockWriter`
* `TemperatureWriter`
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
| Scanning(Direct)                |   1498/   64 |  1042/   53 |
| Scanning(Single,SwSpi)          |   1520/   58 |  1064/   47 |
| Scanning(Single,HwSpi)          |   1582/   59 |  1126/   48 |
| Scanning(Dual,SwSpi)            |   1428/   51 |   972/   40 |
| Scanning(Dual,HwSpi)            |   1502/   52 |  1046/   41 |
| Scanning(DirectFast)            |   1258/   94 |   802/   83 |
| Scanning(Single,FastSwSpi)      |   1412/   56 |   956/   45 |
| Scanning(Dual,FastSwSpi)        |   1028/   49 |   572/   38 |
| Tm1637(Normal)                  |   1590/   39 |  1134/   28 |
| Tm1637(Fast)                    |    930/   36 |   474/   25 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |    578/   24 |   122/   13 |
| NumberWriter+Stub               |    682/   28 |   226/   17 |
| ClockWriter+Stub                |    796/   29 |   340/   18 |
| TemperatureWriter+Stub          |    764/   28 |   308/   17 |
| CharWriter+Stub                 |    758/   28 |   302/   17 |
| StringWriter+Stub               |    988/   36 |   532/   25 |
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
| Scanning(Direct)                |   4492/  204 |  1020/   53 |
| Scanning(Single,SwSpi)          |   4516/  198 |  1044/   47 |
| Scanning(Single,HwSpi)          |   4578/  199 |  1106/   48 |
| Scanning(Dual,SwSpi)            |   4424/  191 |   952/   40 |
| Scanning(Dual,HwSpi)            |   4498/  192 |  1026/   41 |
| Scanning(DirectFast)            |   4138/  234 |   666/   83 |
| Scanning(Single,FastSwSpi)      |   4408/  196 |   936/   45 |
| Scanning(Dual,FastSwSpi)        |   3908/  189 |   436/   38 |
| Tm1637(Normal)                  |   4660/  179 |  1188/   28 |
| Tm1637(Fast)                    |   3886/  176 |   414/   25 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |   3534/  164 |    62/   13 |
| NumberWriter+Stub               |   3638/  168 |   166/   17 |
| ClockWriter+Stub                |   3752/  169 |   280/   18 |
| TemperatureWriter+Stub          |   3720/  168 |   248/   17 |
| CharWriter+Stub                 |   3714/  168 |   242/   17 |
| StringWriter+Stub               |   3944/  176 |   472/   25 |
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
| Scanning(Direct)                |  10792/    0 |   728/    0 |
| Scanning(Single,SwSpi)          |  10848/    0 |   784/    0 |
| Scanning(Single,HwSpi)          |  11296/    0 |  1232/    0 |
| Scanning(Dual,SwSpi)            |  10736/    0 |   672/    0 |
| Scanning(Dual,HwSpi)            |  11256/    0 |  1192/    0 |
| Scanning(DirectFast)            |     -1/   -1 |    -1/   -1 |
| Scanning(Single,FastSwSpi)      |     -1/   -1 |    -1/   -1 |
| Scanning(Dual,FastSwSpi)        |     -1/   -1 |    -1/   -1 |
| Tm1637(Normal)                  |  10808/    0 |   744/    0 |
| Tm1637(Fast)                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  10336/    0 |   272/    0 |
| NumberWriter+Stub               |  10672/    0 |   608/    0 |
| ClockWriter+Stub                |  10480/    0 |   416/    0 |
| TemperatureWriter+Stub          |  10736/    0 |   672/    0 |
| CharWriter+Stub                 |  10520/    0 |   456/    0 |
| StringWriter+Stub               |  10704/    0 |   640/    0 |
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
| Scanning(Direct)                |  21528/ 4392 |  2392/  604 |
| Scanning(Single,SwSpi)          |  21592/ 4396 |  2456/  608 |
| Scanning(Single,HwSpi)          |  23340/ 4396 |  4204/  608 |
| Scanning(Dual,SwSpi)            |  21488/ 4392 |  2352/  604 |
| Scanning(Dual,HwSpi)            |  23272/ 4392 |  4136/  604 |
| Scanning(DirectFast)            |     -1/   -1 |    -1/   -1 |
| Scanning(Single,FastSwSpi)      |     -1/   -1 |    -1/   -1 |
| Scanning(Dual,FastSwSpi)        |     -1/   -1 |    -1/   -1 |
| Tm1637(Normal)                  |  21636/ 4372 |  2500/  584 |
| Tm1637(Fast)                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  19328/ 4340 |   192/  552 |
| NumberWriter+Stub               |  19616/ 4344 |   480/  556 |
| ClockWriter+Stub                |  19496/ 4348 |   360/  560 |
| TemperatureWriter+Stub          |  19712/ 4344 |   576/  556 |
| CharWriter+Stub                 |  19500/ 4344 |   364/  556 |
| StringWriter+Stub               |  19676/ 4348 |   540/  560 |
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
| Scanning(Direct)                | 257772/27260 |  1072/  476 |
| Scanning(Single,SwSpi)          | 257844/27244 |  1144/  460 |
| Scanning(Single,HwSpi)          | 258948/27252 |  2248/  468 |
| Scanning(Dual,SwSpi)            | 257728/27248 |  1028/  464 |
| Scanning(Dual,HwSpi)            | 258912/27256 |  2212/  472 |
| Scanning(DirectFast)            |     -1/   -1 |    -1/   -1 |
| Scanning(Single,FastSwSpi)      |     -1/   -1 |    -1/   -1 |
| Scanning(Dual,FastSwSpi)        |     -1/   -1 |    -1/   -1 |
| Tm1637(Normal)                  | 257936/27224 |  1236/  440 |
| Tm1637(Fast)                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           | 256876/27200 |   176/  416 |
| NumberWriter+Stub               | 257372/27200 |   672/  416 |
| ClockWriter+Stub                | 257164/27208 |   464/  424 |
| TemperatureWriter+Stub          | 257484/27200 |   784/  416 |
| CharWriter+Stub                 | 257084/27200 |   384/  416 |
| StringWriter+Stub               | 257316/27208 |   616/  424 |
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
| Scanning(Direct)                | 200496/13780 |  2766/  680 |
| Scanning(Single,SwSpi)          | 200552/13780 |  2822/  680 |
| Scanning(Single,HwSpi)          | 202844/13828 |  5114/  728 |
| Scanning(Dual,SwSpi)            | 200428/13780 |  2698/  680 |
| Scanning(Dual,HwSpi)            | 202796/13828 |  5066/  728 |
| Scanning(DirectFast)            |     -1/   -1 |    -1/   -1 |
| Scanning(Single,FastSwSpi)      |     -1/   -1 |    -1/   -1 |
| Scanning(Dual,FastSwSpi)        |     -1/   -1 |    -1/   -1 |
| Tm1637(Normal)                  | 200744/13756 |  3014/  656 |
| Tm1637(Fast)                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           | 199242/13568 |  1512/  468 |
| NumberWriter+Stub               | 199630/13576 |  1900/  476 |
| ClockWriter+Stub                | 199518/13576 |  1788/  476 |
| TemperatureWriter+Stub          | 199750/13576 |  2020/  476 |
| CharWriter+Stub                 | 199474/13576 |  1744/  476 |
| StringWriter+Stub               | 199650/13576 |  1920/  476 |
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
| Scanning(Direct)                |  11884/ 4584 |  4260/ 1536 |
| Scanning(Single,SwSpi)          |  11936/ 4588 |  4312/ 1540 |
| Scanning(Single,HwSpi)          |  13008/ 4644 |  5384/ 1596 |
| Scanning(Dual,SwSpi)            |  11840/ 4584 |  4216/ 1536 |
| Scanning(Dual,HwSpi)            |  12912/ 4640 |  5288/ 1592 |
| Scanning(DirectFast)            |     -1/   -1 |    -1/   -1 |
| Scanning(Single,FastSwSpi)      |     -1/   -1 |    -1/   -1 |
| Scanning(Dual,FastSwSpi)        |     -1/   -1 |    -1/   -1 |
| Tm1637(Normal)                  |  12576/ 4564 |  4952/ 1516 |
| Tm1637(Fast)                    |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  10924/ 4552 |  3300/ 1504 |
| NumberWriter+Stub               |  11400/ 4556 |  3776/ 1508 |
| ClockWriter+Stub                |  11112/ 4560 |  3488/ 1512 |
| TemperatureWriter+Stub          |  11556/ 4556 |  3932/ 1508 |
| CharWriter+Stub                 |  11100/ 4556 |  3476/ 1508 |
| StringWriter+Stub               |  11300/ 4560 |  3676/ 1512 |
+--------------------------------------------------------------+

```

