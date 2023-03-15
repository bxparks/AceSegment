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

**Version**: AceSegment v0.13.0

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

**v0.12**

* Add `Tm1638AnodeModule`. Very similar to `Tm1638Module`.

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
* `Tm1638AnodeModule`
* `Max7219Module`
* `Ht16k33Module`

### ATtiny85

* 8MHz ATtiny85
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* SpenceKonde/ATTinyCore 1.5.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    260/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   1310/   58 |  1050/   47 |
| DirectFast4Module               |   1056/   88 |   796/   77 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   1710/   60 |  1450/   49 |
| Hybrid(HardSpiFast)             |   1658/   58 |  1398/   47 |
| Hybrid(SimpleSpi)               |   1342/   53 |  1082/   42 |
| Hybrid(SimpleSpiFast)           |   1212/   48 |   952/   37 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   1656/   59 |  1396/   48 |
| Hc595(HardSpiFast)              |   1404/   58 |  1144/   47 |
| Hc595(SimpleSpi)                |   1284/   53 |  1024/   42 |
| Hc595(SimpleSpiFast)            |    886/   48 |   626/   37 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |   1130/   31 |   870/   20 |
| Tm1637(SimpleTmi1637Fast)       |    568/   26 |   308/   15 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |   1086/   32 |   826/   21 |
| Tm1638(SimpleTmi1638Fast)       |    568/   25 |   308/   14 |
| Tm1638Anode(SimpleTmi1638)      |   1080/   30 |   820/   19 |
| Tm1638Anode(SimpleTmi1638Fast)  |    562/   23 |   302/   12 |
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
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* Arduino AVR Boards 1.8.5

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   1548/   58 |  1092/   47 |
| DirectFast4Module               |   1304/   88 |   848/   77 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   1626/   56 |  1170/   45 |
| Hybrid(HardSpiFast)             |   1578/   54 |  1122/   43 |
| Hybrid(SimpleSpi)               |   1574/   53 |  1118/   42 |
| Hybrid(SimpleSpiFast)           |   1446/   48 |   990/   37 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   1566/   55 |  1110/   44 |
| Hc595(HardSpiFast)              |   1542/   54 |  1086/   43 |
| Hc595(SimpleSpi)                |   1506/   53 |  1050/   42 |
| Hc595(SimpleSpiFast)            |   1110/   48 |   654/   37 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |   1422/   31 |   966/   20 |
| Tm1637(SimpleTmi1637Fast)       |    856/   26 |   400/   15 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |   1372/   32 |   916/   21 |
| Tm1638(SimpleTmi1638Fast)       |    842/   25 |   386/   14 |
| Tm1638Anode(SimpleTmi1638)      |   1368/   30 |   912/   19 |
| Tm1638Anode(SimpleTmi1638Fast)  |    838/   23 |   382/   12 |
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
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   3472/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |   4544/  198 |  1072/   47 |
| DirectFast4Module               |   4186/  228 |   714/   77 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |   4622/  196 |  1150/   45 |
| Hybrid(HardSpiFast)             |   4574/  194 |  1102/   43 |
| Hybrid(SimpleSpi)               |   4570/  193 |  1098/   42 |
| Hybrid(SimpleSpiFast)           |   4442/  188 |   970/   37 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |   4584/  195 |  1112/   44 |
| Hc595(HardSpiFast)              |   4550/  194 |  1078/   43 |
| Hc595(SimpleSpi)                |   4524/  193 |  1052/   42 |
| Hc595(SimpleSpiFast)            |   4014/  188 |   542/   37 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |   4514/  171 |  1042/   20 |
| Tm1637(SimpleTmi1637Fast)       |   3834/  166 |   362/   15 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |   4464/  172 |   992/   21 |
| Tm1638(SimpleTmi1638Fast)       |   3820/  165 |   348/   14 |
| Tm1638Anode(SimpleTmi1638)      |   4460/  170 |   988/   19 |
| Tm1638Anode(SimpleTmi1638Fast)  |   3816/  163 |   344/   12 |
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
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* STM32duino 2.4.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  21392/ 3556 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |  26376/ 3980 |  4984/  424 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |  26540/ 3992 |  5148/  436 |
| Hybrid(SimpleSpi)               |  26440/ 3984 |  5048/  428 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |  26456/ 3996 |  5064/  440 |
| Hc595(SimpleSpi)                |  26368/ 3988 |  4976/  432 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |  26484/ 3960 |  5092/  404 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |  26348/ 3960 |  4956/  404 |
| Tm1638Anode(SimpleTmi1638)      |  26372/ 3956 |  4980/  400 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |  26324/ 3964 |  4932/  408 |
| Max7219(SimpleSpi)              |  26228/ 3956 |  4836/  400 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  32880/ 4192 | 11488/  636 |
| Ht16k33(SimpleWire)             |  26536/ 3956 |  5144/  400 |
+--------------------------------------------------------------+

```

### ESP8266

* NodeMCU 1.0, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
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
| Tm1638(SimpleTmi1638)           | 261529/28224 |  1440/  332 |
| Tm1638Anode(SimpleTmi1638)      | 261561/28224 |  1472/  332 |
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
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* ESP32 Boards 2.0.7

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 228345/21976 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    | 239665/22328 | 11320/  352 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 | 241557/22392 | 13212/  416 |
| Hybrid(SimpleSpi)               | 239713/22336 | 11368/  360 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  | 241537/22392 | 13192/  416 |
| Hc595(SimpleSpi)                | 239617/22336 | 11272/  360 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           | 239897/22312 | 11552/  336 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           | 239761/22312 | 11416/  336 |
| Tm1638Anode(SimpleTmi1638)      | 239773/22304 | 11428/  328 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                | 241413/22360 | 13068/  384 |
| Max7219(SimpleSpi)              | 239485/22304 | 11140/  328 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                | 259441/22656 | 31096/  680 |
| Ht16k33(SimpleWire)             | 239977/22304 | 11632/  328 |
+--------------------------------------------------------------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.31.0
* Teensyduino 1.57
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  10092/ 4152 |     0/    0 |
|---------------------------------+--------------+-------------|
| DirectModule                    |  11176/ 4396 |  1084/  244 |
|---------------------------------+--------------+-------------|
| Hybrid(HardSpi)                 |  12248/ 4464 |  2156/  312 |
| Hybrid(SimpleSpi)               |  11636/ 4400 |  1544/  248 |
|---------------------------------+--------------+-------------|
| Hc595(HardSpi)                  |  12172/ 4468 |  2080/  316 |
| Hc595(SimpleSpi)                |  11572/ 4404 |  1480/  252 |
|---------------------------------+--------------+-------------|
| Tm1637(SimpleTmi1637)           |  11780/ 4376 |  1688/  224 |
|---------------------------------+--------------+-------------|
| Tm1638(SimpleTmi1638)           |  11536/ 4376 |  1444/  224 |
| Tm1638Anode(SimpleTmi1638)      |  11568/ 4372 |  1476/  220 |
|---------------------------------+--------------+-------------|
| Max7219(HardSpi)                |  12576/ 4436 |  2484/  284 |
| Max7219(SimpleSpi)              |  11088/ 4372 |   996/  220 |
|---------------------------------+--------------+-------------|
| Ht16k33(TwoWire)                |  14004/ 5196 |  3912/ 1044 |
| Ht16k33(SimpleWire)             |  12732/ 4376 |  2640/  224 |
+--------------------------------------------------------------+

```

