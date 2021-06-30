# AutoBenchmark

This program creates instances of various subclasses `LedModule` using different
configurations:

* `DirectModule`: group and segment pins directly connected to MCU
* `DirectFast4Module`: same as `DirectModule` but using `digitalWriteFast`
  library
* `HybridModule`: group pins connected directly to MCU, but segment pins
  connected to one 74HC595 that is accessed through SPI
* `Hc595Module`: group pins and segment pins connected to two 74HC595 chips
  which are accessed through SPI
* `Tm1637Module`: an LED module using a TM1637 controller, accessed through
  a custom two-wire protocol similar to I2C
* `Max7219Module`: an LED module using a MAX7219 controller, accessed through
  SPI
* `Ht16k33Module`: an LED module using an HT16K33 controller, accessed through
  I2C

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
* `Ht16k33Module::flush()`
    * Sends out the buffered digits using I2C

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
  TM1637 modules allows a much shorter `DELAY_MICROS`. Added CPU benchmark
  numbers for `DELAY_MICROS = 5` microseconds (e.g. `Tm1637(4,SoftTmi,5us)` and
  `Tm1637(4,SoftTmiFast,5us)`). The `flush()` or `flushIncremental()` durations
  are almost a factor of 10X to 20X shorter compared to `DELAY_MICROS = 100`.

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

**v0.7+**

* Extract communication interfaces into AceSPI, AceTMI, and AceWire libraries.
  No change.
* Copy interface objects into various modules (Hc595Module, Max7219Module,
  Tm1637Module, Ht16k33Module) by value instead of by reference.
    * Remove an extra layer of indirection.
    * Makes almost no difference in execution speed. Maybe if I squint hard
      enough, it looks like a few microseconds faster on average?
    * Saves flash consumption on AVR processors (see MemoryBenchmark/README.md).

## Results

The following tables show the number of microseconds taken by:

