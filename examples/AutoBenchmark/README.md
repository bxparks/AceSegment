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

**Version**: AceSegment v0.8.1

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceCommon](https://github.com/bxparks/AceCommon)
* [AceSegment](https://github.com/bxparks/AceButton)

On AVR processors, one of the following libraries is required to run the
`digitalWriteFast()` versions of the low-level drivers:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast
* https://github.com/NicksonYap/digitalWriteFast

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
  numbers for `DELAY_MICROS = 5` microseconds (e.g. `Tm1637(4,SimpleTmi,5us)`
  and `Tm1637(4,SimpleTmiFast,5us)`). The `flush()` or `flushIncremental()`
  durations are almost a factor of 10X to 20X shorter compared to `DELAY_MICROS
  = 100`.

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

**v0.8**

* Extract communication interfaces into AceSPI, AceTMI, and AceWire libraries.
  No change.
* Copy interface objects into various modules (Hc595Module, Max7219Module,
  Tm1637Module, Ht16k33Module) by value instead of by reference.
    * Remove an extra layer of indirection.
    * Makes almost no difference in execution speed. Maybe if I squint hard
      enough, it looks like a few microseconds faster on average?
    * Saves flash consumption on AVR processors (see MemoryBenchmark/README.md).
* Regenerate after attaching an actual HT16K33 LED module on the I2C bus.
    * The hardware I2C library `<Wire.h>` checks for a proper ACK
      response from the I2C device. If a NACK is received, then it stops
      transmitting the rest of the data in the transmit buffer and returns
      immediately from `endTransmission()`.
    * Causes AutoBenchmark to produces incorrect timing (too short) for the
      `Ht16k33(TwoWire)` benchmark.
    * Software I2C implementations such as `SimpleWireInterface` and
      `SimpleWireFastInterface` do *not* check the response from the I2C device.
      The benchmark results are not affected.
    * For AVR processors, the software I2C implementation of
      `SimpleWireFastInterface` using `digitalWriteFast()` is *faster* than
      hardware I2C using `<Wire.h>`:
        * `<Wire.h>` with `setClock(400000)` produces throughput of about 234
          kHz.
        * `SimpleWireFastInterface` using 1us delayMicros produces a throughput
          of about 350 kHz.
    * For 32-bit processors without a `digitalWriteFast` library,
      `SimpleWireFastInterface` is comparable to the 100 kHz setting of
      `<Wire.h>`, until we get to very fast 32-bit processors like the ESP32 and
      Teensy 3.2, where `SimpleWireInterface` becomes competitive with 400 kHz
      of `<Wire.h>`.

**v0.8+**

* Remove `virtual` keyword from `LedModule` methods.
    * No significant changes in execution time.

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
        * SimpleSpi: software bitbanging SPI using `digitalWrite()`
        * SimpleSpiFast: software bitbanging SPI using `<digitalWriteFast.h>`
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
        * SimpleTmi: software bigbanging the TM1637 protocol using
          `digitalWrite()` with a `delayMicros` of 100  microseconds or 5
          microseconds
        * SimpleTmiFast: software bigbanging the TM1637 protocol using
          `<digitalWriteFast.h>` and a `DELAY_MICROS` of 100  microseconds or 5
          microseconds
* `Max7219Module::flush()`
    * Sends all digits in the buffer to the MAX7219 LED module using 4 types of
      communcation interfaces
        * SimpleSpi: software SPI using `digitalWrite()`
        * SimpleSpiFast: software SPI using `<digitalWriteFast.h>`
        * HardSpi: hardware SPI using `<SPI.h>` and `digitalWrite()` to control
          the latch pin
        * HardSpiFast: hardware SPI using `<SPI.h>` and `<digitalWriteFast.h>`
          to control the latch pin
* `Ht16k33Module::flush()`
    * Sends all 4 digits in the buffer and the brightness setting to the HT16K33
      LED module using 2 communication interfaces:
        * TwoWire: hardware I2C using `<Wire.h>`
        * SimpleWire: AceSegment's custom bitbanging I2C implementation
        * SimpleWireFast: Same as SimpleWire using `<digitalWriteFast.h>`
    * Total bits: the addr, 5 x 16-bit words, then another addr, and the
      brightness, for a total of 13 bytes. Each byte sends 8 bits and reads the
      ACK bit from the slave, so 9 bits per byte. Total bits: 117 bits.

On AVR processors, the "fast" options are available using one of the
digitalWriteFast libraries whose `digitalWriteFast()` functions can be up to 50X
faster if the `pin` number and `value` parameters are compile-time constants. In
addition, the `digitalWriteFast` functions reduce flash memory consumption by
600-700 bytes compared to their non-fast equivalents.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): 9
sizeof(LedMatrixDualHc595<HardSpiInterface>): 9
sizeof(LedModule): 6
sizeof(ScanningModule<LedMatrixBase, 4>): 25
sizeof(DirectModule<4>): 34
sizeof(DirectFast4Module<...>): 28
sizeof(HybridModule<SimpleSpiInterface, 4>): 34
sizeof(Hc595Module<SimpleSpiInterface, 8>): 50
sizeof(Tm1637Module<SimpleTmiInterface, 4>): 17
sizeof(Tm1637Module<SimpleTmiInterface, 6>): 19
sizeof(Max7219Module<SimpleSpiInterface, 8>): 19
sizeof(Ht16k33Module<TwoWireInterface, 4>): 14
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 17
sizeof(PatternWriter): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 5
sizeof(StringWriter): 2
sizeof(LevelWriter): 2
sizeof(StringScroller): 8

CPU:
+-------------------------------------------+-------------------+---------+
| Functionality                             |   min/  avg/  max | samples |
|-------------------------------------------+-------------------+---------|
| Direct(4)                                 |    76/   83/   92 |      40 |
| Direct(4,subfields)                       |     4/   15/   96 |     640 |
| DirectFast4(4)                            |    28/   33/   40 |      40 |
| DirectFast4(4,subfields)                  |     4/    9/   36 |     640 |
|-------------------------------------------+-------------------+---------|
| Hybrid(4,HardSpi)                         |    36/   41/   56 |      40 |
| Hybrid(4,HardSpi,subfields)               |     4/   10/   48 |     640 |
| Hybrid(4,HardSpiFast)                     |    24/   27/   36 |      40 |
| Hybrid(4,HardSpiFast,subfields)           |     4/    9/   36 |     640 |
| Hybrid(4,SimpleSpi)                       |   156/  163/  184 |      40 |
| Hybrid(4,SimpleSpi,subfields)             |     4/   24/  172 |     640 |
| Hybrid(4,SimpleSpiFast)                   |    28/   34/   44 |      40 |
| Hybrid(4,SimpleSpiFast,subfields)         |     4/   10/   40 |     640 |
|-------------------------------------------+-------------------+---------|
| Hc595(8,HardSpi)                          |    28/   32/   40 |      80 |
| Hc595(8,HardSpi,subfields)                |     4/   10/   44 |    1280 |
| Hc595(8,HardSpiFast)                      |    16/   19/   32 |      80 |
| Hc595(8,HardSpiFast,subfields)            |     4/    9/   28 |    1280 |
| Hc595(8,SimpleSpi)                        |   268/  275/  308 |      80 |
| Hc595(8,SimpleSpi,subfields)              |     4/   39/  308 |    1280 |
| Hc595(8,SimpleSpiFast)                    |    24/   30/   40 |      80 |
| Hc595(8,SimpleSpiFast,subfields)          |     4/   10/   36 |    1280 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,100us)                 | 22312/22344/22580 |      10 |
| Tm1637(4,SimpleTmi,100us,incremental)     |  3612/ 8807/10356 |      50 |
| Tm1637(4,SimpleTmiFast,100us)             | 21064/21100/21364 |      10 |
| Tm1637(4,SimpleTmiFast,100us,incremental) |  3412/ 8316/ 9828 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,5us)                   |  2248/ 2283/ 2480 |      10 |
| Tm1637(4,SimpleTmi,5us,incremental)       |   364/  894/ 1124 |      50 |
| Tm1637(4,SimpleTmiFast,5us)               |  1000/ 1034/ 1120 |      10 |
| Tm1637(4,SimpleTmiFast,5us,incremental)   |   164/  403/  508 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(6,SimpleTmi,100us)                 | 28056/28090/28376 |      10 |
| Tm1637(6,SimpleTmi,100us,incremental)     |  3612/ 9176/10356 |      70 |
| Tm1637(6,SimpleTmiFast,100us)             | 26484/26518/26788 |      10 |
| Tm1637(6,SimpleTmiFast,100us,incremental) |  3412/ 8664/ 9820 |      70 |
|-------------------------------------------+-------------------+---------|
| Max7219(8,HardSpi)                        |   208/  226/  240 |      20 |
| Max7219(8,HardSpiFast)                    |    96/  108/  116 |      20 |
| Max7219(8,SimpleSpi)                      |  2380/ 2390/ 2520 |      20 |
| Max7219(8,SimpleSpiFast)                  |   208/  218/  236 |      20 |
|-------------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire,100kHz)                 |  1460/ 1463/ 1496 |      20 |
| Ht16k33(4,TwoWire,400kHz)                 |   500/  507/  532 |      20 |
| Ht16k33(4,SimpleWire,1us)                 |  2544/ 2556/ 2684 |      20 |
| Ht16k33(4,SimpleWireFast,1us)             |   224/  233/  252 |      20 |
+-------------------------------------------+-------------------+---------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(LedMatrixDirect<>): 9
sizeof(LedMatrixDirectFast4<6..13, 2..5>): 3
sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): 9
sizeof(LedMatrixDualHc595<HardSpiInterface>): 9
sizeof(LedModule): 6
sizeof(ScanningModule<LedMatrixBase, 4>): 25
sizeof(DirectModule<4>): 34
sizeof(DirectFast4Module<...>): 28
sizeof(HybridModule<SimpleSpiInterface, 4>): 34
sizeof(Hc595Module<SimpleSpiInterface, 8>): 50
sizeof(Tm1637Module<SimpleTmiInterface, 4>): 17
sizeof(Tm1637Module<SimpleTmiInterface, 6>): 19
sizeof(Max7219Module<SimpleSpiInterface, 8>): 19
sizeof(Ht16k33Module<TwoWireInterface, 4>): 14
sizeof(Ht16k33Module<SimpleWireInterface, 4>): 17
sizeof(PatternWriter): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 5
sizeof(StringWriter): 2
sizeof(LevelWriter): 2
sizeof(StringScroller): 8

