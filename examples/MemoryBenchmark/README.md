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

**Version**: AceSegment v0.10.0

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
* Add memory usage for `Tm1637Module` using `SimpleTmi1637FastInterface` which
  uses `digitalWriteFast` library for AVR processors. Saves 662 - 776 bytes of
  flash on AVR processors compared to `Tm1637Module` using normal
  `SimpleTmi1637Interface`.
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
* Extract out `readAck()`, saving 10 bytes of flash for `SimpleTmi1637Interface` and
  6 bytes of flash for `SimpleTmi1637FastInterface`.
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
        * Tm1637Module(SimpleTmi1637) saves 90 bytes,
        * Ht16k33Module(SimpleWire) saves 68 bytes of flash,
        * Max7219Module(SimpleSpi) saves 30 bytes of flash.
    * On 32-bit processors, the flash consumption usually goes *up* by 4-20
      bytes, but decreases by a few bytes in a few cases.
    * The 32-bit processors have so much more flash memory than 8-bit
      processors, I think this tradeoff is worth it.

**v0.8.2**

* Remove `virtual` keyword from `LedModule` methods.
    * Decreases flash usage by 60 bytes for `Tm1637Module`, 14 bytes for
      `Max7219Module`, 32 bytes for `Ht16k33Module`, and 2-14 bytes for
      `Hc595Module`.
    * Decreases static ram usage by 7-8 bytes for all Module classes.
    * Further decreases flash usage by 10-70 bytes for various Writer classes.
* Templatize Writer classes on `T_LED_MODULE` instead of hardcoding it to
  `LedModule`.
    * Seems to reduce flash size of some Writer classes on some platforms by
      hundreds of bytes, I think because methods can be better inlined, and
      unused methods are not compiled and linked in.
* Add `isFlushRequired()` and clear appropriate flags after `flush()`.
    * Increases flash consumption by about 8 bytes on AVR.

**v0.9**

* Moved Writer classes to AceSegmentWriter library.

**v0.10**

* Add `beginTransmission()`, `endTransmission()`, `transfer()`, and
  `transfer16()` methods to AceSPI library, which become the building blocks for
  the `send8()` and `send16()` convenience functions.
    * Seems to increase flash usage by about 20 bytes on AVR for
    * `HardSpiInterface` and `HardSpiFastInterface`, even though nothing really
      changed functionally.
    * On 32-bit processors, no significant difference.
* Add benchmarks for `Tm1638Module`.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program for various LED modules.

* `ClockInterface`, `GpioInterface` (usually optimized away by the compiler)
* `SimpleSpiInterface`, `SimpleSpiFastInterface`, `HardSpiInterface`,
  `HardSpiFastInterface`
* `DirectModule`
* `DirectFast4Module`
* `HybridModule`
* `Hc595Module`
* `Tm1637Module`
* `Tm1638Module`
* `Max7219Module`
* `Ht16k33Module`

### ATtiny85

* 8MHz ATtiny85
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SpenceKonde/ATTinyCore 1.5.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    260/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   1296/   58 |  1036/   47 |
| DirectFast4Module               |   1042/   88 |   782/   77 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   1712/   60 |  1452/   49 |
| Hybrid(HardSpiFast)             |   1652/   58 |  1392/   47 |
| Hybrid(SimpleSpi)               |   1328/   53 |  1068/   42 |
| Hybrid(SimpleSpiFast)           |   1198/   48 |   938/   37 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   1638/   59 |  1378/   48 |
| Hc595(HardSpiFast)              |   1386/   58 |  1126/   47 |
| Hc595(SimpleSpi)                |   1270/   53 |  1010/   42 |
| Hc595(SimpleSpiFast)            |    868/   48 |   608/   37 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |   1132/   31 |   872/   20 |
| Tm1637(SimpleTmi1637Fast)       |    570/   26 |   310/   15 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |    968/   32 |   708/   21 |
| Tm1638(SimpleTmi1638Fast)       |    512/   25 |   252/   14 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |   1304/   43 |  1044/   32 |
| Max7219(HardSpiFast)            |   1052/   41 |   792/   30 |
| Max7219(SimpleSpi)              |    960/   37 |   700/   26 |
| Max7219(SimpleSpiFast)          |    546/   32 |   286/   21 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   1302/   69 |  1042/   58 |
| Ht16k33(SimpleWire)             |   1174/   33 |   914/   22 |
| Ht16k33(SimpleWireFast)         |    656/   27 |   396/   16 |
+--------------------------------------------------------------+

```

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Arduino AVR Boards 1.8.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   1534/   58 |  1078/   47 |
| DirectFast4Module               |   1290/   88 |   834/   77 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   1612/   56 |  1156/   45 |
| Hybrid(HardSpiFast)             |   1564/   54 |  1108/   43 |
| Hybrid(SimpleSpi)               |   1560/   53 |  1104/   42 |
| Hybrid(SimpleSpiFast)           |   1432/   48 |   976/   37 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   1552/   55 |  1096/   44 |
| Hc595(HardSpiFast)              |   1526/   54 |  1070/   43 |
| Hc595(SimpleSpi)                |   1492/   53 |  1036/   42 |
| Hc595(SimpleSpiFast)            |   1094/   48 |   638/   37 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |   1424/   31 |   968/   20 |
| Tm1637(SimpleTmi1637Fast)       |    858/   26 |   402/   15 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |   1264/   32 |   808/   21 |
| Tm1638(SimpleTmi1638Fast)       |    762/   25 |   306/   14 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |   1240/   39 |   784/   28 |
| Max7219(HardSpiFast)            |   1232/   37 |   776/   26 |
| Max7219(SimpleSpi)              |   1182/   37 |   726/   26 |
| Max7219(SimpleSpiFast)          |    764/   32 |   308/   21 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   2856/  243 |  2400/  232 |
| Ht16k33(SimpleWire)             |   1458/   33 |  1002/   22 |
| Ht16k33(SimpleWireFast)         |    926/   27 |   470/   16 |
+--------------------------------------------------------------+

```

### Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   3472/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   4530/  198 |  1058/   47 |
| DirectFast4Module               |   4172/  228 |   700/   77 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   4608/  196 |  1136/   45 |
| Hybrid(HardSpiFast)             |   4560/  194 |  1088/   43 |
| Hybrid(SimpleSpi)               |   4556/  193 |  1084/   42 |
| Hybrid(SimpleSpiFast)           |   4428/  188 |   956/   37 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   4570/  195 |  1098/   44 |
| Hc595(HardSpiFast)              |   4534/  194 |  1062/   43 |
| Hc595(SimpleSpi)                |   4510/  193 |  1038/   42 |
| Hc595(SimpleSpiFast)            |   3998/  188 |   526/   37 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |   4516/  171 |  1044/   20 |
| Tm1637(SimpleTmi1637Fast)       |   3836/  166 |   364/   15 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |   4356/  172 |   884/   21 |
| Tm1638(SimpleTmi1638Fast)       |   3740/  165 |   268/   14 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |   4310/  179 |   838/   28 |
| Max7219(HardSpiFast)            |   4290/  177 |   818/   26 |
| Max7219(SimpleSpi)              |   4252/  177 |   780/   26 |
| Max7219(SimpleSpiFast)          |   3720/  172 |   248/   21 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |   5840/  383 |  2368/  232 |
| Ht16k33(SimpleWire)             |   4550/  173 |  1078/   22 |
| Ht16k33(SimpleWireFast)         |   3902/  167 |   430/   16 |
+--------------------------------------------------------------+

```

### STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* STM32duino 2.2.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  21880/ 3540 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |  24696/ 3960 |  2816/  420 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |  26608/ 3972 |  4728/  432 |
| Hybrid(SimpleSpi)               |  24760/ 3964 |  2880/  424 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |  26572/ 3976 |  4692/  436 |
| Hc595(SimpleSpi)                |  24688/ 3968 |  2808/  428 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |  24816/ 3940 |  2936/  400 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |  24564/ 3940 |  2684/  400 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |  26452/ 3944 |  4572/  404 |
| Max7219(SimpleSpi)              |  24560/ 3936 |  2680/  396 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  28972/ 4124 |  7092/  584 |
| Ht16k33(SimpleWire)             |  24868/ 3936 |  2988/  396 |
+--------------------------------------------------------------+

```

### ESP8266

* NodeMCU 1.0, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP8266 Boards 3.0.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 260089/27892 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    | 261537/28260 |  1448/  368 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 | 262681/28260 |  2592/  368 |
| Hybrid(SimpleSpi)               | 261625/28244 |  1536/  352 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  | 262661/28272 |  2572/  380 |
| Hc595(SimpleSpi)                | 261509/28256 |  1420/  364 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           | 261625/28224 |  1536/  332 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           | 261385/28224 |  1296/  332 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                | 262541/28240 |  2452/  348 |
| Max7219(SimpleSpi)              | 261389/28224 |  1300/  332 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                | 264381/28592 |  4292/  700 |
| Ht16k33(SimpleWire)             | 261705/28224 |  1616/  332 |
+--------------------------------------------------------------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP32 Boards 2.0.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 204501/16060 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    | 208477/16532 |  3976/  472 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 | 210681/16596 |  6180/  536 |
| Hybrid(SimpleSpi)               | 208521/16540 |  4020/  480 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  | 210665/16596 |  6164/  536 |
| Hc595(SimpleSpi)                | 208425/16540 |  3924/  480 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           | 208685/16516 |  4184/  456 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           | 208421/16516 |  3920/  456 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                | 210525/16564 |  6024/  504 |
| Max7219(SimpleSpi)              | 208281/16508 |  3780/  448 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                | 232077/17228 | 27576/ 1168 |
| Ht16k33(SimpleWire)             | 208765/16516 |  4264/  456 |
+--------------------------------------------------------------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  10216/ 4152 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |  11296/ 4396 |  1080/  244 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |  12372/ 4464 |  2156/  312 |
| Hybrid(SimpleSpi)               |  11756/ 4400 |  1540/  248 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |  12296/ 4468 |  2080/  316 |
| Hc595(SimpleSpi)                |  11696/ 4404 |  1480/  252 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |  11904/ 4376 |  1688/  224 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |  11456/ 4376 |  1240/  224 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |  12700/ 4436 |  2484/  284 |
| Max7219(SimpleSpi)              |  11212/ 4372 |   996/  220 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  14128/ 5196 |  3912/ 1044 |
| Ht16k33(SimpleWire)             |  12856/ 4376 |  2640/  224 |
+--------------------------------------------------------------+

```

