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

**Version**: AceSegment v0.5

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
      capacitors (20 nF) installed on the DIO and CLK lines. They should have
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

## Results

The following tables show the number of microseconds taken by:

* `ScanningModule::renderFieldNow()`
    * renders the 8 segments of a single LED digit. If the LED module has 4
      digits, then `renderFieldNow()` must be called 4 times to render the light
      pattern of the entire LED module. The entire rendering is then called a
      frame.
* `Tm1637Module::flush()`
    * sends all digits in the buffer to the TM1637 LED module using the I2C-like
      protocol
    * a bitDelay of 100 microseconds is used
* `Max7219Module::flush()`
    * sends all digits in the buffer to the MAX7219 LED module using standard
      software or hardware SPI

Most people can no longer see flickering of the display at about 60 frames a
second. To achieve that, the `renderFieldNow()` method must be called 240
times a second for a module with 4 digits, or every 4.17 milliseconds. The
results below show that every processor, even the slowest AVR processor, is able
to meet this threshhold.

* For the `BaseModule` and `DirectFast4Module`, this involves turning off the
  previous digit, sending the 8 bits for the current digit's 8 LED segments in a
  loop, then turning on the current digit. The `digitalWrite()` function is
  called 10 times.
* For the `HybridModule` type, the 8 LED segment bits are sent
  using software SPI or hardware SPI. (Software SPI uses the `shiftOut()`
  method, which is implemented using a loop of `digitalWrite()`.
* For the `Hc595Module` type, the LED digit pins and the LED
  segment pins are using conceptually a single SPI transaction. For software
  SPI, this is implemented using 2 `shiftOut()` operations. For hardware SPI,
  this uses a single `SPI::transfer16()` command.

On AVR processors, the "fast" options are available using the
[digitalWriteFast](https://github.com/NicksonYap/digitalWriteFast) library whose
`digitalWriteFast()` functions can be up to 50X faster if the `pin` number and
`value` parameters are compile-time constants. In addition, the
`digitalWriteFast` functions reduce flash memory consumption by 600-700 bytes
for `SoftWireFastInterface`, `SoftSpiFastInterface`, and `HardSpiFastInterface`
compared to their non-fast equivalents.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.13
* Arduino AVR Boards 1.8.3
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftWireFastInterface<4, 5, 100>): 1
sizeof(SoftSpiInterface): 3
sizeof(SoftSpiFastInterface<11, 12, 13>): 1
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface<11, 12, 13>): 1
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
sizeof(Tm1637Module<SoftWireInterface, 4>): 14
sizeof(Tm1637Module<SoftWireInterface, 6>): 16
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(LedDisplay): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 5
sizeof(StringWriter): 2
sizeof(StringScroller): 11

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct(4)                              |    76/   82/   88 |      40 |
| Direct(4,subfields)                    |     4/   13/   88 |     640 |
| DirectFast4(4)                         |    28/   31/   36 |      40 |
| DirectFast4(4,subfields)               |     4/    8/   36 |     640 |
|----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                      |   152/  161/  176 |      40 |
| Hybrid(4,SoftSpi,subfields)            |     4/   22/  176 |     640 |
| Hybrid(4,SoftSpiFast)                  |    28/   34/   40 |      40 |
| Hybrid(4,SoftSpiFast,subfields)        |     4/    8/   40 |     640 |
| Hybrid(4,HardSpi)                      |    36/   41/   52 |      40 |
| Hybrid(4,HardSpi,subfields)            |     4/    9/   48 |     640 |
| Hybrid(4,HardSpiFast)                  |    24/   29/   36 |      40 |
| Hybrid(4,HardSpiFast,subfields)        |     4/    8/   36 |     640 |
|----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                       |   268/  273/  308 |      80 |
| Hc595(8,SoftSpi,subfields)             |     4/   36/  304 |    1280 |
| Hc595(8,SoftSpiFast)                   |    24/   27/   36 |      80 |
| Hc595(8,SoftSpiFast,subfields)         |     4/    8/   36 |    1280 |
| Hc595(8,HardSpi)                       |    28/   30/   40 |      80 |
| Hc595(8,HardSpi,subfields)             |     4/    8/   36 |    1280 |
| Hc595(8,HardSpiFast)                   |    12/   18/   28 |      80 |
| Hc595(8,HardSpiFast,subfields)         |     4/    7/   32 |    1280 |
|----------------------------------------+-------------------+---------|
| Tm1637(4,SoftWire)                     | 22316/22348/22568 |      10 |
| Tm1637(4,SoftWire,incremental)         |  3616/ 8810/10320 |      50 |
| Tm1637(4,SoftWireFast)                 | 21064/21092/21316 |      10 |
| Tm1637(4,SoftWireFast,incremental)     |  3412/ 8315/ 9776 |      50 |
| Tm1637(6,SoftWire)                     | 28060/28092/28344 |      10 |
| Tm1637(6,SoftWire,incremental)         |  3616/ 9178/10316 |      70 |
| Tm1637(6,SoftWireFast)                 | 26484/26511/26732 |      10 |
| Tm1637(6,SoftWireFast,incremental)     |  3412/ 8663/ 9768 |      70 |
|----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                     |  2380/ 2395/ 2600 |      20 |
| Max7219(8,SoftSpiFast)                 |   208/  218/  240 |      20 |
| Max7219(8,HardSpi)                     |   220/  232/  248 |      20 |
| Max7219(8,HardSpiFast)                 |   108/  117/  124 |      20 |
+----------------------------------------+-------------------+---------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.13
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftWireFastInterface<4, 5, 100>): 1
sizeof(SoftSpiInterface): 3
sizeof(SoftSpiFastInterface<11, 12, 13>): 1
sizeof(HardSpiInterface): 3
sizeof(HardSpiFastInterface<11, 12, 13>): 1
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
sizeof(Tm1637Module<SoftWireInterface, 4>): 14
sizeof(Tm1637Module<SoftWireInterface, 6>): 16
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(LedDisplay): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 5
sizeof(StringWriter): 2
sizeof(StringScroller): 11

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct(4)                              |    72/   78/   88 |      40 |
| Direct(4,subfields)                    |     4/   13/   84 |     640 |
| DirectFast4(4)                         |    24/   28/   32 |      40 |
| DirectFast4(4,subfields)               |     4/    8/   40 |     640 |
|----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                      |   148/  152/  156 |      40 |
| Hybrid(4,SoftSpi,subfields)            |     4/   21/  160 |     640 |
| Hybrid(4,SoftSpiFast)                  |    28/   32/   36 |      40 |
| Hybrid(4,SoftSpiFast,subfields)        |     4/    8/   40 |     640 |
| Hybrid(4,HardSpi)                      |    36/   40/   48 |      40 |
| Hybrid(4,HardSpi,subfields)            |     4/    9/   48 |     640 |
| Hybrid(4,HardSpiFast)                  |    24/   28/   36 |      40 |
| Hybrid(4,HardSpiFast,subfields)        |     4/    7/   36 |     640 |
|----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                       |   252/  255/  264 |      80 |
| Hc595(8,SoftSpi,subfields)             |     4/   35/  264 |    1280 |
| Hc595(8,SoftSpiFast)                   |    24/   25/   32 |      80 |
| Hc595(8,SoftSpiFast,subfields)         |     4/    8/   32 |    1280 |
| Hc595(8,HardSpi)                       |    28/   30/   36 |      80 |
| Hc595(8,HardSpi,subfields)             |     4/    8/   36 |    1280 |
| Hc595(8,HardSpiFast)                   |    16/   16/   20 |      80 |
| Hc595(8,HardSpiFast,subfields)         |     4/    7/   24 |    1280 |
|----------------------------------------+-------------------+---------|
| Tm1637(4,SoftWire)                     | 22440/22450/22460 |      10 |
| Tm1637(4,SoftWire,incremental)         |  3632/ 8853/10164 |      50 |
| Tm1637(4,SoftWireFast)                 | 21172/21182/21196 |      10 |
| Tm1637(4,SoftWireFast,incremental)     |  3428/ 8354/ 9592 |      50 |
| Tm1637(6,SoftWire)                     | 28212/28223/28232 |      10 |
| Tm1637(6,SoftWire,incremental)         |  3628/ 9226/10164 |      70 |
| Tm1637(6,SoftWireFast)                 | 26620/26628/26636 |      10 |
| Tm1637(6,SoftWireFast,incremental)     |  3428/ 8705/ 9592 |      70 |
|----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                     |  2244/ 2247/ 2256 |      20 |
| Max7219(8,SoftSpiFast)                 |   208/  211/  216 |      20 |
| Max7219(8,HardSpi)                     |   232/  235/  244 |      20 |
| Max7219(8,HardSpiFast)                 |   108/  109/  120 |      20 |
+----------------------------------------+-------------------+---------+

