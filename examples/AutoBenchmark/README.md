# AutoBenchmark

This program creates instances of `ScanningModule` using different
configurations of the `LedMatrix` class:

* `direct`: group and segment pins directly connected to MCU
* `single_sw_spi`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through software SPI (`SoftSpiInterface`)
* `single_hw_spi`: group pins connected directly to MCU, but segment pins
  connected to 74HC595 accessed through hardware SPI (`HardSpiInterface`)
* `dual_sw_spi`: group pins and segment pins connected to 74HC595 accessed
  through software SPI (`SoftSpiInterface`)
* `dual_hw_spi`: group pins and segment pins connected to 74HC595 accessed
  through hardware SPI (`HardSpiInterface`)

It measures the time taken by `ScanningModule::renderFieldNow()` which
renders a single digit (multiple fields make up a frame, a frame is the
rendering of all digits on the display module).

**Version**: AceSegment v0.4+

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

**v0.4+:**

* Add benchmarks for `Tm1637Module`. The CPU time is mostly determined by
  the calls to `delayMicroseconds()`, which is required to meet the electrical
  characteristics of the LED module.
* Add benchmarks for `Max7219Module`.
* Upgrade from ESP32 Core v1.0.4 to v1.0.6.
* Add benchmarks for `LedMatrixSingleShiftRegister`,
  `LedMatrixDualShiftRegister`, and `Max7219Module` using `HardSpiFastInteface`.

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

* For the `LedMatrixDirect` type, this involves turning off the previous digit,
  sending the 8 bits for the current digit's 8 LED segments in a loop, then
  turning on the current digit. The `digitalWrite()` function is called 10
  times.
