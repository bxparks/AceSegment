# AutoBenchmark

This program creates instances of various subclasses `LedModule` using different
configurations:

* `DirectModule`: group and segment pins directly connected to MCU
* `DirectFast4Module`: same as `DirectModule` but using `digitalWriteFast` library
* `HybridModule`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through SPI
* `Hc595Module`: group pins and segment pins connected to two 74HC595
  which are accessed through SPI
* `Tm1637Module`: an LED module using a TM1637 driver chip, accessed through
  a two-wire protocol similar to I2C
* `Max7219Module`: an LED module using a MAX7219 driver chip, accessed through
  SPI

We then measure the time taken by the methods used to render the digit to the
LED module:

* Subclasses of `ScanningModule` (`Hc595Module`, `HybridModule`, `DirectModule`)
    * Measure the time taken by `ScanningModule::renderFieldNow()` which renders
      a single digit (multiple fields make up a frame (a single frame is the
      complete rendering of all digits on the display module).
* `Tm1637Module::flush()
    * Sends out the buffered digits over the custom two-wire protocol.
* `Max7219Module::flush()`
    * Sends out the buffered digits using SPI.

**Version**: AceSegment v0.7

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceCommon](https://github.com/bxparks/AceCommon)
* [AceSegment](https://github.com/bxparks/AceButton)

On AVR processors, the following library is required to run the
`digitalWriteFast()` versions of the low-level drivers:

* [digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast)

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

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

The CPU times below are given in microseconds. The "samples" column is the
number of `TimingStats::update()` calls that were made.

## CPU Time Changes

**v0.4:**
* Huge refactoring of core AceSegment classes. Rewrote AutoBenchmark
  to match similar programs in the AceButton, AceCrc and AceTime libraries.

**v0.5:**

* Add benchmarks for `Tm1637Module`.
    * The CPU time is mostly determined by the calls to `delayMicroseconds()`,
      which is must be about 100 microseconds due to the unusually large
      capacitors (10 nF) installed on the DIO and CLK lines. They should have
      been about 100X smaller (200 pF).
    * Benchmarks both 4-digit and 6-digit LED modules given separately
      because the `Tm1637::flush()` method is roughtly proportional to the
      number of digits.
    * Benchmarks for `Tm1637::flushIncremental()` is independent of the total
      number of digits because only a single digit is sent for each call.
      The maximum time for a single call to `flushIncremental()` is about 10 ms.
    * For a 6-digit module, `flush()` can take as much as 27-30 milliseconds (!)
      which uncomfortably close to the maximum amount of time before an ESP8266
      resets via the watch dog timer. On ESP8266 (and maybe others), the
      `flushIncremental()` should be used.
* Add benchmarks for `DirectModule`, `DirectFast4Module`, `HybridModule` with 4
  digits, because these are custom made by hand.
* Add `Max7219Module` with 8 digits as found on off-the-shelf LED modules.
  Duration of `flush()` almost doubles from 4 digits to 8 digits obviously.
* Add `Hc595Module` with 8 digits as found on off-the-shelf LED modules.
  Duration of `renderFieldWhenReady()` does not change from 4 digits to 8 digits
  because it renders a single digit at time.
* Upgrade from ESP32 Core v1.0.4 to v1.0.6.
* Adding `byteOrder` and `remapArray` parameters to `Hc595Module` increases
  the CPU time of `renderFieldsNow()` by a tiny amount, maybe a microsecond on a
  AVR. For 32-bit processors, the difference seems to be within the noise.

**v0.6:**

* `HardSpiInterface` is slightly slower on the fastest processors (e.g. ESP8266,
  ESP32, Teensy 3.2), because the SPI frequency was reduced from 20 MHz to 8
  MHz. No difference on the slower processors. Those fast processors can
  actually sustain a 20 MHz SPI, which breaks the MAX7219 chip because it can
  handle only 16 MHz.
* Verified that removing the 10 nF capacitors from the CLK and DIO lines of the
  TM1637 modules allows a much shorter `BIT_DELAY`. Added CPU benchmark numbers
  for `BIT_DELAY = 5` microseconds (e.g. `Tm1637(4,SoftTmi,5us)` and
  `Tm1637(4,SoftTmiFast,5us)`). The `flush()` or `flushIncremental()` durations
  are almost a factor of 10X to 20X shorter compared to `BIT_DELAY = 100`.

**v0.7:**

* Add benchmarks for `Ht16k33Module`, using `TwoWireInterface`,
  `SimpleWireInterface`, and `SimpleWireFastInterface`.
    * On SAMD21 and STM32, the runtime of the `Ht16k33Module(TwoWire)` seems to
      depend on whether an actual HT16K33 LED module is attached to the I2C bus.
        * SAMD21: the transmission time becomes 50X longer *without* the LED
          module attached. 
        * STM32, the transmission time becomes 30-40X shorter *without* the LED
          module attached.
        * I think this is because the `<Wire.h>` library on the SAMD21 and STM32
          support clock stretching. If the SDA and SCL pins are floating, then
          the library incorrectly thinks that the slave device is holding the
          clock line. (Not sure what happens on the STM32 which causes it to
          perform so much faster.)
        * Fixed by adding 10k pullup resistors on the SDA and SCL lines on the
          dev boards that run AutoBenchmark. Prevents clock stretching even if
          no LED module is actually attached to the pins.

## Results

The following tables show the number of microseconds taken by:

* `DirectModule::renderFieldNow()`, `HybridModule::renderFieldNow()`,
  `Hc595Module::renderFieldNow()`
    * renders the 8 segments of a single LED digit.
    * If the LED module has 4 digits, then `renderFieldNow()` must be called 4
      times to render the light pattern of the entire LED module. The entire
      rendering is then called a frame.
    * Most people can no longer see flickering of the display at about 60 frames
      a second. To achieve that, the `renderFieldNow()` method must be called
      240 times a second for a module with 4 digits, or every 4.17 milliseconds.
    * The results below show that every processor, even the slowest AVR
      processor, is able to meet this threshhold.
* `Tm1637Module::flush()` or `Tm1637Module::flushIncremental()`
    * sends digits in the buffer to the TM1637 LED module using the I2C-like
      protocol
    * results for two values of `BIT_DELAY` are collected
        * 100 microseconds (e.g. `Tm1637(4,SoftTmi)`)
        * 5 microseconds (e.g. `Tm1637(4,SoftTmi,5us)`)
* `Max7219Module::flush()`
    * sends all digits in the buffer to the MAX7219 LED module using
      software SPI or hardware SPI
* `Ht16k33Module::flush()`
    * sends all digits in the buffer to the HT16K33 LED module using
      hardware I2C

On AVR processors, the "fast" options are available using the
[digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast) library whose
`digitalWriteFast()` functions can be up to 50X faster if the `pin` number and
`value` parameters are compile-time constants. In addition, the
`digitalWriteFast` functions reduce flash memory consumption by 600-700 bytes
for `SoftTmiFastInterface`, `SoftSpiFastInterface`, and `HardSpiFastInterface`
compared to their non-fast equivalents.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SoftTmiInterface): 3
sizeof(SoftTmiFastInterface<4, 5, 100>): 1
sizeof(SoftSpiInterface): 3
sizeof(SoftSpiFastInterface<11, 12, 13>): 1
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface): 2
sizeof(TwoWireInterface): 3
sizeof(SimpleWireInterface): 4
sizeof(SimpleWireFastInterface<2, 3, 10>): 1
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 8
sizeof(LedMatrixDualHc595<HardSpiInterface>): 8
sizeof(LedModule): 3
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(DirectModule<4>): 31
sizeof(DirectFast4Module<...>): 25
sizeof(HybridModule<SoftSpiInterface, 4>): 30
sizeof(Hc595Module<SoftSpiInterface, 8>): 46
sizeof(Tm1637Module<SoftTmiInterface, 4>): 14
sizeof(Tm1637Module<SoftTmiInterface, 6>): 16
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(Ht16k33Module<TwoWireInterface, 4>): 11
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 11
sizeof(PatternWriter): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 5
sizeof(StringWriter): 2
sizeof(LevelWriter): 2
sizeof(StringScroller): 8

CPU:
+-----------------------------------------+-------------------+---------+
| Functionality                           |   min/  avg/  max | samples |
|-----------------------------------------+-------------------+---------|
| Direct(4)                               |    76/   82/   88 |      40 |
| Direct(4,subfields)                     |     4/   13/   88 |     640 |
| DirectFast4(4)                          |    28/   30/   36 |      40 |
| DirectFast4(4,subfields)                |     4/    8/   36 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |   152/  161/  180 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     4/   22/  176 |     640 |
| Hybrid(4,SoftSpiFast)                   |    28/   35/   40 |      40 |
| Hybrid(4,SoftSpiFast,subfields)         |     4/    8/   40 |     640 |
| Hybrid(4,HardSpi)                       |    36/   41/   48 |      40 |
| Hybrid(4,HardSpi,subfields)             |     4/    8/   44 |     640 |
| Hybrid(4,HardSpiFast)                   |    24/   28/   40 |      40 |
| Hybrid(4,HardSpiFast,subfields)         |     4/    7/   36 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |   268/  272/  308 |      80 |
| Hc595(8,SoftSpi,subfields)              |     4/   36/  308 |    1280 |
| Hc595(8,SoftSpiFast)                    |    24/   27/   36 |      80 |
| Hc595(8,SoftSpiFast,subfields)          |     4/    8/   32 |    1280 |
| Hc595(8,HardSpi)                        |    24/   29/   36 |      80 |
| Hc595(8,HardSpi,subfields)              |     4/    8/   36 |    1280 |
| Hc595(8,HardSpiFast)                    |    12/   17/   28 |      80 |
| Hc595(8,HardSpiFast,subfields)          |     4/    6/   24 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22316/22346/22568 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3612/ 8810/10344 |      50 |
| Tm1637(4,SoftTmiFast,100us)             | 21068/21099/21356 |      10 |
| Tm1637(4,SoftTmiFast,100us,incremental) |  3412/ 8317/ 9812 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2252/ 2288/ 2492 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   364/  896/ 1128 |      50 |
| Tm1637(4,SoftTmiFast,5us)               |   996/ 1031/ 1104 |      10 |
| Tm1637(4,SoftTmiFast,5us,incremental)   |   164/  404/  508 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 28060/28095/28372 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3612/ 9178/10304 |      70 |
| Tm1637(6,SoftTmiFast,100us)             | 26488/26517/26744 |      10 |
| Tm1637(6,SoftTmiFast,100us,incremental) |  3412/ 8665/ 9760 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |  2380/ 2394/ 2592 |      20 |
| Max7219(8,SoftSpiFast)                  |   208/  218/  240 |      20 |
| Max7219(8,HardSpi)                      |   208/  221/  240 |      20 |
| Max7219(8,HardSpiFast)                  |    96/  106/  116 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   336/  344/  356 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  3744/ 3751/ 3784 |      20 |
| Ht16k33(4,SimpleWireFast,4us)           |  1444/ 1451/ 1500 |      20 |
+-----------------------------------------+-------------------+---------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SoftTmiInterface): 3
sizeof(SoftTmiFastInterface<4, 5, 100>): 1
sizeof(SoftSpiInterface): 3
sizeof(SoftSpiFastInterface<11, 12, 13>): 1
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface): 2
sizeof(TwoWireInterface): 3
sizeof(SimpleWireInterface): 4
sizeof(SimpleWireFastInterface<2, 3, 10>): 1
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 8
sizeof(LedMatrixDualHc595<HardSpiInterface>): 8
sizeof(LedModule): 3
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(DirectModule<4>): 31
sizeof(DirectFast4Module<...>): 25
sizeof(HybridModule<SoftSpiInterface, 4>): 30
sizeof(Hc595Module<SoftSpiInterface, 8>): 46
sizeof(Tm1637Module<SoftTmiInterface, 4>): 14
sizeof(Tm1637Module<SoftTmiInterface, 6>): 16
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(Ht16k33Module<TwoWireInterface, 4>): 11
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 11
sizeof(PatternWriter): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 5
sizeof(StringWriter): 2
sizeof(LevelWriter): 2
sizeof(StringScroller): 8

CPU:
+-----------------------------------------+-------------------+---------+
| Functionality                           |   min/  avg/  max | samples |
|-----------------------------------------+-------------------+---------|
| Direct(4)                               |    72/   78/   88 |      40 |
| Direct(4,subfields)                     |     4/   13/   84 |     640 |
| DirectFast4(4)                          |    24/   28/   36 |      40 |
| DirectFast4(4,subfields)                |     4/    8/   32 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |   148/  152/  164 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     4/   21/  160 |     640 |
| Hybrid(4,SoftSpiFast)                   |    28/   33/   40 |      40 |
| Hybrid(4,SoftSpiFast,subfields)         |     4/    8/   40 |     640 |
| Hybrid(4,HardSpi)                       |    36/   40/   48 |      40 |
| Hybrid(4,HardSpi,subfields)             |     4/    9/   48 |     640 |
| Hybrid(4,HardSpiFast)                   |    24/   27/   36 |      40 |
| Hybrid(4,HardSpiFast,subfields)         |     4/    7/   32 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |   252/  255/  264 |      80 |
| Hc595(8,SoftSpi,subfields)              |     4/   35/  268 |    1280 |
| Hc595(8,SoftSpiFast)                    |    24/   25/   32 |      80 |
| Hc595(8,SoftSpiFast,subfields)          |     4/    8/   32 |    1280 |
| Hc595(8,HardSpi)                        |    28/   29/   36 |      80 |
| Hc595(8,HardSpi,subfields)              |     4/    8/   36 |    1280 |
| Hc595(8,HardSpiFast)                    |    12/   15/   20 |      80 |
| Hc595(8,HardSpiFast,subfields)          |     4/    6/   20 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22440/22448/22456 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3628/ 8853/10168 |      50 |
| Tm1637(4,SoftTmiFast,100us)             | 21176/21184/21192 |      10 |
| Tm1637(4,SoftTmiFast,100us,incremental) |  3428/ 8354/ 9596 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2268/ 2272/ 2280 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   368/  899/ 1032 |      50 |
| Tm1637(4,SoftTmiFast,5us)               |  1008/ 1008/ 1012 |      10 |
| Tm1637(4,SoftTmiFast,5us,incremental)   |   164/  400/  468 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 28212/28221/28232 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3628/ 9226/10168 |      70 |
| Tm1637(6,SoftTmiFast,100us)             | 26624/26633/26644 |      10 |
| Tm1637(6,SoftTmiFast,100us,incremental) |  3432/ 8707/ 9596 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |  2244/ 2247/ 2260 |      20 |
| Max7219(8,SoftSpiFast)                  |   208/  211/  220 |      20 |
| Max7219(8,HardSpi)                      |   220/  226/  236 |      20 |
| Max7219(8,HardSpiFast)                  |    96/  100/  112 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   336/  340/  352 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  3764/ 3771/ 3776 |      20 |
| Ht16k33(4,SimpleWireFast,4us)           |  1452/ 1457/ 1464 |      20 |
+-----------------------------------------+-------------------+---------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.3

```
Sizes of Objects:
sizeof(SoftTmiInterface): 3
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 8
sizeof(TwoWireInterface): 8
sizeof(SimpleWireInterface): 4
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 24
sizeof(Tm1637Module<SoftTmiInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 20
sizeof(PatternWriter): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(LevelWriter): 4
sizeof(StringScroller): 12

CPU:
+-----------------------------------------+-------------------+---------+
| Functionality                           |   min/  avg/  max | samples |
|-----------------------------------------+-------------------+---------|
| Direct(4)                               |    24/   24/   25 |      40 |
| Direct(4,subfields)                     |     2/    5/   26 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    51/   51/   56 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     2/    8/   56 |     640 |
| Hybrid(4,HardSpi)                       |    24/   24/   28 |      40 |
| Hybrid(4,HardSpi,subfields)             |     2/    5/   28 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    88/   88/   93 |      80 |
| Hc595(8,SoftSpi,subfields)              |     3/   13/   93 |    1280 |
| Hc595(8,HardSpi)                        |    24/   24/   28 |      80 |
| Hc595(8,HardSpi,subfields)              |     3/    5/   29 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22210/22214/22219 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3595/ 8759/10055 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2108/ 2109/ 2112 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   341/  833/  956 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 27922/27925/27931 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3595/ 9128/10054 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   776/  781/  783 |      20 |
| Max7219(8,HardSpi)                      |   201/  202/  208 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   238/  238/  241 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  3345/ 3347/ 3352 |      20 |
+-----------------------------------------+-------------------+---------+

```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 2.0.0

```
Sizes of Objects:
sizeof(SoftTmiInterface): 3
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 8
sizeof(TwoWireInterface): 8
sizeof(SimpleWireInterface): 4
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 24
sizeof(Tm1637Module<SoftTmiInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 20
sizeof(PatternWriter): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(LevelWriter): 4
sizeof(StringScroller): 12

CPU:
+-----------------------------------------+-------------------+---------+
| Functionality                           |   min/  avg/  max | samples |
|-----------------------------------------+-------------------+---------|
| Direct(4)                               |    15/   15/   16 |      40 |
| Direct(4,subfields)                     |     1/    3/   18 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    42/   43/   47 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     1/    6/   52 |     640 |
| Hybrid(4,HardSpi)                       |    43/   43/   48 |      40 |
| Hybrid(4,HardSpi,subfields)             |     1/    6/   52 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    76/   76/   81 |      80 |
| Hc595(8,SoftSpi,subfields)              |     1/   10/   85 |    1280 |
| Hc595(8,HardSpi)                        |    44/   44/   49 |      80 |
| Hc595(8,HardSpi,subfields)              |     1/    6/   53 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22409/22409/22410 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3628/ 8837/10143 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2451/ 2456/ 2463 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   397/  969/ 1133 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 28168/28170/28175 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3628/ 9207/10141 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   688/  691/  693 |      20 |
| Max7219(8,HardSpi)                      |   394/  396/  399 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   230/  231/  235 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  4176/ 4185/ 4193 |      20 |
+-----------------------------------------+-------------------+---------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(SoftTmiInterface): 3
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 8
sizeof(TwoWireInterface): 8
sizeof(SimpleWireInterface): 4
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 24
sizeof(Tm1637Module<SoftTmiInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 20
sizeof(PatternWriter): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(LevelWriter): 4
sizeof(StringScroller): 12

CPU:
+-----------------------------------------+-------------------+---------+
| Functionality                           |   min/  avg/  max | samples |
|-----------------------------------------+-------------------+---------|
| Direct(4)                               |    12/   13/   34 |      40 |
| Direct(4,subfields)                     |     0/    2/   24 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    29/   29/   42 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     0/    4/   41 |     640 |
| Hybrid(4,HardSpi)                       |    12/   12/   28 |      40 |
| Hybrid(4,HardSpi,subfields)             |     0/    2/   24 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    50/   50/   62 |      80 |
| Hc595(8,SoftSpi,subfields)              |     0/    6/   63 |    1280 |
| Hc595(8,HardSpi)                        |    14/   14/   22 |      80 |
| Hc595(8,HardSpi,subfields)              |     0/    2/   26 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 21496/21509/21540 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3481/ 8480/ 9769 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  1525/ 1530/ 1561 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   247/  602/  701 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 27024/27050/27096 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3481/ 8838/ 9788 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   460/  460/  468 |      20 |
| Max7219(8,HardSpi)                      |   125/  126/  134 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   245/  246/  269 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  2514/ 2516/ 2534 |      20 |
+-----------------------------------------+-------------------+---------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
Sizes of Objects:
sizeof(SoftTmiInterface): 3
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 8
sizeof(TwoWireInterface): 8
sizeof(SimpleWireInterface): 4
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 24
sizeof(Tm1637Module<SoftTmiInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 20
sizeof(PatternWriter): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(LevelWriter): 4
sizeof(StringScroller): 12

CPU:
+-----------------------------------------+-------------------+---------+
| Functionality                           |   min/  avg/  max | samples |
|-----------------------------------------+-------------------+---------|
| Direct(4)                               |     2/    2/    8 |      40 |
| Direct(4,subfields)                     |     0/    1/    9 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |     4/    4/    8 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     0/    1/    9 |     640 |
| Hybrid(4,HardSpi)                       |     9/   10/   14 |      40 |
| Hybrid(4,HardSpi,subfields)             |     0/    1/   14 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |     7/    7/   15 |      80 |
| Hc595(8,SoftSpi,subfields)              |     0/    1/   16 |    1280 |
| Hc595(8,HardSpi)                        |    11/   11/   20 |      80 |
| Hc595(8,HardSpi,subfields)              |     0/    2/   20 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 21233/21241/21262 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3436/ 8374/ 9618 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  1276/ 1279/ 1285 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   205/  504/  587 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 26687/26695/26706 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3436/ 8727/ 9616 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |    60/   60/   64 |      20 |
| Max7219(8,HardSpi)                      |    90/   91/   99 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   310/  312/  321 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  1993/ 2001/ 2009 |      20 |
+-----------------------------------------+-------------------+---------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(SoftTmiInterface): 3
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 8
sizeof(TwoWireInterface): 8
sizeof(SimpleWireInterface): 4
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 24
sizeof(Tm1637Module<SoftTmiInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 20
sizeof(PatternWriter): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(LevelWriter): 4
sizeof(StringScroller): 12

CPU:
+-----------------------------------------+-------------------+---------+
| Functionality                           |   min/  avg/  max | samples |
|-----------------------------------------+-------------------+---------|
| Direct(4)                               |     6/    6/   10 |      40 |
| Direct(4,subfields)                     |     0/    1/    9 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    10/   10/   11 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     0/    1/   12 |     640 |
| Hybrid(4,HardSpi)                       |     4/    4/    5 |      40 |
| Hybrid(4,HardSpi,subfields)             |     0/    1/    7 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    16/   17/   19 |      80 |
| Hc595(8,SoftSpi,subfields)              |     0/    2/   20 |    1280 |
| Hc595(8,HardSpi)                        |     4/    4/    6 |      80 |
| Hc595(8,HardSpi,subfields)              |     0/    1/    6 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 21154/21155/21158 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3424/ 8341/ 9574 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  1154/ 1155/ 1159 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   187/  455/  529 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 26591/26592/26596 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3425/ 8692/ 9574 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   151/  151/  154 |      20 |
| Max7219(8,HardSpi)                      |    38/   38/   39 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   221/  221/  222 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  1746/ 1747/ 1754 |      20 |
+-----------------------------------------+-------------------+---------+

```

