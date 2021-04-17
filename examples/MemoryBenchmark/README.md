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
  `ScanningModule`.
* Reduce flash by 100-200 bytes on AVR, SAMD21, STM32 and ESP8266 by
  templatizing the `ScanningModule` on `NUM_DIGITS` and `NUM_SUBFIELDS`, and
  merging `patterns` and `brightnesses` arrays directly into `ScanningModule`.
  Flash usage actually goes up by ~40 bytes on Teensy3.2, but it has enough
  flash memory.
* Reduce flash by 300-350 bytes on AVR (~150 on SAMD, 150-500 bytes on STM32,
  ~250 bytes on ESP8266, 300-600 bytes on ESP32) by templatizing LedMatrix
  and ScanningModule on `NUM_DIGITS`, `NUM_SUBFIELDS`, `SwSpiAdapter` and
  `HwSpiAdapter`.
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
  by removing the pointer to `TimingStats` from `ScanningModule`. The pointer
  causes the code for the `TimingStats` class to be pulled in, even if it is not
  used.

**v0.5**

* Slight increase in memory usage (20-30 bytes) on some processors (AVR,
  ESP8266, ESP8266), but slight decrease on others (STM32, Teensy), I think the
  changes are due to some removal/addition of some methods in `LedDisplay`.
* Add memory usage for `Tm1637Module`. Seems to consume something in between
  similar to the `ScanningModule` w/ SW SPI and `ScanningModule` with HW SPI.
* Add memory usage for `Tm1637Module` using `Tm1637DriverFast` which uses
  `digitalWriteFast` library for AVR processors. Saves 662 - 776 bytes of flash
  on AVR processors compared to `Tm1637Module` using normal `Tm1637Driver`.
* Save 150-200 bytes of flash on AVR processors by lifting all of the
  `LedDisplay::writePatternAt()` type of methods to `LedDisplay`, making them
  non-virtual, then funneling these methods through just 2 lower-level virtual
  methods: `setPatternAt()` and `getPatternAt()`. It also made the
  implementation of `Tm1637Module` position remapping easier.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program that includes the resources needed to perform a
`ScanningModule::renderFieldWhenReady()`. This includes:

* `ClockInterface`, `GpioInterface` (which are opimized away by the compiler)
* `SwSpiAdapter` or `HwSpiAdapter`
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
| ScanningDisplay(direct)         |   1536/   69 |  1080/   58 |
| ScanningDisplay(single_sw_spi)  |   1542/   63 |  1086/   52 |
| ScanningDisplay(single_hw_spi)  |   1604/   64 |  1148/   53 |
| ScanningDisplay(dual_sw_spi)    |   1444/   54 |   988/   43 |
| ScanningDisplay(dual_hw_spi)    |   1518/   55 |  1062/   44 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |   1278/   97 |   822/   86 |
| ScanningDisplay(single_sw_fast) |   1434/   61 |   978/   50 |
| ScanningDisplay(dual_sw_fast)   |   1044/   52 |   588/   41 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           |   1606/   39 |  1150/   28 |
| Tm1637Display(Fast)             |    946/   36 |   490/   25 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |    584/   24 |   128/   13 |
| NumberWriter+Stub               |    710/   26 |   254/   15 |
| ClockWriter+Stub                |    768/   27 |   312/   16 |
| TemperatureWriter+Stub          |    768/   26 |   312/   15 |
| CharWriter+Stub                 |    742/   26 |   286/   15 |
| StringWriter+Stub               |    940/   34 |   484/   23 |
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
| ScanningDisplay(direct)         |   4530/  209 |  1058/   58 |
| ScanningDisplay(single_sw_spi)  |   4538/  203 |  1066/   52 |
| ScanningDisplay(single_hw_spi)  |   4600/  204 |  1128/   53 |
| ScanningDisplay(dual_sw_spi)    |   4440/  194 |   968/   43 |
| ScanningDisplay(dual_hw_spi)    |   4514/  195 |  1042/   44 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |   4158/  237 |   686/   86 |
| ScanningDisplay(single_sw_fast) |   4430/  201 |   958/   50 |
| ScanningDisplay(dual_sw_fast)   |   3924/  192 |   452/   41 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           |   4676/  179 |  1204/   28 |
| Tm1637Display(Fast)             |   3902/  176 |   430/   25 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |   3540/  164 |    68/   13 |
| NumberWriter+Stub               |   3666/  166 |   194/   15 |
| ClockWriter+Stub                |   3724/  167 |   252/   16 |
| TemperatureWriter+Stub          |   3724/  166 |   252/   15 |
| CharWriter+Stub                 |   3698/  166 |   226/   15 |
| StringWriter+Stub               |   3896/  174 |   424/   23 |
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
| ScanningDisplay(direct)         |  10792/    0 |   728/    0 |
| ScanningDisplay(single_sw_spi)  |  10840/    0 |   776/    0 |
| ScanningDisplay(single_hw_spi)  |  11288/    0 |  1224/    0 |
| ScanningDisplay(dual_sw_spi)    |  10728/    0 |   664/    0 |
| ScanningDisplay(dual_hw_spi)    |  11248/    0 |  1184/    0 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           |  10800/    0 |   736/    0 |
| Tm1637Display(Fast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |  10312/    0 |   248/    0 |
| NumberWriter+Stub               |  10632/    0 |   568/    0 |
| ClockWriter+Stub                |  10432/    0 |   368/    0 |
| TemperatureWriter+Stub          |  10696/    0 |   632/    0 |
| CharWriter+Stub                 |  10472/    0 |   408/    0 |
| StringWriter+Stub               |  10656/    0 |   592/    0 |
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
| ScanningDisplay(direct)         |  21512/ 4016 |  2376/  228 |
| ScanningDisplay(single_sw_spi)  |  21576/ 4020 |  2440/  232 |
| ScanningDisplay(single_hw_spi)  |  23320/ 4020 |  4184/  232 |
| ScanningDisplay(dual_sw_spi)    |  21468/ 4012 |  2332/  224 |
| ScanningDisplay(dual_hw_spi)    |  23252/ 4012 |  4116/  224 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           |  21612/ 3984 |  2476/  196 |
| Tm1637Display(Fast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |  19300/ 3948 |   164/  160 |
| NumberWriter+Stub               |  19576/ 3952 |   440/  164 |
| ClockWriter+Stub                |  19456/ 3956 |   320/  168 |
| TemperatureWriter+Stub          |  19668/ 3952 |   532/  164 |
| CharWriter+Stub                 |  19460/ 3952 |   324/  164 |
| StringWriter+Stub               |  19628/ 3956 |   492/  168 |
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
| ScanningDisplay(direct)         | 257764/26860 |  1064/   76 |
| ScanningDisplay(single_sw_spi)  | 257836/26868 |  1136/   84 |
| ScanningDisplay(single_hw_spi)  | 258940/26876 |  2240/   92 |
| ScanningDisplay(dual_sw_spi)    | 257704/26848 |  1004/   64 |
| ScanningDisplay(dual_hw_spi)    | 258904/26856 |  2204/   72 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           | 257896/26824 |  1196/   40 |
| Tm1637Display(Fast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubDisplay                     | 256836/26792 |   136/    8 |
| NumberWriter+Stub               | 257316/26792 |   616/    8 |
| ClockWriter+Stub                | 257108/26800 |   408/   16 |
| TemperatureWriter+Stub          | 257412/26792 |   712/    8 |
| CharWriter+Stub                 | 257028/26792 |   328/    8 |
| StringWriter+Stub               | 257260/26816 |   560/   32 |
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
| ScanningDisplay(direct)         | 200468/13404 |  2738/  304 |
| ScanningDisplay(single_sw_spi)  | 200524/13404 |  2794/  304 |
| ScanningDisplay(single_hw_spi)  | 202816/13452 |  5086/  352 |
| ScanningDisplay(dual_sw_spi)    | 200396/13396 |  2666/  296 |
| ScanningDisplay(dual_hw_spi)    | 202764/13444 |  5034/  344 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           | 200716/13372 |  2986/  272 |
| Tm1637Display(Fast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubDisplay                     | 199194/13176 |  1464/   76 |
| NumberWriter+Stub               | 199570/13184 |  1840/   84 |
| ClockWriter+Stub                | 199462/13184 |  1732/   84 |
| TemperatureWriter+Stub          | 199658/13184 |  1928/   84 |
| CharWriter+Stub                 | 199410/13184 |  1680/   84 |
| StringWriter+Stub               | 199582/13184 |  1852/   84 |
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
| ScanningDisplay(direct)         |  11860/ 4204 |  4236/ 1156 |
| ScanningDisplay(single_sw_spi)  |  11908/ 4208 |  4284/ 1160 |
| ScanningDisplay(single_hw_spi)  |  12980/ 4264 |  5356/ 1216 |
| ScanningDisplay(dual_sw_spi)    |  11824/ 4200 |  4200/ 1152 |
| ScanningDisplay(dual_hw_spi)    |  12888/ 4256 |  5264/ 1208 |
|---------------------------------+--------------+-------------|
| ScanningDisplay(direct_fast)    |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(single_sw_fast) |     -1/   -1 |    -1/   -1 |
| ScanningDisplay(dual_sw_fast)   |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637Display(Normal)           |  12544/ 4172 |  4920/ 1124 |
| Tm1637Display(Fast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubDisplay                     |  10888/ 4156 |  3264/ 1108 |
| NumberWriter+Stub               |  11340/ 4160 |  3716/ 1112 |
| ClockWriter+Stub                |  11052/ 4164 |  3428/ 1116 |
| TemperatureWriter+Stub          |  11492/ 4160 |  3868/ 1112 |
| CharWriter+Stub                 |  11048/ 4160 |  3424/ 1112 |
| StringWriter+Stub               |  11244/ 4164 |  3620/ 1116 |
+--------------------------------------------------------------+

```