* For the `LedMatrixSingleShiftRegister` type, the 8 LED segment bits are sent
  using software SPI or hardware SPI. (Software SPI uses the `shiftOut()`
  method, which is implemented using a loop of `digitalWrite()`.
* For the `LedMatrixDualShiftRegister` type, the LED digit pins and the LED
  segment pins are using conceptually a single SPI transaction. For software
  SPI, this is implemented using 2 `shiftOut()` operations. For hardware SPI,
  this uses a single `transfer16()` command.

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
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 8
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 5
sizeof(LedModule): 2
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(Tm1637Module<SoftWireInterface, 4>): 14
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(LedDisplay): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 2
sizeof(StringWriter): 2
sizeof(StringScroller): 7

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(Direct)                       |    68/   74/   88 |     240 |
| Scanning(Direct,subfields)             |     4/   12/   88 |    3840 |
| Scanning(DirectFast)                   |    24/   28/   40 |     240 |
| Scanning(DirectFast,subfields)         |     4/    8/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Single,SoftSpi)               |   156/  159/  180 |     240 |
| Scanning(Single,SoftSpi,subfields)     |     4/   22/  180 |    3840 |
| Scanning(Single,SoftSpiFast)           |    28/   31/   44 |     240 |
| Scanning(Single,SoftSpiFast,subfields) |     4/    8/   40 |    3840 |
| Scanning(Single,HardSpi)               |    36/   39/   52 |     240 |
| Scanning(Single,HardSpi,subfields)     |     4/    9/   48 |    3840 |
| Scanning(Single,HardSpiFast)           |    24/   27/   40 |     240 |
| Scanning(Single,HardSpiFast,subfields) |     4/    7/   40 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Dual,SoftSpi)                 |   264/  269/  304 |     240 |
| Scanning(Dual,SoftSpi,subfields)       |     4/   34/  296 |    3840 |
| Scanning(Dual,SoftSpiFast)             |    20/   24/   32 |     240 |
| Scanning(Dual,SoftSpiFast,subfields)   |     4/    7/   32 |    3840 |
| Scanning(Dual,HardSpi)                 |    24/   27/   36 |     240 |
| Scanning(Dual,HardSpi,subfields)       |     4/    8/   36 |    3840 |
| Scanning(Dual,HardSpiFast)             |    12/   14/   28 |     240 |
| Scanning(Dual,HardSpiFast,subfields)   |     4/    6/   24 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22308/22329/22600 |      20 |
| Tm1637(SoftWireFast)                   | 21060/21073/21224 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |  1320/ 1331/ 1456 |      20 |
| Max7219(SoftSpiFast)                   |   112/  120/  132 |      20 |
| Max7219(HardSpi)                       |   120/  130/  144 |      20 |
| Max7219(HardSpiFast)                   |    56/   63/   68 |      20 |
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
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 8
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 5
sizeof(LedModule): 2
sizeof(ScanningModule<LedMatrixBase, 4>): 22
sizeof(Tm1637Module<SoftWireInterface, 4>): 14
sizeof(Max7219Module<SoftSpiInterface, 8>): 16
sizeof(LedDisplay): 2
sizeof(NumberWriter): 2
sizeof(ClockWriter): 3
sizeof(TemperatureWriter): 2
sizeof(CharWriter): 2
sizeof(StringWriter): 2
sizeof(StringScroller): 7

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(Direct)                       |    72/   76/   84 |     240 |
| Scanning(Direct,subfields)             |     4/   14/   84 |    3840 |
| Scanning(DirectFast)                   |    24/   28/   36 |     240 |
| Scanning(DirectFast,subfields)         |     4/    8/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Single,SoftSpi)               |   148/  153/  164 |     240 |
| Scanning(Single,SoftSpi,subfields)     |     4/   23/  164 |    3840 |
| Scanning(Single,SoftSpiFast)           |    28/   31/   40 |     240 |
| Scanning(Single,SoftSpiFast,subfields) |     4/    8/   40 |    3840 |
| Scanning(Single,HardSpi)               |    36/   41/   52 |     240 |
| Scanning(Single,HardSpi,subfields)     |     4/    9/   52 |    3840 |
| Scanning(Single,HardSpiFast)           |    24/   27/   36 |     240 |
| Scanning(Single,HardSpiFast,subfields) |     4/    8/   36 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Dual,SoftSpi)                 |   248/  252/  264 |     240 |
| Scanning(Dual,SoftSpi,subfields)       |     4/   36/  264 |    3840 |
| Scanning(Dual,SoftSpiFast)             |    20/   23/   36 |     240 |
| Scanning(Dual,SoftSpiFast,subfields)   |     4/    8/   32 |    3840 |
| Scanning(Dual,HardSpi)                 |    24/   28/   36 |     240 |
| Scanning(Dual,HardSpi,subfields)       |     4/    8/   36 |    3840 |
| Scanning(Dual,HardSpiFast)             |    12/   13/   20 |     240 |
| Scanning(Dual,HardSpiFast,subfields)   |     4/    6/   20 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22432/22439/22452 |      20 |
| Tm1637(SoftWireFast)                   | 21168/21177/21184 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |  1244/ 1247/ 1252 |      20 |
| Max7219(SoftSpiFast)                   |   112/  113/  120 |      20 |
| Max7219(HardSpi)                       |   128/  129/  136 |      20 |
| Max7219(HardSpiFast)                   |    56/   59/   64 |      20 |
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
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 4
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(Direct)                       |    24/   24/   29 |     240 |
| Scanning(Direct,subfields)             |     2/    4/   29 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Single,SoftSpi)               |    51/   51/   58 |     240 |
| Scanning(Single,SoftSpi,subfields)     |     2/    7/   58 |    3840 |
| Scanning(Single,HardSpi)               |    23/   23/   30 |     240 |
| Scanning(Single,HardSpi,subfields)     |     2/    4/   30 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Dual,SoftSpi)                 |    87/   87/   93 |     240 |
| Scanning(Dual,SoftSpi,subfields)       |     2/   10/   94 |    3840 |
| Scanning(Dual,HardSpi)                 |    22/   22/   26 |     240 |
| Scanning(Dual,HardSpi,subfields)       |     3/    4/   27 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22222/22225/22234 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |   430/  432/  436 |      20 |
| Max7219(HardSpi)                       |   106/  107/  111 |      20 |
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
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 4
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(Direct)                       |    14/   14/   20 |     240 |
| Scanning(Direct,subfields)             |     1/    2/   25 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Single,SoftSpi)               |    32/   32/   38 |     240 |
| Scanning(Single,SoftSpi,subfields)     |     1/    4/   43 |    3840 |
| Scanning(Single,HardSpi)               |    40/   41/   46 |     240 |
| Scanning(Single,HardSpi,subfields)     |     1/    4/   65 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Dual,SoftSpi)                 |    55/   56/   61 |     240 |
| Scanning(Dual,SoftSpi,subfields)       |     1/    5/   71 |    3840 |
| Scanning(Dual,HardSpi)                 |    40/   40/   45 |     240 |
| Scanning(Dual,HardSpi,subfields)       |     1/    5/   63 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 22399/22405/22413 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |   281/  283/  287 |      20 |
| Max7219(HardSpi)                       |   201/  202/  206 |      20 |
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
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 4
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(Direct)                       |    12/   12/   25 |     240 |
| Scanning(Direct,subfields)             |     0/    2/   25 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Single,SoftSpi)               |    29/   29/   41 |     240 |
| Scanning(Single,SoftSpi,subfields)     |     0/    4/   45 |    3840 |
| Scanning(Single,HardSpi)               |    11/   11/   24 |     240 |
| Scanning(Single,HardSpi,subfields)     |     0/    2/   23 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Dual,SoftSpi)                 |    50/   50/   59 |     240 |
| Scanning(Dual,SoftSpi,subfields)       |     0/    7/   67 |    3840 |
| Scanning(Dual,HardSpi)                 |    12/   12/   20 |     240 |
| Scanning(Dual,HardSpi,subfields)       |     0/    2/   28 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 21496/21501/21532 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |   254/  255/  271 |      20 |
| Max7219(HardSpi)                       |    61/   61/   70 |      20 |
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
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 4
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(Direct)                       |     2/    2/   11 |     240 |
| Scanning(Direct,subfields)             |     0/    1/   11 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Single,SoftSpi)               |     4/    4/   13 |     240 |
| Scanning(Single,SoftSpi,subfields)     |     0/    1/   13 |    3840 |
| Scanning(Single,HardSpi)               |     9/    9/   18 |     240 |
| Scanning(Single,HardSpi,subfields)     |     0/    1/   18 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Dual,SoftSpi)                 |     7/    7/   15 |     240 |
| Scanning(Dual,SoftSpi,subfields)       |     0/    1/   15 |    3840 |
| Scanning(Dual,HardSpi)                 |     9/    9/   17 |     240 |
| Scanning(Dual,HardSpi,subfields)       |     0/    1/   18 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 21228/21239/21252 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |    33/   33/   36 |      20 |
| Max7219(HardSpi)                       |    43/   44/   52 |      20 |
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
sizeof(LedMatrixSingleShiftRegister<SoftSpiInterface>): 16
sizeof(LedMatrixDualShiftRegister<HardSpiInterface>): 12
sizeof(LedModule): 4
sizeof(ScanningModule<LedMatrixBase, 4>): 32
sizeof(Tm1637Module<SoftWireInterface, 4>): 24
sizeof(Max7219Module<SoftSpiInterface, 8>): 28
sizeof(LedDisplay): 4
sizeof(NumberWriter): 4
sizeof(ClockWriter): 8
sizeof(TemperatureWriter): 4
sizeof(CharWriter): 4
sizeof(StringWriter): 4
sizeof(StringScroller): 12