CPU:
+-------------------------------------------+-------------------+---------+
| Functionality                             |   min/  avg/  max | samples |
|-------------------------------------------+-------------------+---------|
| Direct(4)                                 |    76/   79/   88 |      40 |
| Direct(4,subfields)                       |     4/   15/   84 |     640 |
| DirectFast4(4)                            |    28/   29/   36 |      40 |
| DirectFast4(4,subfields)                  |     4/    9/   36 |     640 |
|-------------------------------------------+-------------------+---------|
| Hybrid(4,HardSpi)                         |    36/   40/   52 |      40 |
| Hybrid(4,HardSpi,subfields)               |     4/   10/   44 |     640 |
| Hybrid(4,HardSpiFast)                     |    24/   26/   32 |      40 |
| Hybrid(4,HardSpiFast,subfields)           |     4/    9/   32 |     640 |
| Hybrid(4,SimpleSpi)                       |   148/  153/  160 |      40 |
| Hybrid(4,SimpleSpi,subfields)             |     4/   23/  164 |     640 |
| Hybrid(4,SimpleSpiFast)                   |    28/   31/   36 |      40 |
| Hybrid(4,SimpleSpiFast,subfields)         |     4/   10/   40 |     640 |
|-------------------------------------------+-------------------+---------|
| Hc595(8,HardSpi)                          |    28/   31/   40 |      80 |
| Hc595(8,HardSpi,subfields)                |     4/   10/   36 |    1280 |
| Hc595(8,HardSpiFast)                      |    16/   18/   24 |      80 |
| Hc595(8,HardSpiFast,subfields)            |     4/    9/   24 |    1280 |
| Hc595(8,SimpleSpi)                        |   252/  257/  264 |      80 |
| Hc595(8,SimpleSpi,subfields)              |     4/   37/  268 |    1280 |
| Hc595(8,SimpleSpiFast)                    |    24/   28/   36 |      80 |
| Hc595(8,SimpleSpiFast,subfields)          |     4/   10/   36 |    1280 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,100us)                 | 22436/22443/22452 |      10 |
| Tm1637(4,SimpleTmi,100us,incremental)     |  3632/ 8851/10164 |      50 |
| Tm1637(4,SimpleTmiFast,100us)             | 21172/21184/21192 |      10 |
| Tm1637(4,SimpleTmiFast,100us,incremental) |  3428/ 8354/ 9596 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,5us)                   |  2264/ 2268/ 2272 |      10 |
| Tm1637(4,SimpleTmi,5us,incremental)       |   368/  897/ 1036 |      50 |
| Tm1637(4,SimpleTmiFast,5us)               |  1004/ 1007/ 1008 |      10 |
| Tm1637(4,SimpleTmiFast,5us,incremental)   |   164/  399/  464 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(6,SimpleTmi,100us)                 | 28208/28216/28232 |      10 |
| Tm1637(6,SimpleTmi,100us,incremental)     |  3628/ 9223/10164 |      70 |
| Tm1637(6,SimpleTmiFast,100us)             | 26620/26630/26640 |      10 |
| Tm1637(6,SimpleTmiFast,100us,incremental) |  3428/ 8705/ 9596 |      70 |
|-------------------------------------------+-------------------+---------|
| Max7219(8,HardSpi)                        |   220/  223/  228 |      20 |
| Max7219(8,HardSpiFast)                    |    96/  100/  108 |      20 |
| Max7219(8,SimpleSpi)                      |  2244/ 2246/ 2256 |      20 |
| Max7219(8,SimpleSpiFast)                  |   208/  210/  216 |      20 |
|-------------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire,100kHz)                 |  1456/ 1463/ 1472 |      20 |
| Ht16k33(4,TwoWire,400kHz)                 |   500/  503/  504 |      20 |
| Ht16k33(4,SimpleWire,1us)                 |  2556/ 2563/ 2572 |      20 |
| Ht16k33(4,SimpleWireFast,1us)             |   224/  227/  236 |      20 |
+-------------------------------------------+-------------------+---------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.3

