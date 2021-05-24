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

**Version**: AceSegment v0.5+

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

**v0.5+**

* Add support for multiple SPI buses in `HardSpiInterface` and
  `HardSpiFastInterface`. Increases flash memory by 10-30 bytes.
* Add benchmarks for `StringScroller` and `HorizontalLevelWriter`.

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
* `HorizontalLevelWriter`

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
| DirectModule                    |   1486/   64 |  1030/   53 |
| DirectFast4Module               |   1250/   94 |   794/   83 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |   1508/   58 |  1052/   47 |
| Hybrid(SoftSpiFast)             |   1400/   56 |   944/   45 |
| Hybrid(HardSpi)                 |   1588/   62 |  1132/   51 |
| Hybrid(HardSpiFast)             |   1510/   59 |  1054/   48 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |   1486/   58 |  1030/   47 |
| Hc595(SoftSpiFast)              |   1076/   56 |   620/   45 |
| Hc595(HardSpi)                  |   1574/   62 |  1118/   51 |
| Hc595(HardSpiFast)              |   1478/   59 |  1022/   48 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |   1582/   39 |  1126/   28 |
| Tm1637(SoftTmiFast)             |    926/   36 |   470/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |   1214/   44 |   758/   33 |
| Max7219(SoftSpiFast)            |    774/   42 |   318/   31 |
| Max7219(HardSpi)                |   1322/   48 |   866/   37 |
| Max7219(HardSpiFast)            |   1084/   45 |   628/   34 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |    578/   24 |   122/   13 |
| NumberWriter+Stub               |    682/   28 |   226/   17 |
| ClockWriter+Stub                |    766/   29 |   310/   18 |
| TemperatureWriter+Stub          |    764/   28 |   308/   17 |
| CharWriter+Stub                 |    788/   31 |   332/   20 |
| StringWriter+Stub               |    988/   39 |   532/   28 |
| StringScroller+Stub             |   1036/   45 |   580/   34 |
| HorizontalLevelWriter+Stub      |    716/   28 |   260/   17 |
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
| Hybrid(HardSpi)                 |   4584/  202 |  1112/   51 |
| Hybrid(HardSpiFast)             |   4506/  199 |  1034/   48 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |   4482/  198 |  1010/   47 |
| Hc595(SoftSpiFast)              |   3958/  196 |   486/   45 |
| Hc595(HardSpi)                  |   4570/  202 |  1098/   51 |
| Hc595(HardSpiFast)              |   4462/  199 |   990/   48 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |   4652/  179 |  1180/   28 |
| Tm1637(SoftTmiFast)             |   3882/  176 |   410/   25 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |   4284/  184 |   812/   33 |
| Max7219(SoftSpiFast)            |   3730/  182 |   258/   31 |
| Max7219(HardSpi)                |   4392/  188 |   920/   37 |
| Max7219(HardSpiFast)            |   4142/  185 |   670/   34 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |   3534/  164 |    62/   13 |
| NumberWriter+Stub               |   3638/  168 |   166/   17 |
| ClockWriter+Stub                |   3722/  169 |   250/   18 |
| TemperatureWriter+Stub          |   3720/  168 |   248/   17 |
| CharWriter+Stub                 |   3744/  171 |   272/   20 |
| StringWriter+Stub               |   3944/  179 |   472/   28 |
| StringScroller+Stub             |   3992/  185 |   520/   34 |
| HorizontalLevelWriter+Stub      |   3672/  168 |   200/   17 |
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
| DirectModule                    |  10816/    0 |   752/    0 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |  10864/    0 |   800/    0 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 |  11304/    0 |  1240/    0 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |  10760/    0 |   696/    0 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  |  11280/    0 |  1216/    0 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |  10808/    0 |   744/    0 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  10616/    0 |   552/    0 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  11136/    0 |  1072/    0 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  10336/    0 |   272/    0 |
| NumberWriter+Stub               |  10672/    0 |   608/    0 |
| ClockWriter+Stub                |  10480/    0 |   416/    0 |
| TemperatureWriter+Stub          |  10736/    0 |   672/    0 |
| CharWriter+Stub                 |  10544/    0 |   480/    0 |
| StringWriter+Stub               |  10728/    0 |   664/    0 |
| StringScroller+Stub             |  10672/    0 |   608/    0 |
| HorizontalLevelWriter+Stub      |  10424/    0 |   360/    0 |
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
| DirectModule                    |  21536/ 4392 |  2400/  604 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |  21592/ 4396 |  2456/  608 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 |  23344/ 4400 |  4208/  612 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |  21508/ 4400 |  2372/  612 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  |  23300/ 4404 |  4164/  616 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |  21628/ 4372 |  2492/  584 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  21404/ 4372 |  2268/  584 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  23200/ 4376 |  4064/  588 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  19328/ 4340 |   192/  552 |
| NumberWriter+Stub               |  19616/ 4344 |   480/  556 |
| ClockWriter+Stub                |  19496/ 4348 |   360/  560 |
| TemperatureWriter+Stub          |  19712/ 4344 |   576/  556 |
| CharWriter+Stub                 |  19536/ 4352 |   400/  564 |
| StringWriter+Stub               |  19708/ 4356 |   572/  568 |
| StringScroller+Stub             |  19660/ 4364 |   524/  576 |
| HorizontalLevelWriter+Stub      |  19408/ 4344 |   272/  556 |
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
| DirectModule                    | 257804/27260 |  1104/  476 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 | 257876/27244 |  1176/  460 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 | 258980/27260 |  2280/  476 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  | 257776/27256 |  1076/  472 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  | 258960/27264 |  2260/  480 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 | 257920/27224 |  1220/  440 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                | 257656/27224 |   956/  440 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                | 258856/27240 |  2156/  456 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           | 256876/27200 |   176/  416 |
| NumberWriter+Stub               | 257372/27200 |   672/  416 |
| ClockWriter+Stub                | 257196/27208 |   496/  424 |
| TemperatureWriter+Stub          | 257484/27200 |   784/  416 |
| CharWriter+Stub                 | 257116/27208 |   416/  424 |
| StringWriter+Stub               | 257364/27216 |   664/  432 |
| StringScroller+Stub             | 257316/27224 |   616/  440 |
| HorizontalLevelWriter+Stub      | 257020/27200 |   320/  416 |
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
| DirectModule                    | 200506/13760 |  2758/  676 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 | 200546/13768 |  2798/  684 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 | 202842/13816 |  5094/  732 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  | 200442/13768 |  2694/  684 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  | 202818/13824 |  5070/  740 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 | 200702/13744 |  2954/  660 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                | 200308/13728 |  2560/  644 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                | 202700/13784 |  4952/  700 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           | 199228/13560 |  1480/  476 |
| NumberWriter+Stub               | 199616/13560 |  1868/  476 |
| ClockWriter+Stub                | 199560/13568 |  1812/  484 |
| TemperatureWriter+Stub          | 199736/13560 |  1988/  476 |
| CharWriter+Stub                 | 199500/13568 |  1752/  484 |
| StringWriter+Stub               | 199696/13576 |  1948/  492 |
| StringScroller+Stub             | 199648/13584 |  1900/  500 |
| HorizontalLevelWriter+Stub      | 199412/13560 |  1664/  476 |
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
| DirectModule                    |  11900/ 4584 |  4276/ 1536 |
| DirectFast4Module               |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hybrid(SoftSpi)                 |  11944/ 4588 |  4320/ 1540 |
| Hybrid(SoftSpiFast)             |     -1/   -1 |    -1/   -1 |
| Hybrid(HardSpi)                 |  12988/ 4648 |  5364/ 1600 |
| Hybrid(HardSpiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Hc595(SoftSpi)                  |  11880/ 4592 |  4256/ 1544 |
| Hc595(SoftSpiFast)              |     -1/   -1 |    -1/   -1 |
| Hc595(HardSpi)                  |  12928/ 4652 |  5304/ 1604 |
| Hc595(HardSpiFast)              |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Tm1637(SoftTmi)                 |  12556/ 4564 |  4932/ 1516 |
| Tm1637(SoftTmiFast)             |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| Max7219(SoftSpi)                |  11832/ 4564 |  4208/ 1516 |
| Max7219(SoftSpiFast)            |     -1/   -1 |    -1/   -1 |
| Max7219(HardSpi)                |  13296/ 4624 |  5672/ 1576 |
| Max7219(HardSpiFast)            |     -1/   -1 |    -1/   -1 |
|---------------------------------+--------------+-------------|
| StubModule+LedDisplay           |  10924/ 4552 |  3300/ 1504 |
| NumberWriter+Stub               |  11400/ 4556 |  3776/ 1508 |
| ClockWriter+Stub                |  11112/ 4560 |  3488/ 1512 |
| TemperatureWriter+Stub          |  11556/ 4556 |  3932/ 1508 |
| CharWriter+Stub                 |  11124/ 4564 |  3500/ 1516 |
| StringWriter+Stub               |  11328/ 4568 |  3704/ 1520 |
| StringScroller+Stub             |  11324/ 4576 |  3700/ 1528 |
| HorizontalLevelWriter+Stub      |  11076/ 4556 |  3452/ 1508 |
+--------------------------------------------------------------+

```