```

### SAMD21 M0 Mini

* 48 MHz ARM Cortex-M0+
* Arduino IDE 1.8.13
* Sparkfun SAMD Core 1.8.1

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Tm1637Module<SoftWireInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(StringScroller): 20

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct(4)                              |    23/   24/   28 |      40 |
| Direct(4,subfields)                    |     2/    5/   28 |     640 |
|----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                      |    52/   53/   57 |      40 |
| Hybrid(4,SoftSpi,subfields)            |     2/    8/   56 |     640 |
| Hybrid(4,HardSpi)                      |    23/   24/   27 |      40 |
| Hybrid(4,HardSpi,subfields)            |     2/    5/   28 |     640 |
|----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                       |    88/   89/   93 |      80 |
| Hc595(8,SoftSpi,subfields)             |     3/   13/   94 |    1280 |
| Hc595(8,HardSpi)                       |    23/   23/   27 |      80 |
| Hc595(8,HardSpi,subfields)             |     3/    5/   27 |    1280 |
|----------------------------------------+-------------------+---------|
| Tm1637(4,SoftWire)                     | 22206/22209/22215 |      10 |
| Tm1637(4,SoftWire,incremental)         |  3594/ 8757/10051 |      50 |
| Tm1637(6,SoftWire)                     | 27913/27920/27926 |      10 |
| Tm1637(6,SoftWire,incremental)         |  3594/ 9126/10052 |      70 |
|----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                     |   798/  800/  802 |      20 |
| Max7219(8,HardSpi)                     |   191/  192/  196 |      20 |
+----------------------------------------+-------------------+---------+

```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.13
* STM32duino 1.9.0

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Tm1637Module<SoftWireInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(StringScroller): 20

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct(4)                              |    14/   14/   15 |      40 |
| Direct(4,subfields)                    |     1/    2/   15 |     640 |
|----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                      |    31/   31/   36 |      40 |
| Hybrid(4,SoftSpi,subfields)            |     1/    4/   50 |     640 |
| Hybrid(4,HardSpi)                      |    40/   40/   45 |      40 |
| Hybrid(4,HardSpi,subfields)            |     1/    5/   50 |     640 |
|----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                       |    53/   53/   57 |      80 |
| Hc595(8,SoftSpi,subfields)             |     1/    7/   63 |    1280 |
| Hc595(8,HardSpi)                       |    40/   40/   45 |      80 |
| Hc595(8,HardSpi,subfields)             |     1/    6/   45 |    1280 |
|----------------------------------------+-------------------+---------|
| Tm1637(4,SoftWire)                     | 22382/22383/22385 |      10 |
| Tm1637(4,SoftWire,incremental)         |  3623/ 8825/10140 |      50 |
| Tm1637(6,SoftWire)                     | 28136/28141/28144 |      10 |
| Tm1637(6,SoftWire,incremental)         |  3623/ 9197/10136 |      70 |
|----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                     |   478/  481/  484 |      20 |
| Max7219(8,HardSpi)                     |   361/  363/  367 |      20 |
+----------------------------------------+-------------------+---------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.13
* ESP8266 Boards 2.7.4

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Tm1637Module<SoftWireInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(StringScroller): 20

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct(4)                              |    12/   13/   40 |      40 |
| Direct(4,subfields)                    |     0/    2/   20 |     640 |
|----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                      |    29/   29/   41 |      40 |
| Hybrid(4,SoftSpi,subfields)            |     0/    3/   42 |     640 |
| Hybrid(4,HardSpi)                      |    11/   11/   27 |      40 |
| Hybrid(4,HardSpi,subfields)            |     0/    2/   23 |     640 |
|----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                       |    50/   50/   62 |      80 |
| Hc595(8,SoftSpi,subfields)             |     0/    6/   62 |    1280 |
| Hc595(8,HardSpi)                       |    12/   12/   25 |      80 |
| Hc595(8,HardSpi,subfields)             |     0/    2/   25 |    1280 |
|----------------------------------------+-------------------+---------|
| Tm1637(4,SoftWire)                     | 21497/21506/21541 |      10 |
| Tm1637(4,SoftWire,incremental)         |  3481/ 8479/ 9749 |      50 |
| Tm1637(6,SoftWire)                     | 27025/27035/27049 |      10 |
| Tm1637(6,SoftWire,incremental)         |  3481/ 8838/ 9762 |      70 |
|----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                     |   460/  461/  474 |      20 |
| Max7219(8,HardSpi)                     |   111/  111/  120 |      20 |
+----------------------------------------+-------------------+---------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.13
* ESP32 Boards 1.0.6

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Tm1637Module<SoftWireInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(StringScroller): 20

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct(4)                              |     2/    2/    8 |      40 |
| Direct(4,subfields)                    |     0/    1/    7 |     640 |
|----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                      |     4/    4/    7 |      40 |
| Hybrid(4,SoftSpi,subfields)            |     0/    1/    7 |     640 |
| Hybrid(4,HardSpi)                      |     9/    9/   17 |      40 |
| Hybrid(4,HardSpi,subfields)            |     0/    1/   17 |     640 |
|----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                       |     7/    7/   10 |      80 |
| Hc595(8,SoftSpi,subfields)             |     0/    1/   16 |    1280 |
| Hc595(8,HardSpi)                       |     9/    9/   18 |      80 |
| Hc595(8,HardSpi,subfields)             |     0/    1/   19 |    1280 |
|----------------------------------------+-------------------+---------|
| Tm1637(4,SoftWire)                     | 21235/21240/21251 |      10 |
| Tm1637(4,SoftWire,incremental)         |  3437/ 8375/ 9616 |      50 |
| Tm1637(6,SoftWire)                     | 26687/26694/26701 |      10 |
| Tm1637(6,SoftWire,incremental)         |  3436/ 8727/ 9616 |      70 |
|----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                     |    60/   60/   71 |      20 |
| Max7219(8,HardSpi)                     |    78/   79/   86 |      20 |
+----------------------------------------+-------------------+---------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.13
* Teensyduino 1.53
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(SoftWireInterface): 4
sizeof(SoftSpiInterface): 3
sizeof(HardSpiInterface): 3
sizeof(LedMatrixDirect<>): 16
sizeof(LedMatrixSingleHc595<SoftSpiInterface>): 16
sizeof(LedMatrixDualHc595<HardSpiInterface>): 16
sizeof(LedModule): 8
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(DirectModule<4>): 48
sizeof(HybridModule<SoftSpiInterface, 4>): 48
sizeof(Hc595Module<SoftSpiInterface, 8>): 64
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Tm1637Module<SoftWireInterface, 6>): 28
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 12
sizeof(StringWriter): 4
sizeof(StringScroller): 20

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Direct(4)                              |     6/    6/   10 |      40 |
| Direct(4,subfields)                    |     0/    1/    9 |     640 |
|----------------------------------------+-------------------+---------|
| Hybrid(4,SoftSpi)                      |    10/   10/   11 |      40 |
| Hybrid(4,SoftSpi,subfields)            |     0/    1/   13 |     640 |
| Hybrid(4,HardSpi)                      |     3/    3/    4 |      40 |
| Hybrid(4,HardSpi,subfields)            |     0/    1/    7 |     640 |
|----------------------------------------+-------------------+---------|
| Hc595(8,SoftSpi)                       |    16/   16/   19 |      80 |
| Hc595(8,SoftSpi,subfields)             |     0/    2/   20 |    1280 |
| Hc595(8,HardSpi)                       |     3/    3/    5 |      80 |
| Hc595(8,HardSpi,subfields)             |     0/    1/    6 |    1280 |
|----------------------------------------+-------------------+---------|
| Tm1637(4,SoftWire)                     | 21179/21180/21185 |      10 |
| Tm1637(4,SoftWire,incremental)         |  3423/ 8338/ 9573 |      50 |
| Tm1637(6,SoftWire)                     | 26612/26614/26617 |      10 |
| Tm1637(6,SoftWire,incremental)         |  3423/ 8689/ 9573 |      70 |
|----------------------------------------+-------------------+---------|
| Max7219(8,SoftSpi)                     |   152/  152/  154 |      20 |
| Max7219(8,HardSpi)                     |    30/   30/   34 |      20 |
+----------------------------------------+-------------------+---------+

```