CPU:
+----------------------------------------+-------------------+---------+
| Functionality                          |   min/  avg/  max | samples |
|----------------------------------------+-------------------+---------|
| Scanning(Direct)                       |     5/    5/    9 |     240 |
| Scanning(Direct,subfields)             |     0/    1/    9 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Single,SoftSpi)               |     9/    9/   12 |     240 |
| Scanning(Single,SoftSpi,subfields)     |     0/    1/   13 |    3840 |
| Scanning(Single,HardSpi)               |     3/    3/    6 |     240 |
| Scanning(Single,HardSpi,subfields)     |     0/    0/    6 |    3840 |
|----------------------------------------+-------------------+---------|
| Scanning(Dual,SoftSpi)                 |    16/   16/   20 |     240 |
| Scanning(Dual,SoftSpi,subfields)       |     0/    2/   20 |    3840 |
| Scanning(Dual,HardSpi)                 |     3/    3/    4 |     240 |
| Scanning(Dual,HardSpi,subfields)       |     0/    1/    7 |    3840 |
|----------------------------------------+-------------------+---------|
| Tm1637(SoftWire)                       | 21165/21166/21172 |      20 |
|----------------------------------------+-------------------+---------|
| Max7219(SoftSpi)                       |    83/   83/   86 |      20 |
| Max7219(HardSpi)                       |    17/   17/   21 |      20 |
+----------------------------------------+-------------------+---------+

```