* `DirectModule::renderFieldNow()`, `HybridModule::renderFieldNow()`,
  `Hc595Module::renderFieldNow()`
    * Renders a single LED digit.
    * If the LED module has 4 digits, then `renderFieldNow()` must be called 4
      times to render the light pattern of the entire LED module. Each digit
      is called a "field" and entire rendering of 4 fields is then called a
      "frame".
    * Most people can no longer see flickering of the display at about 60 frames
      a second. To achieve that, the `renderFieldNow()` method must be called
      240 times a second for a module with 4 digits, or every 4.17 milliseconds.
    * The results below show that every processor, even the slowest AVR
      processor, is able to meet this threshhold.
    * Several communication implemenations are tested:
        * SoftSpi: software bitbanging SPI using `digitalWrite()`
        * SoftSpiFast: software bitbanging SPI using `<digitalWriteFast.h>`
        * HardSpi: hardware SPI using `<SPI.h>` and `digitalWrite()` to control
          the latch pin
        * HardSpiFast: hardware SPI using `<SPI.h>` and `<digitalWriteFast.h>``
        * to control the latch pin
    * The `subfield` label indicates that `NUM_SUBFIELDS = 16` was used to
      obtain PWM brightness modulation on a per-digit basis.
        * Each call to `renderFieldNow()` performs a rendering of one of the 16
          subfields of a given digit, which gives us the PWM.
        * The average duration goes down, but the maximum remains the same.
* `Tm1637Module::flush()` or `Tm1637Module::flushIncremental()`
    * Sends digits in the buffer to the TM1637 LED module using the I2C-like
      protocol.
    * Results for two values of `DELAY_MICROS` are collected:
        * SoftTmi: software bigbanging the TM1637 protocol using
          `digitalWrite()` with a `delayMicros` of 100  microseconds or 5
          microseconds
        * SoftTmiFast: software bigbanging the TM1637 protocol using
          `<digitalWriteFast.h>` and a `DELAY_MICROS` of 100  microseconds or 5
          microseconds
* `Max7219Module::flush()`
    * Sends all digits in the buffer to the MAX7219 LED module using 4 types of
      communcation interfaces
        * SoftSpi: software SPI using `digitalWrite()`
        * SoftSpiFast: software SPI using `<digitalWriteFast.h>`
        * HardSpi: hardware SPI using `<SPI.h>` and `digitalWrite()` to control
          the latch pin
        * HardSpiFast: hardware SPI using `<SPI.h>` and `<digitalWriteFast.h>`
          to control the latch pin
* `Ht16k33Module::flush()`
    * Sends all digits in the buffer to the HT16K33 LED module using 2
      communication interfaces:
        * TwoWire: hardware I2C using `<Wire.h>`
        * SimpleWire: AceSegment's custom bitbanging I2C implementation
        * SimpleWireFast: Same as SimpleWire using `<digitalWriteFast.h>`

On AVR processors, the "fast" options are available using the
[digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast) library whose
`digitalWriteFast()` functions can be up to 50X faster if the `pin` number and
`value` parameters are compile-time constants. In addition, the
`digitalWriteFast` functions reduce flash memory consumption by 600-700 bytes
for `SoftTmiFastInterface`, `SoftSpiFastInterface`, `HardSpiFastInterface`, and
`SimpleWireFastInterface` compared to their non-fast equivalents.

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
sizeof(TwoWireInterface): 2
sizeof(SimpleWireInterface): 3
sizeof(SimpleWireFastInterface<2, 3, 10>): 1
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 9
sizeof(LedMatrixDualHc595<HardSpiInterface>): 9
sizeof(LedModule): 3
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(DirectModule<4>): 31
sizeof(DirectFast4Module<...>): 25
sizeof(HybridModule<SoftSpiInterface, 4>): 31
sizeof(Hc595Module<SoftSpiInterface, 8>): 47
sizeof(Tm1637Module<SoftTmiInterface, 4>): 15
sizeof(Tm1637Module<SoftTmiInterface, 6>): 17
sizeof(Max7219Module<SoftSpiInterface, 8>): 17
sizeof(Ht16k33Module<TwoWireInterface, 4>): 12
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 13
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
| Direct(4)                               |    72/   82/   88 |      40 |
| Direct(4,subfields)                     |     4/   13/   88 |     640 |
| DirectFast4(4)                          |    24/   30/   36 |      40 |
| DirectFast4(4,subfields)                |     4/    8/   36 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |   152/  160/  180 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     4/   22/  180 |     640 |
| Hybrid(4,SoftSpiFast)                   |    28/   33/   44 |      40 |
| Hybrid(4,SoftSpiFast,subfields)         |     4/    8/   40 |     640 |
| Hybrid(4,HardSpi)                       |    32/   39/   52 |      40 |
| Hybrid(4,HardSpi,subfields)             |     4/    8/   48 |     640 |
| Hybrid(4,HardSpiFast)                   |    20/   25/   32 |      40 |
| Hybrid(4,HardSpiFast,subfields)         |     4/    7/   32 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |   268/  273/  304 |      80 |
| Hc595(8,SoftSpi,subfields)              |     4/   36/  300 |    1280 |
| Hc595(8,SoftSpiFast)                    |    24/   27/   40 |      80 |
| Hc595(8,SoftSpiFast,subfields)          |     4/    8/   32 |    1280 |
| Hc595(8,HardSpi)                        |    24/   30/   36 |      80 |
| Hc595(8,HardSpi,subfields)              |     4/    8/   36 |    1280 |
| Hc595(8,HardSpiFast)                    |    12/   17/   28 |      80 |
| Hc595(8,HardSpiFast,subfields)          |     4/    6/   24 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22312/22341/22560 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3612/ 8806/10340 |      50 |
| Tm1637(4,SoftTmiFast,100us)             | 21068/21101/21356 |      10 |
| Tm1637(4,SoftTmiFast,100us,incremental) |  3412/ 8316/ 9812 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2244/ 2282/ 2488 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   364/  893/ 1120 |      50 |
| Tm1637(4,SoftTmiFast,5us)               |  1004/ 1033/ 1104 |      10 |
| Tm1637(4,SoftTmiFast,5us,incremental)   |   164/  403/  508 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 28056/28088/28360 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3612/ 9176/10344 |      70 |
| Tm1637(6,SoftTmiFast,100us)             | 26488/26520/26776 |      10 |
| Tm1637(6,SoftTmiFast,100us,incremental) |  3412/ 8665/ 9812 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |  2380/ 2395/ 2628 |      20 |
| Max7219(8,SoftSpiFast)                  |   204/  216/  236 |      20 |
| Max7219(8,HardSpi)                      |   204/  218/  236 |      20 |
| Max7219(8,HardSpiFast)                  |   100/  107/  116 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   340/  345/  356 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  3744/ 3754/ 3792 |      20 |
| Ht16k33(4,SimpleWireFast,4us)           |  1444/ 1452/ 1532 |      20 |
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
sizeof(TwoWireInterface): 2
sizeof(SimpleWireInterface): 3
sizeof(SimpleWireFastInterface<2, 3, 10>): 1
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 9
sizeof(LedMatrixDualHc595<HardSpiInterface>): 9
sizeof(LedModule): 3
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(DirectModule<4>): 31
sizeof(DirectFast4Module<...>): 25
sizeof(HybridModule<SoftSpiInterface, 4>): 31
sizeof(Hc595Module<SoftSpiInterface, 8>): 47
sizeof(Tm1637Module<SoftTmiInterface, 4>): 15
sizeof(Tm1637Module<SoftTmiInterface, 6>): 17
sizeof(Max7219Module<SoftSpiInterface, 8>): 17
sizeof(Ht16k33Module<TwoWireInterface, 4>): 12
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 13
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
| Direct(4)                               |    72/   77/   88 |      40 |
| Direct(4,subfields)                     |     4/   13/   84 |     640 |
| DirectFast4(4)                          |    24/   28/   36 |      40 |
| DirectFast4(4,subfields)                |     4/    8/   32 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |   144/  151/  160 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     4/   21/  156 |     640 |
| Hybrid(4,SoftSpiFast)                   |    28/   30/   36 |      40 |
| Hybrid(4,SoftSpiFast,subfields)         |     4/    8/   36 |     640 |
| Hybrid(4,HardSpi)                       |    32/   38/   44 |      40 |
| Hybrid(4,HardSpi,subfields)             |     4/    9/   48 |     640 |
| Hybrid(4,HardSpiFast)                   |    20/   24/   32 |      40 |
| Hybrid(4,HardSpiFast,subfields)         |     4/    7/   28 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |   252/  255/  264 |      80 |
| Hc595(8,SoftSpi,subfields)              |     4/   35/  264 |    1280 |
| Hc595(8,SoftSpiFast)                    |    24/   25/   32 |      80 |
| Hc595(8,SoftSpiFast,subfields)          |     4/    8/   32 |    1280 |
| Hc595(8,HardSpi)                        |    28/   29/   40 |      80 |
| Hc595(8,HardSpi,subfields)              |     4/    8/   36 |    1280 |
| Hc595(8,HardSpiFast)                    |    12/   15/   20 |      80 |
| Hc595(8,HardSpiFast,subfields)          |     4/    6/   24 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22436/22441/22452 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3632/ 8850/10164 |      50 |
| Tm1637(4,SoftTmiFast,100us)             | 21176/21185/21192 |      10 |
| Tm1637(4,SoftTmiFast,100us,incremental) |  3428/ 8355/ 9596 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2264/ 2266/ 2276 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   368/  896/ 1036 |      50 |
| Tm1637(4,SoftTmiFast,5us)               |  1008/ 1008/ 1012 |      10 |
| Tm1637(4,SoftTmiFast,5us,incremental)   |   164/  400/  468 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 28208/28215/28224 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3632/ 9223/10164 |      70 |
| Tm1637(6,SoftTmiFast,100us)             | 26624/26634/26644 |      10 |
| Tm1637(6,SoftTmiFast,100us,incremental) |  3428/ 8706/ 9596 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |  2240/ 2246/ 2256 |      20 |
| Max7219(8,SoftSpiFast)                  |   204/  210/  216 |      20 |
| Max7219(8,HardSpi)                      |   220/  223/  232 |      20 |
| Max7219(8,HardSpiFast)                  |    96/  100/  112 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   336/  340/  344 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  3764/ 3772/ 3776 |      20 |
| Ht16k33(4,SimpleWireFast,4us)           |  1448/ 1456/ 1464 |      20 |
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
sizeof(TwoWireInterface): 4
sizeof(SimpleWireInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 20
sizeof(Tm1637Module<SoftTmiInterface, 6>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 24
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 16
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
| Direct(4)                               |    24/   24/   28 |      40 |
| Direct(4,subfields)                     |     2/    5/   27 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    54/   54/   59 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     2/    8/   59 |     640 |
| Hybrid(4,HardSpi)                       |    24/   25/   29 |      40 |
| Hybrid(4,HardSpi,subfields)             |     2/    5/   27 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    92/   92/   96 |      80 |
| Hc595(8,SoftSpi,subfields)              |     3/   13/   97 |    1280 |
| Hc595(8,HardSpi)                        |    24/   24/   28 |      80 |
| Hc595(8,HardSpi,subfields)              |     3/    5/   29 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22212/22215/22219 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3595/ 8760/10054 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2109/ 2109/ 2113 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   342/  833/  957 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 27925/27927/27933 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3597/ 9129/10056 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   821/  824/  826 |      20 |
| Max7219(8,HardSpi)                      |   203/  204/  207 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   237/  237/  241 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  3356/ 3358/ 3361 |      20 |
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
sizeof(TwoWireInterface): 4
sizeof(SimpleWireInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 20
sizeof(Tm1637Module<SoftTmiInterface, 6>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 24
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 16
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
| Direct(4)                               |    15/   15/   19 |      40 |
| Direct(4,subfields)                     |     1/    3/   35 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    42/   43/   48 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     1/    6/   51 |     640 |
| Hybrid(4,HardSpi)                       |    43/   43/   49 |      40 |
| Hybrid(4,HardSpi,subfields)             |     1/    6/   52 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    76/   76/   82 |      80 |
| Hc595(8,SoftSpi,subfields)              |     1/   10/   85 |    1280 |
| Hc595(8,HardSpi)                        |    44/   44/   50 |      80 |
| Hc595(8,HardSpi,subfields)              |     1/    6/   53 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 22405/22409/22416 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3627/ 8837/10146 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  2450/ 2454/ 2459 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   397/  969/ 1129 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 28163/28169/28175 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3627/ 9208/10146 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   688/  692/  694 |      20 |
| Max7219(8,HardSpi)                      |   394/  396/  400 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   230/  230/  232 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  4172/ 4177/ 4182 |      20 |
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
sizeof(TwoWireInterface): 4
sizeof(SimpleWireInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 20
sizeof(Tm1637Module<SoftTmiInterface, 6>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 24
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 16
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
| Direct(4)                               |    12/   13/   36 |      40 |
| Direct(4,subfields)                     |     0/    2/   20 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    29/   29/   41 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     0/    4/   42 |     640 |
| Hybrid(4,HardSpi)                       |    12/   12/   32 |      40 |
| Hybrid(4,HardSpi,subfields)             |     0/    2/   24 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    50/   51/   66 |      80 |
| Hc595(8,SoftSpi,subfields)              |     0/    6/   63 |    1280 |
| Hc595(8,HardSpi)                        |    14/   14/   26 |      80 |
| Hc595(8,HardSpi,subfields)              |     0/    2/   26 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 21496/21506/21543 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3481/ 8479/ 9749 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  1525/ 1525/ 1526 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   248/  603/  707 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 27025/27054/27118 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3481/ 8836/ 9757 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   459/  460/  467 |      20 |
| Max7219(8,HardSpi)                      |   125/  126/  137 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   245/  246/  269 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  2514/ 2516/ 2538 |      20 |
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
sizeof(TwoWireInterface): 4
sizeof(SimpleWireInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 20
sizeof(Tm1637Module<SoftTmiInterface, 6>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 24
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 16
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
| Direct(4)                               |     2/    3/   10 |      40 |
| Direct(4,subfields)                     |     0/    1/    9 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |     4/    4/   10 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     0/    1/   12 |     640 |
| Hybrid(4,HardSpi)                       |    10/   10/   15 |      40 |
| Hybrid(4,HardSpi,subfields)             |     0/    1/   13 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |     7/    7/   15 |      80 |
| Hc595(8,SoftSpi,subfields)              |     0/    1/   16 |    1280 |
| Hc595(8,HardSpi)                        |    11/   11/   15 |      80 |
| Hc595(8,HardSpi,subfields)              |     0/    2/   18 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 21228/21238/21247 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3435/ 8374/ 9614 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  1277/ 1280/ 1284 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   205/  504/  584 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 26691/26697/26702 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3435/ 8727/ 9616 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |    60/   60/   68 |      20 |
| Max7219(8,HardSpi)                      |    90/   91/   99 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   311/  312/  319 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  1999/ 2002/ 2008 |      20 |
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
sizeof(TwoWireInterface): 4
sizeof(SimpleWireInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftTmiInterface, 4>): 20
sizeof(Tm1637Module<SoftTmiInterface, 6>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 24
sizeof(Ht16k33Module<TwoWireInterface, 4>): 20
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 16
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
| Direct(4)                               |     5/    6/    9 |      40 |
| Direct(4,subfields)                     |     0/    1/    9 |     640 |
|-----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                       |    10/   10/   12 |      40 |
| Hybrid(4,SoftSpi,subfields)             |     0/    1/   12 |     640 |
| Hybrid(4,HardSpi)                       |     4/    4/    7 |      40 |
| Hybrid(4,HardSpi,subfields)             |     0/    1/    6 |     640 |
|-----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                        |    16/   16/   19 |      80 |
| Hc595(8,SoftSpi,subfields)              |     0/    2/   20 |    1280 |
| Hc595(8,HardSpi)                        |     4/    4/    5 |      80 |
| Hc595(8,HardSpi,subfields)              |     0/    1/    7 |    1280 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,100us)                 | 21145/21146/21151 |      10 |
| Tm1637(4,SoftTmi,100us,incremental)     |  3422/ 8337/ 9571 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(4,SoftTmi,5us)                   |  1151/ 1152/ 1156 |      10 |
| Tm1637(4,SoftTmi,5us,incremental)       |   185/  453/  522 |      50 |
|-----------------------------------------+-------------------+---------|
| Tm1637(6,SoftTmi,100us)                 | 26580/26581/26586 |      10 |
| Tm1637(6,SoftTmi,100us,incremental)     |  3423/ 8686/ 9569 |      70 |
|-----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                      |   151/  151/  153 |      20 |
| Max7219(8,HardSpi)                      |    38/   39/   42 |      20 |
|-----------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire)                      |   221/  221/  222 |      20 |
| Ht16k33(4,SimpleWire,4us)               |  1754/ 1755/ 1761 |      20 |
+-----------------------------------------+-------------------+---------+

```

