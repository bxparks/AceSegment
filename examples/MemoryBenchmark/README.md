# Memory Benchmark

The `MemoryBenchmark.ino` collects the amount of flash and static memory
consumed by different implementations in the AceSegment library.

It compiles various code snippets which are controlled by the `FEATURE` macro
flag. The `collect.sh` edits this `FEATURE` flag programmatically, then runs the
Arduino IDE compiler on the program, and extracts the flash and static memory
usage into a text file (e.g. `nano.txt`).

The numbers shown below should be considered to be rough estimates. It is often
difficult to separate out the code size of the library from the overhead imposed
by the runtime environment of the processor. For example, it often seems like
the ESP8266 allocates flash memory in blocks of a certain quantity, so the
calculated flash size can jump around in unexpected ways.

**Version**: AceSegment v0.8.1

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
  and ScanningModule on `NUM_DIGITS`, `NUM_SUBFIELDS`, `SimpleSpiInterface` and
  `HardSpiInterface`.
* Reduce flash by flattening the `LedMatrix` hierarchy into templatized
  classes, and removing virtual methods. Saves 250-300 bytes on AVR, 150-200 on
  SAMD, 150-300 on STM32, 200-300 on ESP8266, 300-1300 bytes on ESP32, 800-1300
  bytes on Teensy 3.2.