```
Sizes of Objects:
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SimpleSpiInterface, 4>): 48
sizeof(Hc595Module<SimpleSpiInterface, 8>): 64
sizeof(Tm1637Module<SimpleTmiInterface, 4>): 24
sizeof(Tm1637Module<SimpleTmiInterface, 6>): 24
sizeof(Max7219Module<SimpleSpiInterface, 8>): 24
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
+-------------------------------------------+-------------------+---------+
| Functionality                             |   min/  avg/  max | samples |
|-------------------------------------------+-------------------+---------|
| Direct(4)                                 |    25/   25/   29 |      40 |
| Direct(4,subfields)                       |     3/    5/   30 |     640 |
|-------------------------------------------+-------------------+---------|
| Hybrid(4,HardSpi)                         |    25/   25/   29 |      40 |
| Hybrid(4,HardSpi,subfields)               |     3/    5/   30 |     640 |
| Hybrid(4,SimpleSpi)                       |    54/   55/   59 |      40 |
| Hybrid(4,SimpleSpi,subfields)             |     3/    9/   60 |     640 |
|-------------------------------------------+-------------------+---------|
| Hc595(8,HardSpi)                          |    25/   25/   30 |      80 |
| Hc595(8,HardSpi,subfields)                |     4/    7/   30 |    1280 |
| Hc595(8,SimpleSpi)                        |    93/   94/  100 |      80 |
| Hc595(8,SimpleSpi,subfields)              |     4/   15/  100 |    1280 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,100us)                 | 22213/22218/22225 |      10 |
| Tm1637(4,SimpleTmi,100us,incremental)     |  3596/ 8761/10054 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,5us)                   |  2109/ 2109/ 2115 |      10 |
| Tm1637(4,SimpleTmi,5us,incremental)       |   342/  833/  957 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(6,SimpleTmi,100us)                 | 27924/27929/27935 |      10 |
| Tm1637(6,SimpleTmi,100us,incremental)     |  3596/ 9129/10057 |      70 |
|-------------------------------------------+-------------------+---------|
| Max7219(8,HardSpi)                        |   202/  203/  208 |      20 |
| Max7219(8,SimpleSpi)                      |   822/  825/  827 |      20 |
|-------------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire,100kHz)                 |  1341/ 1342/ 1348 |      20 |
| Ht16k33(4,TwoWire,400kHz)                 |   379/  380/  384 |      20 |
| Ht16k33(4,SimpleWire,1us)                 |  2068/ 2068/ 2073 |      20 |
+-------------------------------------------+-------------------+---------+

```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 2.0.0

