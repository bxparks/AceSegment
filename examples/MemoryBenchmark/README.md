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
  changes are due to some removal/addition of some methods in `PatternWriter`.
* Add memory usage for `Tm1637Module`. Seems to consume something in between
  similar to the `ScanningModule` w/ SW SPI and `ScanningModule` with HW SPI.
* Add memory usage for `Tm1637Module` using `SoftTmiFastInterface` which uses
  `digitalWriteFast` library for AVR processors. Saves 662 - 776 bytes of flash
  on AVR processors compared to `Tm1637Module` using normal `SoftTmiInterface`.
* Save 150-200 bytes of flash on AVR processors by lifting all of the
  `PatternWriter::writePatternAt()` type of methods to `PatternWriter`, making
  them non-virtual, then funneling these methods through just 2 lower-level
  virtual methods: `setPatternAt()` and `getPatternAt()`. It also made the
  implementation of `Tm1637Module` position remapping easier.
* Extracting `LedModule` from `PatternWriter` saves 10-40 bytes on AVR for
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
* Extract out `readAck()`, saving 10 bytes of flash for `SoftTmiInterface` and
  6 bytes of flash for `SoftTmiFastInterface`.
* Add `Ht16k33Module(SimpleWire)` and `Ht16k33Module(SimpleWireFast)`.
* Rename `LedDisplay` to `PatternWriter` and remove one layer of abstraction.
  Saves 10-22 bytes of flash and 2 bytes of static RAM for most Writer
  classes (exception: `ClockWriter` and `StringWriter` which increases by 10-16
  bytes of flash).

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
* `Ht16k33Module`
* `NumberWriter`
* `ClockWriter`
* `TemperatureWriter`
* `CharWriter`
* `StringWriter`
* `StringScroller`
* `LevelWriter`

The `StubModule` is a dummy subclass of `LedModule` needed to create the
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
| Tm1637(SoftTmi)                 |   1278/   38 |  1018/   27 |
| Tm1637(SoftTmiFast)             |    634/   36 |   374/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |    992/   44 |   732/   33 |
| Max7219(SoftSpiFast)            |    558/   42 |   298/   31 |
| Max7219(HardSpi)                |   1348/   50 |  1088/   39 |
| Max7219(HardSpiFast)            |   1010/   49 |   750/   38 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   1338/   77 |  1078/   66 |
| Ht16k33(SimpleWire)             |   1264/   36 |  1004/   25 |
| Ht16k33(SimpleWireFast)         |    658/   33 |   398/   22 |
|---------------------------------+--------------+-------------|
| StubModule                      |    296/   11 |    36/    0 |
| PatternWriter+Stub              |    412/   26 |   152/   15 |
| NumberWriter+Stub               |    456/   26 |   196/   15 |
| ClockWriter+Stub                |    568/   27 |   308/   16 |
| TemperatureWriter+Stub          |    524/   26 |   264/   15 |
| CharWriter+Stub                 |    582/   29 |   322/   18 |
| StringWriter+Stub               |    784/   37 |   524/   26 |
| StringScroller+Stub             |    814/   43 |   554/   32 |
| LevelWriter+Stub                |    490/   26 |   230/   15 |
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
| Tm1637(SoftTmi)                 |   1568/   38 |  1112/   27 |
| Tm1637(SoftTmiFast)             |    920/   36 |   464/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |   1214/   44 |   758/   33 |
| Max7219(SoftSpiFast)            |    774/   42 |   318/   31 |
| Max7219(HardSpi)                |   1290/   46 |   834/   35 |
| Max7219(HardSpiFast)            |   1184/   45 |   728/   34 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   2866/  251 |  2410/  240 |
| Ht16k33(SimpleWire)             |   1558/   36 |  1102/   25 |
| Ht16k33(SimpleWireFast)         |    968/   33 |   512/   22 |
|---------------------------------+--------------+-------------|
| StubModule                      |    494/   11 |    38/    0 |
| PatternWriter+Stub              |    608/   26 |   152/   15 |
| NumberWriter+Stub               |    664/   26 |   208/   15 |
| ClockWriter+Stub                |    782/   27 |   326/   16 |
| TemperatureWriter+Stub          |    736/   26 |   280/   15 |
| CharWriter+Stub                 |    778/   29 |   322/   18 |
| StringWriter+Stub               |    998/   37 |   542/   26 |
| StringScroller+Stub             |   1014/   43 |   558/   32 |
| LevelWriter+Stub                |    694/   26 |   238/   15 |
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
| Tm1637(SoftTmi)                 |   4638/  178 |  1166/   27 |
| Tm1637(SoftTmiFast)             |   3876/  176 |   404/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |   4284/  184 |   812/   33 |
| Max7219(SoftSpiFast)            |   3730/  182 |   258/   31 |
| Max7219(HardSpi)                |   4360/  186 |   888/   35 |
| Max7219(HardSpiFast)            |   4242/  185 |   770/   34 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   5850/  391 |  2378/  240 |
| Ht16k33(SimpleWire)             |   4628/  176 |  1156/   25 |
| Ht16k33(SimpleWireFast)         |   3924/  173 |   452/   22 |
|---------------------------------+--------------+-------------|
| StubModule                      |   3508/  151 |    36/    0 |
| PatternWriter+Stub              |   3564/  166 |    92/   15 |
| NumberWriter+Stub               |   3620/  166 |   148/   15 |
| ClockWriter+Stub                |   3738/  167 |   266/   16 |
| TemperatureWriter+Stub          |   3692/  166 |   220/   15 |
| CharWriter+Stub                 |   3734/  169 |   262/   18 |
| StringWriter+Stub               |   3954/  177 |   482/   26 |
| StringScroller+Stub             |   3970/  183 |   498/   32 |
| LevelWriter+Stub                |   3650/  166 |   178/   15 |
+--------------------------------------------------------------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.3

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
| Tm1637(SoftTmi)                 |  10784/    0 |   720/    0 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  10600/    0 |   536/    0 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  11096/    0 |  1032/    0 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  11904/    0 |  1840/    0 |
| Ht16k33(SimpleWire)             |  10840/    0 |   776/    0 |
| Ht16k33(SimpleWireFast)         |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule                      |  10312/    0 |   248/    0 |
| PatternWriter+Stub              |  10336/    0 |   272/    0 |
| NumberWriter+Stub               |  10648/    0 |   584/    0 |
| ClockWriter+Stub                |  10448/    0 |   384/    0 |
| TemperatureWriter+Stub          |  10712/    0 |   648/    0 |
| CharWriter+Stub                 |  10520/    0 |   456/    0 |
| StringWriter+Stub               |  10656/    0 |   592/    0 |
| StringScroller+Stub             |  10640/    0 |   576/    0 |
| LevelWriter+Stub                |  10400/    0 |   336/    0 |
+--------------------------------------------------------------+