* Reduce flash by 250-400 bytes on AVR by providing ability to use
  `digitalWriteFast()` (https://github.com/NicksonYap/digitalWriteFast) using
  the `scanning/LedMatrixDirectFast4.h` and `ace_spi/SimpleSpiFastInterface.h`
  classes.
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
* Add memory usage for `Tm1637Module` using `SimpleTmiFastInterface` which uses
  `digitalWriteFast` library for AVR processors. Saves 662 - 776 bytes of flash
  on AVR processors compared to `Tm1637Module` using normal
  `SimpleTmiInterface`.
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

**v0.7**

* Add benchmarks for `Ht16k33Module`. Consumes about 2400 bytes of flash on
  ATmega328 (Nano) or ATmega32U4 (Pro Micro), about 2X larger than any other LED
  module due to the I2C `<Wire.h>` library.
* The `Max7219(HardSpiFast)` increases by about 100 on AVR because the previous
  version neglected to call `Max7219Module::flush()`.
* Modules using hardware SPI (through `HardSpiInterface` or
  `HardSpiFastInterface`) becomes slightly smaller (30 bytes of flash, 2 bytes
  of static RAM on AVR) due to removal of explicit `pinMode(dataPin, X)` and
  `pinMode(clockPin, X)`. These are deferred to `SPIClass::begin()`.
* Extract out `readAck()`, saving 10 bytes of flash for `SimpleTmiInterface` and
  6 bytes of flash for `SimpleTmiFastInterface`.
* Add `Ht16k33Module(SimpleWire)` and `Ht16k33Module(SimpleWireFast)`.
* Rename `LedDisplay` to `PatternWriter` and remove one layer of abstraction.
  Saves 10-22 bytes of flash and 2 bytes of static RAM for most Writer
  classes (exception: `ClockWriter` and `StringWriter` which increases by 10-16
  bytes of flash).
* Modify `FEATURE_BASELINE` for TeensyDuino so that `malloc()` and `free()`
  are included in its memory consumption. When a class is used polymorphically
  (i.e. its virtual methods are called), TeensyDuino seems to automatically pull
  in `malloc()` and `free()`, which seems to consume about 3200 bytes of flash
  and 1100 bytes of static memory. This happens for all FEATURES other than
  BASELINE, so we have to make sure that BASELINE also pulls in these. All
  results for Teensy 3.2 become lower by 3200 bytes of flash and 1100 bytes of
  static RAM.

**v0.8**

* Extract communcation interfaces into AceSPI, AceTMI, and AceWire libraries.
  No change in memory consumption.
* Copy AceSPI, AceTMI, and AceWire interface objects by *value* into various
  modules (i.e. Hc595Module, Ht16k33Module, Max7219Module, Tm1637Module)
  instead of by *reference*.
    * Interface objects are thin-adapters which hold only a few parameters (0 to
      3) and are immutable.
    * Copying them by-value into the various modules eliminates an extra level
      of indirection through a pointer to the interface objects.
    * On AVR processors, this saves between 0 to 90 bytes of flash on most
      configurations. The most significant savings occur with the following:
        * Tm1637Module(SimpleTmi) saves 90 bytes,
        * Ht16k33Module(SimpleWire) saves 68 bytes of flash,
        * Max7219Module(SimpleSpi) saves 30 bytes of flash.
    * On 32-bit processors, the flash consumption usually goes *up* by 4-20
      bytes, but decreases by a few bytes in a few cases.
    * The 32-bit processors have so much more flash memory than 8-bit
      processors, I think this tradeoff is worth it.

**v0.8+**

* Remove `virtual` keyword from `LedModule` methods.
    * Decreases flash usage by 60 bytes for `Tm1637Module`, 14 bytes for
      `Max7219Module`, 32 bytes for `Ht16k33Module`, and 2-14 bytes for
      `Hc595Module`.
    * Decreases static ram usage by 7-8 bytes for all Module classes.
    * Further decreases flash usage by 10-70 bytes for various Writer classes.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program for various `LedModule` configurations and various Writer classes.

* `ClockInterface`, `GpioInterface` (usually optimized away by the compiler)
* `SimpleSpiInterface`, `SimpleSpiFastInterface`, `HardSpiInterface`,
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
| DirectModule                    |   1280/   57 |  1020/   46 |
| DirectFast4Module               |   1026/   87 |   766/   76 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   1664/   58 |  1404/   47 |
| Hybrid(HardSpiFast)             |   1616/   56 |  1356/   45 |
| Hybrid(SimpleSpi)               |   1306/   52 |  1046/   41 |
| Hybrid(SimpleSpiFast)           |   1178/   47 |   918/   36 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   1622/   58 |  1362/   47 |
| Hc595(HardSpiFast)              |   1326/   56 |  1066/   45 |
| Hc595(SimpleSpi)                |   1254/   52 |   994/   41 |
| Hc595(SimpleSpiFast)            |    842/   47 |   582/   36 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               |   1132/   31 |   872/   20 |
| Tm1637(SimpleTmiFast)           |    570/   26 |   310/   15 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |   1316/   43 |  1056/   32 |
| Max7219(HardSpiFast)            |    992/   41 |   732/   30 |
| Max7219(SimpleSpi)              |    952/   37 |   692/   26 |
| Max7219(SimpleSpiFast)          |    538/   32 |   278/   21 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   1294/   69 |  1034/   58 |
| Ht16k33(SimpleWire)             |   1166/   33 |   906/   22 |
| Ht16k33(SimpleWireFast)         |    648/   27 |   388/   16 |
|---------------------------------+--------------+-------------|
| StubModule                      |    322/   21 |    62/   10 |
| PatternWriter+Stub              |    338/   23 |    78/   12 |
| NumberWriter+Stub               |    418/   23 |   158/   12 |
| ClockWriter+Stub                |    554/   24 |   294/   13 |
| TemperatureWriter+Stub          |    462/   23 |   202/   12 |
| CharWriter+Stub                 |    520/   26 |   260/   15 |
| StringWriter+Stub               |    748/   34 |   488/   23 |
| StringScroller+Stub             |    804/   40 |   544/   29 |
| LevelWriter+Stub                |    424/   23 |   164/   12 |
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
| DirectModule                    |   1518/   57 |  1062/   46 |
| DirectFast4Module               |   1274/   87 |   818/   76 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   1580/   54 |  1124/   43 |
| Hybrid(HardSpiFast)             |   1522/   52 |  1066/   41 |
| Hybrid(SimpleSpi)               |   1534/   52 |  1078/   41 |
| Hybrid(SimpleSpiFast)           |   1412/   47 |   956/   36 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   1534/   54 |  1078/   43 |
| Hc595(HardSpiFast)              |   1468/   52 |  1012/   41 |
| Hc595(SimpleSpi)                |   1474/   52 |  1018/   41 |
| Hc595(SimpleSpiFast)            |   1066/   47 |   610/   36 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               |   1424/   31 |   968/   20 |
| Tm1637(SimpleTmiFast)           |    858/   26 |   402/   15 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |   1260/   39 |   804/   28 |
| Max7219(HardSpiFast)            |   1170/   37 |   714/   26 |
| Max7219(SimpleSpi)              |   1174/   37 |   718/   26 |
| Max7219(SimpleSpiFast)          |    756/   32 |   300/   21 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   2846/  243 |  2390/  232 |
| Ht16k33(SimpleWire)             |   1450/   33 |   994/   22 |
| Ht16k33(SimpleWireFast)         |    916/   27 |   460/   16 |
|---------------------------------+--------------+-------------|
| StubModule                      |    522/   21 |    66/   10 |
| PatternWriter+Stub              |    534/   23 |    78/   12 |
| NumberWriter+Stub               |    630/   23 |   174/   12 |
| ClockWriter+Stub                |    764/   24 |   308/   13 |
| TemperatureWriter+Stub          |    674/   23 |   218/   12 |
| CharWriter+Stub                 |    718/   26 |   262/   15 |
| StringWriter+Stub               |    970/   34 |   514/   23 |
| StringScroller+Stub             |   1016/   40 |   560/   29 |
| LevelWriter+Stub                |    626/   23 |   170/   12 |
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
| DirectModule                    |   4514/  197 |  1042/   46 |
| DirectFast4Module               |   4156/  227 |   684/   76 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   4576/  194 |  1104/   43 |
| Hybrid(HardSpiFast)             |   4518/  192 |  1046/   41 |
| Hybrid(SimpleSpi)               |   4530/  192 |  1058/   41 |
| Hybrid(SimpleSpiFast)           |   4408/  187 |   936/   36 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   4552/  194 |  1080/   43 |
| Hc595(HardSpiFast)              |   4474/  192 |  1002/   41 |
| Hc595(SimpleSpi)                |   4492/  192 |  1020/   41 |
| Hc595(SimpleSpiFast)            |   3970/  187 |   498/   36 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               |   4516/  171 |  1044/   20 |
| Tm1637(SimpleTmiFast)           |   3836/  166 |   364/   15 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |   4330/  179 |   858/   28 |
| Max7219(HardSpiFast)            |   4228/  177 |   756/   26 |
| Max7219(SimpleSpi)              |   4244/  177 |   772/   26 |
| Max7219(SimpleSpiFast)          |   3712/  172 |   240/   21 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   5830/  383 |  2358/  232 |
| Ht16k33(SimpleWire)             |   4542/  173 |  1070/   22 |
| Ht16k33(SimpleWireFast)         |   3892/  167 |   420/   16 |
|---------------------------------+--------------+-------------|
| StubModule                      |   3500/  161 |    28/   10 |
| PatternWriter+Stub              |   3512/  163 |    40/   12 |
| NumberWriter+Stub               |   3608/  163 |   136/   12 |
| ClockWriter+Stub                |   3742/  164 |   270/   13 |
| TemperatureWriter+Stub          |   3652/  163 |   180/   12 |
| CharWriter+Stub                 |   3696/  166 |   224/   15 |
| StringWriter+Stub               |   3926/  174 |   454/   23 |
| StringScroller+Stub             |   3972/  180 |   500/   29 |
| LevelWriter+Stub                |   3604/  163 |   132/   12 |
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
| baseline                        |   9940/    0 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |  10652/    0 |   712/    0 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |  11136/    0 |  1196/    0 |
| Hybrid(SimpleSpi)               |  10712/    0 |   772/    0 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |  11128/    0 |  1188/    0 |
| Hc595(SimpleSpi)                |  10628/    0 |   688/    0 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               |  10660/    0 |   720/    0 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |  10968/    0 |  1028/    0 |
| Max7219(SimpleSpi)              |  10464/    0 |   524/    0 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  11740/    0 |  1800/    0 |
| Ht16k33(SimpleWire)             |  10704/    0 |   764/    0 |
|---------------------------------+--------------+-------------|
| StubModule                      |  10140/    0 |   200/    0 |
| PatternWriter+Stub              |  10156/    0 |   216/    0 |
| NumberWriter+Stub               |  10452/    0 |   512/    0 |
| ClockWriter+Stub                |  10300/    0 |   360/    0 |
| TemperatureWriter+Stub          |  10540/    0 |   600/    0 |
| CharWriter+Stub                 |  10340/    0 |   400/    0 |
| StringWriter+Stub               |  10460/    0 |   520/    0 |
| StringScroller+Stub             |  10476/    0 |   536/    0 |
| LevelWriter+Stub                |  10216/    0 |   276/    0 |
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
| DirectModule                    |  24172/ 3956 |  2752/  420 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |  26100/ 3968 |  4680/  432 |
| Hybrid(SimpleSpi)               |  24244/ 3960 |  2824/  424 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |  26060/ 3972 |  4640/  436 |
| Hc595(SimpleSpi)                |  24160/ 3964 |  2740/  428 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               |  24308/ 3936 |  2888/  400 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |  25936/ 3940 |  4516/  404 |
| Max7219(SimpleSpi)              |  24032/ 3932 |  2612/  396 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  28644/ 4116 |  7224/  580 |
| Ht16k33(SimpleWire)             |  24352/ 3932 |  2932/  396 |
|---------------------------------+--------------+-------------|
| StubModule                      |  21572/ 3900 |   152/  364 |
| PatternWriter+Stub              |  21584/ 3904 |   164/  368 |
| NumberWriter+Stub               |  21852/ 3904 |   432/  368 |
| ClockWriter+Stub                |  21720/ 3908 |   300/  372 |
| TemperatureWriter+Stub          |  21952/ 3904 |   532/  368 |
| CharWriter+Stub                 |  21764/ 3912 |   344/  376 |
| StringWriter+Stub               |  21888/ 3916 |   468/  380 |
| StringScroller+Stub             |  21892/ 3924 |   472/  388 |
| LevelWriter+Stub                |  21652/ 3904 |   232/  368 |
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
| DirectModule                    | 257736/27056 |  1036/  272 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 | 258896/27072 |  2196/  288 |
| Hybrid(SimpleSpi)               | 257824/27056 |  1124/  272 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  | 258876/27076 |  2176/  292 |
| Hc595(SimpleSpi)                | 257724/27060 |  1024/  276 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               | 257868/27028 |  1168/  244 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                | 258804/27044 |  2104/  260 |
| Max7219(SimpleSpi)              | 257604/27028 |   904/  244 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                | 261348/27500 |  4648/  716 |
| Ht16k33(SimpleWire)             | 257980/27028 |  1280/  244 |
|---------------------------------+--------------+-------------|
| StubModule                      | 256776/26996 |    76/  212 |
| PatternWriter+Stub              | 256792/27004 |    92/  220 |
| NumberWriter+Stub               | 257272/27004 |   572/  220 |
| ClockWriter+Stub                | 257080/27004 |   380/  220 |
| TemperatureWriter+Stub          | 257384/27004 |   684/  220 |
| CharWriter+Stub                 | 257032/27012 |   332/  228 |
| StringWriter+Stub               | 257200/27012 |   500/  228 |
| StringScroller+Stub             | 257232/27020 |   532/  236 |
| LevelWriter+Stub                | 256904/27004 |   204/  220 |
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
| DirectModule                    | 200258/13568 |  2510/  484 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 | 202570/13632 |  4822/  548 |
| Hybrid(SimpleSpi)               | 200306/13576 |  2558/  492 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  | 202546/13632 |  4798/  548 |
| Hc595(SimpleSpi)                | 200210/13576 |  2462/  492 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               | 200466/13552 |  2718/  468 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                | 202456/13592 |  4708/  508 |
| Max7219(SimpleSpi)              | 200052/13536 |  2304/  452 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                | 209690/14280 | 11942/ 1196 |
| Ht16k33(SimpleWire)             | 200526/13552 |  2778/  468 |
|---------------------------------+--------------+-------------|
| StubModule                      | 198940/13360 |  1192/  276 |
| PatternWriter+Stub              | 198956/13368 |  1208/  284 |
| NumberWriter+Stub               | 199328/13368 |  1580/  284 |
| ClockWriter+Stub                | 199276/13368 |  1528/  284 |
| TemperatureWriter+Stub          | 199444/13368 |  1696/  284 |
| CharWriter+Stub                 | 199212/13376 |  1464/  292 |
| StringWriter+Stub               | 199344/13376 |  1596/  292 |
| StringScroller+Stub             | 199364/13384 |  1616/  300 |
| LevelWriter+Stub                | 199068/13368 |  1320/  284 |
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
| baseline                        |  10880/ 4152 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |  11952/ 4396 |  1072/  244 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |  13024/ 4464 |  2144/  312 |
| Hybrid(SimpleSpi)               |  11984/ 4400 |  1104/  248 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |  12948/ 4468 |  2068/  316 |
| Hc595(SimpleSpi)                |  11920/ 4404 |  1040/  252 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi)               |  12568/ 4376 |  1688/  224 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |  13356/ 4436 |  2476/  284 |
| Max7219(SimpleSpi)              |  11868/ 4372 |   988/  220 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  14496/ 5036 |  3616/  884 |
| Ht16k33(SimpleWire)             |  13516/ 4376 |  2636/  224 |
|---------------------------------+--------------+-------------|
| StubModule                      |  10940/ 4360 |    60/  208 |
| PatternWriter+Stub              |  10956/ 4364 |    76/  212 |
| NumberWriter+Stub               |  11444/ 4364 |   564/  212 |
| ClockWriter+Stub                |  11076/ 4368 |   196/  216 |
| TemperatureWriter+Stub          |  11560/ 4364 |   680/  212 |
| CharWriter+Stub                 |  11136/ 4372 |   256/  220 |
| StringWriter+Stub               |  11280/ 4376 |   400/  224 |
| StringScroller+Stub             |  11284/ 4384 |   404/  232 |
| LevelWriter+Stub                |  11024/ 4364 |   144/  212 |
+--------------------------------------------------------------+

```