```
Sizes of Objects:
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SimpleSpiInterface, 4>): 48
sizeof(Hc595Module<SimpleSpiInterface, 8>): 64
sizeof(Tm1637Module<SimpleTmiInterface, 4>): 24
sizeof(Tm1637Module<SimpleTmiInterface, 6>): 24
sizeof(Max7219Module<SimpleSpiInterface, 8>): 24
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
+-------------------------------------------+-------------------+---------+
| Functionality                             |   min/  avg/  max | samples |
|-------------------------------------------+-------------------+---------|
| Direct(4)                                 |    15/   15/   16 |      40 |
| Direct(4,subfields)                       |     1/    3/   23 |     640 |
|-------------------------------------------+-------------------+---------|
| Hybrid(4,HardSpi)                         |    43/   43/   48 |      40 |
| Hybrid(4,HardSpi,subfields)               |     1/    6/   52 |     640 |
| Hybrid(4,SimpleSpi)                       |    43/   43/   47 |      40 |
| Hybrid(4,SimpleSpi,subfields)             |     1/    6/   64 |     640 |
|-------------------------------------------+-------------------+---------|
| Hc595(8,HardSpi)                          |    44/   44/   50 |      80 |
| Hc595(8,HardSpi,subfields)                |     1/    6/   53 |    1280 |
| Hc595(8,SimpleSpi)                        |    76/   76/   82 |      80 |
| Hc595(8,SimpleSpi,subfields)              |     1/   10/   97 |    1280 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,100us)                 | 22406/22412/22419 |      10 |
| Tm1637(4,SimpleTmi,100us,incremental)     |  3629/ 8838/10147 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,5us)                   |  2459/ 2463/ 2468 |      10 |
| Tm1637(4,SimpleTmi,5us,incremental)       |   398/  971/ 1135 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(6,SimpleTmi,100us)                 | 28174/28180/28193 |      10 |
| Tm1637(6,SimpleTmi,100us,incremental)     |  3629/ 9211/10146 |      70 |
|-------------------------------------------+-------------------+---------|
| Max7219(8,HardSpi)                        |   394/  396/  400 |      20 |
| Max7219(8,SimpleSpi)                      |   688/  691/  694 |      20 |
|-------------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire,100kHz)                 |  1322/ 1322/ 1327 |      20 |
| Ht16k33(4,TwoWire,400kHz)                 |   407/  408/  412 |      20 |
| Ht16k33(4,SimpleWire,1us)                 |  2987/ 2991/ 2992 |      20 |
+-------------------------------------------+-------------------+---------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SimpleSpiInterface, 4>): 48
sizeof(Hc595Module<SimpleSpiInterface, 8>): 64
sizeof(Tm1637Module<SimpleTmiInterface, 4>): 24
sizeof(Tm1637Module<SimpleTmiInterface, 6>): 24
sizeof(Max7219Module<SimpleSpiInterface, 8>): 24
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
+-------------------------------------------+-------------------+---------+
| Functionality                             |   min/  avg/  max | samples |
|-------------------------------------------+-------------------+---------|
| Direct(4)                                 |    12/   14/   40 |      40 |
| Direct(4,subfields)                       |     1/    2/   32 |     640 |
|-------------------------------------------+-------------------+---------|
| Hybrid(4,HardSpi)                         |    12/   12/   24 |      40 |
| Hybrid(4,HardSpi,subfields)               |     1/    2/   28 |     640 |
| Hybrid(4,SimpleSpi)                       |    29/   30/   49 |      40 |
| Hybrid(4,SimpleSpi,subfields)             |     1/    4/   49 |     640 |
|-------------------------------------------+-------------------+---------|
| Hc595(8,HardSpi)                          |    15/   15/   31 |      80 |
| Hc595(8,HardSpi,subfields)                |     1/    3/   35 |    1280 |
| Hc595(8,SimpleSpi)                        |    51/   51/   67 |      80 |
| Hc595(8,SimpleSpi,subfields)              |     1/    7/   72 |    1280 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,100us)                 | 21498/21504/21546 |      10 |
| Tm1637(4,SimpleTmi,100us,incremental)     |  3481/ 8478/ 9745 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,5us)                   |  1528/ 1528/ 1531 |      10 |
| Tm1637(4,SimpleTmi,5us,incremental)       |   248/  603/  696 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(6,SimpleTmi,100us)                 | 27027/27030/27047 |      10 |
| Tm1637(6,SimpleTmi,100us,incremental)     |  3481/ 8834/ 9745 |      70 |
|-------------------------------------------+-------------------+---------|
| Max7219(8,HardSpi)                        |   126/  126/  138 |      20 |
| Max7219(8,SimpleSpi)                      |   460/  461/  472 |      20 |
|-------------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire,100kHz)                 |  1322/ 1326/ 1357 |      20 |
| Ht16k33(4,TwoWire,400kHz)                 |   347/  347/  348 |      20 |
| Ht16k33(4,SimpleWire,1us)                 |  1330/ 1332/ 1354 |      20 |
+-------------------------------------------+-------------------+---------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
Sizes of Objects:
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SimpleSpiInterface, 4>): 48
sizeof(Hc595Module<SimpleSpiInterface, 8>): 64
sizeof(Tm1637Module<SimpleTmiInterface, 4>): 24
sizeof(Tm1637Module<SimpleTmiInterface, 6>): 24
sizeof(Max7219Module<SimpleSpiInterface, 8>): 24
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
+-------------------------------------------+-------------------+---------+
| Functionality                             |   min/  avg/  max | samples |
|-------------------------------------------+-------------------+---------|
| Direct(4)                                 |     2/    3/   11 |      40 |
| Direct(4,subfields)                       |     0/    1/    7 |     640 |
|-------------------------------------------+-------------------+---------|
| Hybrid(4,HardSpi)                         |    10/   10/   14 |      40 |
| Hybrid(4,HardSpi,subfields)               |     0/    1/   15 |     640 |
| Hybrid(4,SimpleSpi)                       |     4/    4/   12 |      40 |
| Hybrid(4,SimpleSpi,subfields)             |     0/    1/    9 |     640 |
|-------------------------------------------+-------------------+---------|
| Hc595(8,HardSpi)                          |    11/   11/   15 |      80 |
| Hc595(8,HardSpi,subfields)                |     1/    2/   20 |    1280 |
| Hc595(8,SimpleSpi)                        |     7/    7/   16 |      80 |
| Hc595(8,SimpleSpi,subfields)              |     1/    1/   16 |    1280 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,100us)                 | 21230/21238/21245 |      10 |
| Tm1637(4,SimpleTmi,100us,incremental)     |  3436/ 8375/ 9619 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,5us)                   |  1274/ 1278/ 1287 |      10 |
| Tm1637(4,SimpleTmi,5us,incremental)       |   205/  504/  585 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(6,SimpleTmi,100us)                 | 26695/26699/26705 |      10 |
| Tm1637(6,SimpleTmi,100us,incremental)     |  3436/ 8728/ 9618 |      70 |
|-------------------------------------------+-------------------+---------|
| Max7219(8,HardSpi)                        |    90/   91/   98 |      20 |
| Max7219(8,SimpleSpi)                      |    60/   61/   68 |      20 |
|-------------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire,100kHz)                 |  1382/ 1383/ 1390 |      20 |
| Ht16k33(4,TwoWire,400kHz)                 |   422/  422/  423 |      20 |
| Ht16k33(4,SimpleWire,1us)                 |   825/  832/  837 |      20 |
+-------------------------------------------+-------------------+---------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 20
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SimpleSpiInterface, 4>): 48
sizeof(Hc595Module<SimpleSpiInterface, 8>): 64
sizeof(Tm1637Module<SimpleTmiInterface, 4>): 24
sizeof(Tm1637Module<SimpleTmiInterface, 6>): 24
sizeof(Max7219Module<SimpleSpiInterface, 8>): 24
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
+-------------------------------------------+-------------------+---------+
| Functionality                             |   min/  avg/  max | samples |
|-------------------------------------------+-------------------+---------|
| Direct(4)                                 |     6/    6/    9 |      40 |
| Direct(4,subfields)                       |     0/    1/    9 |     640 |
|-------------------------------------------+-------------------+---------|
| Hybrid(4,HardSpi)                         |     4/    4/    5 |      40 |
| Hybrid(4,HardSpi,subfields)               |     0/    1/    5 |     640 |
| Hybrid(4,SimpleSpi)                       |    10/   10/   11 |      40 |
| Hybrid(4,SimpleSpi,subfields)             |     0/    1/   11 |     640 |
|-------------------------------------------+-------------------+---------|
| Hc595(8,HardSpi)                          |     5/    5/    9 |      80 |
| Hc595(8,HardSpi,subfields)                |     1/    2/    7 |    1280 |
| Hc595(8,SimpleSpi)                        |    17/   17/   18 |      80 |
| Hc595(8,SimpleSpi,subfields)              |     1/    3/   21 |    1280 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,100us)                 | 21183/21185/21189 |      10 |
| Tm1637(4,SimpleTmi,100us,incremental)     |  3424/ 8341/ 9577 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(4,SimpleTmi,5us)                   |  1156/ 1157/ 1161 |      10 |
| Tm1637(4,SimpleTmi,5us,incremental)       |   187/  458/  531 |      50 |
|-------------------------------------------+-------------------+---------|
| Tm1637(6,SimpleTmi,100us)                 | 26638/26640/26642 |      10 |
| Tm1637(6,SimpleTmi,100us,incremental)     |  3425/ 8691/ 9575 |      70 |
|-------------------------------------------+-------------------+---------|
| Max7219(8,HardSpi)                        |    40/   40/   41 |      20 |
| Max7219(8,SimpleSpi)                      |   150/  150/  153 |      20 |
|-------------------------------------------+-------------------+---------|
| Ht16k33(4,TwoWire,100kHz)                 |  1310/ 1311/ 1312 |      20 |
| Ht16k33(4,TwoWire,400kHz)                 |   360/  360/  363 |      20 |
| Ht16k33(4,SimpleWire,1us)                 |   600/  602/  607 |      20 |
+-------------------------------------------+-------------------+---------+

```