```

### STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 2.0.0

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
| Tm1637(SoftTmi)                 |  24308/ 3936 |  2888/  400 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  24044/ 3936 |  2624/  400 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  25940/ 3940 |  4520/  404 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  28680/ 4120 |  7260/  584 |
| Ht16k33(SimpleWire)             |  24372/ 3932 |  2952/  396 |
| Ht16k33(SimpleWireFast)         |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule                      |  21616/ 3900 |   196/  364 |
| PatternWriter+Stub              |  21636/ 3904 |   216/  368 |
| NumberWriter+Stub               |  21904/ 3904 |   484/  368 |
| ClockWriter+Stub                |  21784/ 3908 |   364/  372 |
| TemperatureWriter+Stub          |  21996/ 3904 |   576/  368 |
| CharWriter+Stub                 |  21824/ 3912 |   404/  376 |
| StringWriter+Stub               |  21964/ 3916 |   544/  380 |
| StringScroller+Stub             |  21944/ 3924 |   524/  388 |
| LevelWriter+Stub                |  21692/ 3904 |   272/  368 |
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
| Tm1637(SoftTmi)                 | 257884/27028 |  1184/  244 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                | 257636/27028 |   936/  244 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                | 258804/27044 |  2104/  260 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                | 261364/27500 |  4664/  716 |
| Ht16k33(SimpleWire)             | 257980/27028 |  1280/  244 |
| Ht16k33(SimpleWireFast)         |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule                      | 256840/26996 |   140/  212 |
| PatternWriter+Stub              | 256872/27004 |   172/  220 |
| NumberWriter+Stub               | 257336/27004 |   636/  220 |
| ClockWriter+Stub                | 257160/27004 |   460/  220 |
| TemperatureWriter+Stub          | 257448/27004 |   748/  220 |
| CharWriter+Stub                 | 257096/27012 |   396/  228 |
| StringWriter+Stub               | 257264/27012 |   564/  228 |
| StringScroller+Stub             | 257296/27020 |   596/  236 |
| LevelWriter+Stub                | 256984/27004 |   284/  220 |
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
| Tm1637(SoftTmi)                 | 200670/13552 |  2922/  468 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                | 200284/13536 |  2536/  452 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                | 202648/13592 |  4900/  508 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                | 209918/14288 | 12170/ 1204 |
| Ht16k33(SimpleWire)             | 200774/13544 |  3026/  460 |
| Ht16k33(SimpleWireFast)         |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule                      | 199196/13360 |  1448/  276 |
| PatternWriter+Stub              | 199256/13368 |  1508/  284 |
| NumberWriter+Stub               | 199580/13368 |  1832/  284 |
| ClockWriter+Stub                | 199528/13368 |  1780/  284 |
| TemperatureWriter+Stub          | 199700/13368 |  1952/  284 |
| CharWriter+Stub                 | 199468/13376 |  1720/  292 |
| StringWriter+Stub               | 199604/13376 |  1856/  292 |
| StringScroller+Stub             | 199604/13384 |  1856/  300 |
| LevelWriter+Stub                | 199364/13368 |  1616/  284 |
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
| Ht16k33(TwoWire)                |  14472/ 5032 |  6848/ 1984 |
| Ht16k33(SimpleWire)             |  13504/ 4364 |  5880/ 1316 |
| Ht16k33(SimpleWireFast)         |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule                      |  10896/ 4352 |  3272/ 1304 |
| PatternWriter+Stub              |  10940/ 4356 |  3316/ 1308 |
| NumberWriter+Stub               |  11372/ 4356 |  3748/ 1308 |
| ClockWriter+Stub                |  11072/ 4360 |  3448/ 1312 |
| TemperatureWriter+Stub          |  11524/ 4356 |  3900/ 1308 |
| CharWriter+Stub                 |  11096/ 4364 |  3472/ 1316 |
| StringWriter+Stub               |  11288/ 4368 |  3664/ 1320 |
| StringScroller+Stub             |  11296/ 4376 |  3672/ 1328 |
| LevelWriter+Stub                |  11048/ 4356 |  3424/ 1308 |
+--------------------------------------------------------------+

```

