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

**Version**: AceSegment v0.6

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
  and ScanningModule on `NUM_DIGITS`, `NUM_SUBFIELDS`, `SoftSpiInterface` and
  `HardSpiInterface`.
* Reduce flash by flattening the `LedMatrix` hierarchy into templatized
  classes, and removing virtual methods. Saves 250-300 bytes on AVR, 150-200 on
  SAMD, 150-300 on STM32, 200-300 on ESP8266, 300-1300 bytes on ESP32, 800-1300
  bytes on Teensy 3.2.
* Reduce flash by 250-400 bytes on AVR by providing ability to use
  `digitalWriteFast()` (https://github.com/NicksonYap/digitalWriteFast) using
  the `scanning/LedMatrixDirectFast4.h` and `hw/SoftSpiFastInterface.h` classes.
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
* Add memory usage for `Tm1637Module` using `SoftTmiFastInterface` which uses
  `digitalWriteFast` library for AVR processors. Saves 662 - 776 bytes of flash
  on AVR processors compared to `Tm1637Module` using normal `SoftTmiInterface`.
* Save 150-200 bytes of flash on AVR processors by lifting all of the
  `LedDisplay::writePatternAt()` type of methods to `LedDisplay`, making them
  non-virtual, then funneling these methods through just 2 lower-level virtual
  methods: `setPatternAt()` and `getPatternAt()`. It also made the
  implementation of `Tm1637Module` position remapping easier.
* Extracting `LedModule` from `LedDisplay` saves 10-40 bytes on AVR for
  `ScanningModule` and `Tm1637Module`, but add about that many bytes for various
  Writer classes (probably because they have to go though one additional layer
  of indirection through the `LedModule`). So overall, I think it's a wash.
* Add `HardSpiFastInterface` which saves 70 bytes for `ScanningModule(Single)`,
  90 bytes for `ScanningModule(Dual)`, and 250 bytes for `Max7219Module`.
* Hide implementation details involving `LedMatrixXxx` and `ScanningModule` by
  using the convenience classes (`DirectModule`, `DirectFast4Module`,
  `HybridModule`, `Hc595Module`).
* Enabling user-defined character sets in `CharWriter` causes the flash memory
  consumption to increase by 30 bytes on AVR processors, and 36 bytes on 32-bit
  processors. Similar increase in `StringWriter` which now explicitly depends on
  CharWriter. But I think the additional configurability is worth it since
  different people have different aesthetic standards and want different fonts.
* Adding `byteOrder` and `remapArray` parameters to `Hc595Module` increases the
  memory consumption by 60 bytes on AVR and about 20-40 bytes on 32-bit
  processors.

**v0.6**

* Add support for multiple SPI buses in `HardSpiInterface` and
  `HardSpiFastInterface`. Increases flash memory by 10-30 bytes.
* Add benchmarks for `StringScroller` and `LevelWriter`.

**v0.6+**

* Add benchmarks for `Ht16k33Module`. Consumes about 2400 bytes of flash on
  ATmega328 (Nano) or ATmega32U4 (Pro Micro), about 2X larger than any other LED
  module due to the I2C `<Wire.h>` library.
* The `Max7219(HardSpiFast)` increases by about 100 on AVR because the previous
  version neglected to call `Max7219Module::flush()`.
* Modules using hardware SPI (through `HardSpiInterface` or
  `HardSpiFastInterface`) becomes slightly smaller (30 bytes of flash, 2 bytes
  of static RAM on AVR) due to removal of explicit `pinMode(dataPin, X)` and
  `pinMode(clockPin, X)`. These are deferred to `SPIClass::begin()`.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program for various `LedModule` configurations and various Writer classes.

* `ClockInterface`, `GpioInterface` (usually optimized away by the compiler)
* `SoftSpiInterface`, `SoftSpiFastInterface`, `HardSpiInterface`,
  `HardSpiFastInterface`
* `DirectModule`
* `DirectFast4Module`
* `HybridModule`
* `Hc595Module`
* `Tm1637Module`
* `Max7219Module`
* `NumberWriter`
* `ClockWriter`
* `TemperatureWriter`
* `CharWriter`
* `StringWriter`
* `StringScroller`
* `LevelWriter`

The `StubDisplay` is a dummy subclass of `LedDisplay` needed to create the
various Writers. To get a better flash consumption of the Writer classes, this
stub class should be subtracted from the numbers below. (Ideally, the
`generate_table.awk` script should do this automatically, but I'm trying to keep
that script more general to avoid maintenance overhead when it is copied into
other `MemoryBenchmark` programs.)

### ATtiny85

* 8MHz ATtiny85
* Arduino IDE 1.8.13
* SpenceKonde/ATTinyCore 1.5.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    260/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   1248/   64 |   988/   53 |
| DirectFast4Module               |   1012/   94 |   752/   83 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |   1282/   58 |  1022/   47 |
| Hybrid(SoftSpiFast)             |   1168/   56 |   908/   45 |
| Hybrid(HardSpi)                 |   1646/   64 |  1386/   53 |
| Hybrid(HardSpiFast)             |   1602/   63 |  1342/   52 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |   1262/   58 |  1002/   47 |
| Hc595(SoftSpiFast)              |    856/   56 |   596/   45 |
| Hc595(HardSpi)                  |   1620/   64 |  1360/   53 |
| Hc595(HardSpiFast)              |   1314/   63 |  1054/   52 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |   1290/   39 |  1030/   28 |
| Tm1637(SoftTmiFast)             |    642/   36 |   382/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |    992/   44 |   732/   33 |
| Max7219(SoftSpiFast)            |    558/   42 |   298/   31 |
| Max7219(HardSpi)                |   1348/   50 |  1088/   39 |
| Max7219(HardSpiFast)            |   1010/   49 |   750/   38 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               |   1338/   77 |  1078/   66 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |    378/   24 |   118/   13 |
| NumberWriter+Stub               |    474/   28 |   214/   17 |
| ClockWriter+Stub                |    554/   29 |   294/   18 |
| TemperatureWriter+Stub          |    552/   28 |   292/   17 |
| CharWriter+Stub                 |    592/   31 |   332/   20 |
| StringWriter+Stub               |    774/   39 |   514/   28 |
| StringScroller+Stub             |    836/   45 |   576/   34 |
| LevelWriter+Stub                |    510/   28 |   250/   17 |
+--------------------------------------------------------------+

```

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
| DirectModule                    |   1486/   64 |  1030/   53 |
| DirectFast4Module               |   1250/   94 |   794/   83 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |   1508/   58 |  1052/   47 |
| Hybrid(SoftSpiFast)             |   1400/   56 |   944/   45 |
| Hybrid(HardSpi)                 |   1556/   60 |  1100/   49 |
| Hybrid(HardSpiFast)             |   1506/   59 |  1050/   48 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |   1486/   58 |  1030/   47 |
| Hc595(SoftSpiFast)              |   1076/   56 |   620/   45 |
| Hc595(HardSpi)                  |   1542/   60 |  1086/   49 |
| Hc595(HardSpiFast)              |   1474/   59 |  1018/   48 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |   1582/   39 |  1126/   28 |
| Tm1637(SoftTmiFast)             |    926/   36 |   470/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |   1214/   44 |   758/   33 |
| Max7219(SoftSpiFast)            |    774/   42 |   318/   31 |
| Max7219(HardSpi)                |   1290/   46 |   834/   35 |
| Max7219(HardSpiFast)            |   1184/   45 |   728/   34 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               |   2866/  251 |  2410/  240 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |    578/   24 |   122/   13 |
| NumberWriter+Stub               |    682/   28 |   226/   17 |
| ClockWriter+Stub                |    766/   29 |   310/   18 |
| TemperatureWriter+Stub          |    764/   28 |   308/   17 |
| CharWriter+Stub                 |    788/   31 |   332/   20 |
| StringWriter+Stub               |    988/   39 |   532/   28 |
| StringScroller+Stub             |   1036/   45 |   580/   34 |
| LevelWriter+Stub                |    716/   28 |   260/   17 |
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
| DirectModule                    |   4482/  204 |  1010/   53 |
| DirectFast4Module               |   4132/  234 |   660/   83 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |   4504/  198 |  1032/   47 |
| Hybrid(SoftSpiFast)             |   4396/  196 |   924/   45 |
| Hybrid(HardSpi)                 |   4552/  200 |  1080/   49 |
| Hybrid(HardSpiFast)             |   4502/  199 |  1030/   48 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |   4482/  198 |  1010/   47 |
| Hc595(SoftSpiFast)              |   3958/  196 |   486/   45 |
| Hc595(HardSpi)                  |   4538/  200 |  1066/   49 |
| Hc595(HardSpiFast)              |   4458/  199 |   986/   48 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |   4652/  179 |  1180/   28 |
| Tm1637(SoftTmiFast)             |   3882/  176 |   410/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |   4284/  184 |   812/   33 |
| Max7219(SoftSpiFast)            |   3730/  182 |   258/   31 |
| Max7219(HardSpi)                |   4360/  186 |   888/   35 |
| Max7219(HardSpiFast)            |   4242/  185 |   770/   34 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               |   5850/  391 |  2378/  240 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |   3534/  164 |    62/   13 |
| NumberWriter+Stub               |   3638/  168 |   166/   17 |
| ClockWriter+Stub                |   3722/  169 |   250/   18 |
| TemperatureWriter+Stub          |   3720/  168 |   248/   17 |
| CharWriter+Stub                 |   3744/  171 |   272/   20 |
| StringWriter+Stub               |   3944/  179 |   472/   28 |
| StringScroller+Stub             |   3992/  185 |   520/   34 |
| LevelWriter+Stub                |   3672/  168 |   200/   17 |
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
| DirectModule                    |  10800/    0 |   736/    0 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |  10848/    0 |   784/    0 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 |  11264/    0 |  1200/    0 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |  10744/    0 |   680/    0 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  |  11248/    0 |  1184/    0 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |  10792/    0 |   728/    0 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  10600/    0 |   536/    0 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  11096/    0 |  1032/    0 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               |  11904/    0 |  1840/    0 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  10320/    0 |   256/    0 |
| NumberWriter+Stub               |  10656/    0 |   592/    0 |
| ClockWriter+Stub                |  10464/    0 |   400/    0 |
| TemperatureWriter+Stub          |  10720/    0 |   656/    0 |
| CharWriter+Stub                 |  10528/    0 |   464/    0 |
| StringWriter+Stub               |  10712/    0 |   648/    0 |
| StringScroller+Stub             |  10656/    0 |   592/    0 |
| LevelWriter+Stub                |  10408/    0 |   344/    0 |
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
| baseline                        |  21420/ 3536 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |  24176/ 3956 |  2756/  420 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |  24232/ 3960 |  2812/  424 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 |  26084/ 3964 |  4664/  428 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |  24148/ 3964 |  2728/  428 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  |  26040/ 3968 |  4620/  432 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |  24316/ 3936 |  2896/  400 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  24044/ 3936 |  2624/  400 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  25940/ 3940 |  4520/  404 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               |  28680/ 4120 |  7260/  584 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  21624/ 3904 |   204/  368 |
| NumberWriter+Stub               |  21912/ 3908 |   492/  372 |
| ClockWriter+Stub                |  21792/ 3912 |   372/  376 |
| TemperatureWriter+Stub          |  22008/ 3908 |   588/  372 |
| CharWriter+Stub                 |  21832/ 3916 |   412/  380 |
| StringWriter+Stub               |  22004/ 3920 |   584/  384 |
| StringScroller+Stub             |  21956/ 3928 |   536/  392 |
| LevelWriter+Stub                |  21704/ 3908 |   284/  372 |
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
| DirectModule                    | 257784/27056 |  1084/  272 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 | 257856/27056 |  1156/  272 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 | 258928/27072 |  2228/  288 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  | 257756/27060 |  1056/  276 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  | 258924/27068 |  2224/  284 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 | 257900/27028 |  1200/  244 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                | 257636/27028 |   936/  244 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                | 258804/27044 |  2104/  260 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               | 261364/27500 |  4664/  716 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           | 256856/27004 |   156/  220 |
| NumberWriter+Stub               | 257352/27004 |   652/  220 |
| ClockWriter+Stub                | 257176/27012 |   476/  228 |
| TemperatureWriter+Stub          | 257464/27004 |   764/  220 |
| CharWriter+Stub                 | 257096/27012 |   396/  228 |
| StringWriter+Stub               | 257344/27020 |   644/  236 |
| StringScroller+Stub             | 257296/27028 |   596/  244 |
| LevelWriter+Stub                | 257000/27004 |   300/  220 |
+--------------------------------------------------------------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 197748/13084 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    | 200482/13568 |  2734/  484 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 | 200522/13576 |  2774/  492 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 | 202790/13624 |  5042/  540 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  | 200418/13576 |  2670/  492 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  | 202766/13632 |  5018/  548 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 | 200678/13552 |  2930/  468 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                | 200284/13536 |  2536/  452 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                | 202648/13592 |  4900/  508 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               | 209918/14288 | 12170/ 1204 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           | 199204/13368 |  1456/  284 |
| NumberWriter+Stub               | 199592/13368 |  1844/  284 |
| ClockWriter+Stub                | 199536/13376 |  1788/  292 |
| TemperatureWriter+Stub          | 199712/13368 |  1964/  284 |
| CharWriter+Stub                 | 199476/13376 |  1728/  292 |
| StringWriter+Stub               | 199672/13384 |  1924/  300 |
| StringScroller+Stub             | 199624/13392 |  1876/  308 |
| LevelWriter+Stub                | 199388/13368 |  1640/  284 |
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
| DirectModule                    |  11880/ 4388 |  4256/ 1340 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |  11924/ 4392 |  4300/ 1344 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 |  12948/ 4452 |  5324/ 1404 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |  11860/ 4396 |  4236/ 1348 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  |  12888/ 4456 |  5264/ 1408 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |  12536/ 4368 |  4912/ 1320 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  11812/ 4368 |  4188/ 1320 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  13256/ 4428 |  5632/ 1380 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(HardWire)               |  14472/ 5032 |  6848/ 1984 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  10904/ 4356 |  3280/ 1308 |
| NumberWriter+Stub               |  11380/ 4360 |  3756/ 1312 |
| ClockWriter+Stub                |  11092/ 4364 |  3468/ 1316 |
| TemperatureWriter+Stub          |  11536/ 4360 |  3912/ 1312 |
| CharWriter+Stub                 |  11104/ 4368 |  3480/ 1320 |
| StringWriter+Stub               |  11308/ 4372 |  3684/ 1324 |
| StringScroller+Stub             |  11304/ 4380 |  3680/ 1332 |
| LevelWriter+Stub                |  11056/ 4360 |  3432/ 1312 |
+--------------------------------------------------------------+

```

