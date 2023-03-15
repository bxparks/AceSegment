# Changelog

* Unreleased
* 0.13.0 (2023-03-15)
    * `LedModule.h`
        * Add `size()` as alternate form of `getNumDigits()`. Old version
          retained for backwards compatibility.
        * Pull `setDecimalPointAt()` from `PatternWriter` down
          to the `LedModule` layer. It's a low-level operation, seems more
          appropriate at the `LedModule` layer.
* 0.12.0 (2022-03-01)
    * Fix invalid pins in `examples/Tm1638Demo` on ESP32 dev board.
    * Add `uint32_t Tm1638Module::readButtons()` method.
        * Calls `SimpleTmi1638Interface::read()` to read the keypad scans from
          the TM1638 controller.
        * Returns a `uint32_t` encoded with the states of the 3x8=24 possible
          buttons supported by the TM1638.
    * Add `uint8_t Tm1637Module::readButtons()` method.
        * Calls `SimpleTmi1637Interface::read()` to read the keypad scans from
          the TM1638 controller.
        * Returns a `uint8_t` encoded with the location of the single button
          that was pressed. TM1637 supports only a single button pressed at
          once.
    * Add `Tm1638AnodeModule` which handles a TM1638 LED Module using Common
      Anode LEDs instead of the usual Common Cathode.
        * The `SEGn` and `GRn` lines are switched, so the
          `Tm1638AnodeModule::flush()` has to loop over the same segment for
          all the digits to construct the `GRIDn` byte to send to the controller
          chip.
        * Add `examples/Tm1638AnodeDemo`.
* 0.11.0 (2022-02-02)
    * **Breaking Change** Upgrade to AceTMI v0.5 which renames TM1637 classes
      to be consistent with the TM1638 classes:
        * `SimpleTmiInterface` to `SimpleTmi1637Interface`
        * `SimpleTmiFastInterface` to `SimpleTmi1637FastInterface`
* 0.10.0 (2022-02-02)
    * Add support for TM1638 LED modules using the `Tm1638Module`.
        * Depends on AceTMI library v0.4.
    * Upgrade tool chain:
        * Arduino IDE from 1.8.13 to 1.8.19
        * Arduino AVR from 1.8.3 to 1.8.4
        * STM32duino from 2.0.0 to 2.2.0
        * ESP32 from 1.0.6 to 2.0.2
        * Teensyduino from 1.53 to 1.56
    * Add note about interrupt-safety regarding 
      [Hc595InterruptDemo](examples/Hc595InterruptDemo).
* 0.9.1 (2021-08-17)
    * Internal updates to maintain compatibility with changes in AceTMI, AceSPI,
      and AceWire.
    * First public release.
* 0.9 (2021-08-10)
    * Move Writer classes to
      [AceSegmentWriter](https://github.com/bxparks/AceSegment) library.
* 0.8.2 (2021-08-09)
    * Make `LedModule::getPatternAt()` into a `const` function.
    * Change `LedModule` methods into non-virtual. There are no more virtual
      methods in the entire library. Saves 10-60 byte of flash for Module
      classes on AVR, and another 10-70 bytes of flash for the Writer classes.
    * Templatize Writer classes on `T_LED_MODULE` instead of hardcoding the
      dependency to `LedModule`.
    * Add `isFlushRequired()` to modules which implement the `flush()` method
      (`Tm1637Module`, `Max7219Module`, `Ht16k33Module`).
        * Clear dirty flags as needed after `flush()`. Adds about 8 bytes of
          flash on AVR.
* 0.8.1 (2021-07-31)
    * Add `HelloTm1636`, `HelloMax7219`, `HelloHt16k33`, and `HelloHc595`
      examples to `examples/` and `README.md`.
    * Clean up methods to retrieve underlying `patternWriter()`, `charWriter()`,
      `numberWriter()`, etc in various Writer classes.
* 0.8 (2021-07-30)
    * Move I2C address into `beginTransmission()` method of `TwoWireInterface`,
      `SimpleWireInterface` and `SimpleWireFastInterface`.
        * More consistent with `TwoWire` class API.
        * Allows multiple HT16K33 LED modules with different addresses to
          share a single `WireInterface`.
    * Add caution that `delayMicroseconds()` on AVR is not accurate for small
      (less than 10) microseconds.
    * Extract `hw/*Spi*Interface` classes into new
      [AceSPI](https://github.com/bxparks/AceSPI) library.
    * Extract `hw/*Tmi*Interface` classes into new
      [AceTMI](https://github.com/bxparks/AceTMI) library.
    * Extract `hw/*Wire*Interface` classes into new
      [AceWire](https://github.com/bxparks/AceWire) library.
    * `examples/WriterTester`: renamed from `examples/AceSegmentTester`
    * Delete `keywords.txt`, takes too much effort to maintain.
    * Add examples/MemoryBenchmark and examples/AutoBenchmark to gather
      memory and CPU usage of various I2C implementations.
    * Rename `SoftSpi*Interface` to `SimpleSpi*Interface`.
    * Rename `SoftTmi*Interface` to `SimpleTmi*Interface`.
* 0.7 (2021-06-16)
    * Support HT16K33 LED modules
        * Add `Ht16k33Module` class to support 4-digit LED display from
          Adafruit or one of its clones.
        * Add `TwoWireInterface` as a thin abstraction between `TwoWire` class
          and the `Ht16k33Module`, to avoid including `<Wire.h>` which pulls in
          about 1000 bytes of flash even if `Ht16k33Module` is never used.
        * Add `SimpleWireInterface`, a very simple software implementation of
          I2C that supports sending only.
        * Add `SimpleWireFastInterface`, same as `SimpleWireInterface` but
          using one of the `digitalWriteFast` libraries.
    * Improve `HardSpiInterface` and `HardSpiFastInterface`
        * Fix Hardware SPI on ESP8266.
            * Defer the initialization to `SPIClass::begin()` which knows to
              call `pinMode(pin, SPECIAL)` instead of `pinMode(pin, OUTPUT)`
              on the SCK and MOSI pins.
            * Call `SPIClass::setHwCs(false)` to allow `HardSpiInterface` to
              control the CS/SS pin, instead of being managed by the `SPIClass`.
        * Add `SPIClass` as template parameter.
        * Remove `dataPin` and `clockPin` parameters, since these pins
          are implicit in the `SPIClass` object.
        * Small reduction of flash (~30 bytes) and static memory (~2 bytes)
          on AVR.
    * Change order of `dioPin` and `clkPin` parameters in `SoftTmiInterface` and
      `SoftTmiFastInterface` to match the order of `sda` and `scl` pins of
      `TwoWire` and `SimpleWireInterface` classes.
    * Rename `LedDisplay` to `PatternWriter` to remove extra layer of
      abstraction.
        * Pass `LedModule` into `Writer` classes (except `StringWriter` and
          `StringScroller` which depend on `CharWriter`).
    * Fix results of `MemoryBenchmark` for TeensyDuino so that the BASELINE
      numbers include 3200 bytes of flash and 1100 bytes of static RAM from
      pulling in `malloc()` and `free()`.
* 0.6 (2021-05-26)
    * `HardSpiInterface`
        * Add support for microcontrollers with 2 SPI buses (ESP32 and STM32F1).
        * Reduce maximum SPI speed from 20 MHz to 8 MHz, since the MAX7219 chip
          cannot handle greater than 16 MHz. Makes no difference with the slower
          processors. On the fastest processors (ESP8266, ES32, and Teensy 3.2),
          this makes the AutoBenchmark for `HardSpiInterace` 5-10% slower. But
          those faster numbers were fake, because they would not have worked on
          a real MAX7219 chip.
        * Move `digitalWrite()` calls to the `CS/SS/Latch` pin inside the
          `beginTransaction()` and `endTransaction()` code block.
    * TM1637
        * Rename `WireInterface` to `TmiInterface` (and `WireFastInterface` to
          `TmiFastInteface`). The `Tmi` stands for `TM1637 Interface`. This
          avoid confusion with a future `WireInterface` which uses the actual
          I2C protocol.
        * Fix bug in `Tm1637Module::flushIncremental()` introduced in 0c37b29a
          which prevented mFlushStage from incrementing properly.
        * Verify that the 10 nF capacitors on the DIO and CLK lines of the
          TM1637 LED modules from diymore.cc can be removed. This allows the
          `bitDelay` to be reduced from 100 microseconds to 1-2 microseconds,
          allowing `Tm1637Module::flush()` and `flushIncremental()` to be
          50-100X faster.
    * `examples/AceSegmentTester`
        * Renamed from `AceSegmentDemo` to make it more obvious that this is an
          internal testing tool, not a demo program.
        * Simplify button handling to use only Clicked and LongPressed events.
          Using DoubleClicked caused too many usability issues with these
          buttons.
    * `examples/Hc595InterruptDemo`
        * Use `TimerOne` library (https://github.com/PaulStoffregen/TimerOne)
          instead of manual interrupt configuration code. The manual
          configuration was unmaintainable across different architectures
          because every microcontroller does it slightly differently.
    * `LevelWriter`
        * Write the specified number of vertical bars (2 bars per digit) to the
          LED display.
* 0.5 (2021-05-14)
    * Extract hardware dependent API from `LedDisplay` into `LedModule`.
    * Create convenience subclasses of `LedModule`:
        * Add `Tm1637Module` class to support LED modules using the TM1637 chip.
        * Add `Max7219Module` class that supports an 8-digit LED modules using a
          single MAX7219 chip.
        * Add `Hc595Module` class to support 8-digit LED modules using dual
          74HC595 shift register chips.
        * Add `HybridModule` class to support modules using a single 74HC595
          shift regsiter on segments, with direct connects to the digit pins.
        * Add `DirectModule` class to support modules whose segment and digit
          pins are directly connected to the GPIO pins.
    * Add support for `remapArray` that maps logical positions to physical
      positions.
        * Handles off-the-shelf LED modules whose digits are wired out of order.
          This includes the 6-digit TM1637 module, the 8-digit MAX7219 module,
          and the 8-digit 74HC595 module.
    * Simplify `LedDisplay` base class API.
    * Add `TemperatureWriter` and `StringScroller`.
    * Huge rewrite of README.md.
        * Move low-level description of `ScanningModule` and `LedMatrixXxx` to
          `docs/scanning_module.md`.
    * Upgrade ESP32 Core from 1.0.4 to 1.0.6. No signficant change detected.
    * Add preliminary support for ATtiny85.
* 0.4 (2021-04-09)
    * A complete refactoring of the previous v0.3 version, which I could not
      understand anymore.
        * Made things faster, smaller, easier to understand, more extensible,
          and hopefully easier to maintain.
        * Remove inheritance, virtual destructors and methods
        * Almost all classes are templatized and do not have virtual functions
        * Exception is `LedDisplay` which is the base interface virtual
            functions. This is a high level function so the CPU and memory
            consumption should be minimal.
        * Remove virtual destructors for all clases, saving ~600 bytes on AVR.
    * Build process
        * Add Makefiles to run tests and compile examples using EpoxyDuino.
        * Add GitHub workflow continuous integration.
    * Add `examples/MemoryBenchmark` and `examples/AutoBenchmark` programs
      to capture memory usages and CPU benchmarks, following the pattern for
      most of my other libraries.
    * New, vastly simplified class hierarchy.
        * `Hardware`
            * low-level accessors to GPIO pins and timers
        * SpiInterface
            * `SoftSpiInterface`
            * `HardSpiInterface`
        * LedMatrix
            * `LedMatrixDirect`
            * `LedMatrixSingleHc595`
            * `LedMatrixDualHc595`
        * `LedDisplay`
            * `ScanningDisplay`
        * Writers
            * `NumberWriter`
            * `ClockWriter`
            * `CharWriter`
            * `StringWriter`
        * Remove `Renderer` and `Driver` subclasses.
    * LedMatrix wiring
        * `LedMatrix` classes are 100% templatized, they no longer form
          a class hierarchy, nor use any virtual functions. This is important
          because these are performance critical classes.
        * Digit and segment value polarities (active low, active high) are now
          specified using only 2 XOR bit masks, instead of asking 3 bools
          (common cathode or not, transitors on segments, transistors on
          digits).
    * `ScanningDisplay`
        * Combines the previous `Driver` and `Renderer` classes into a single
          class, with templatized parameters.
        * Handles the allocation of the LED segment bit `patterns` array, using
          template parameters to determine the size. No heap allocation
          performed.
    * Writer classes
        * All Writer classes (NumberWriter, ClockWriter, CharWriter and
          StringWriter) classes refererence just the `LedDisplay` interface.
        * Renamed `HexWriter` to more general `NumberWriter`.
        * Implement generic `NumberWriter::writeUnsignedDecimalAt()` and
          `writeSignedDecimalAt()` methods, with support for negative and
          positive `boxSize` to get left or right justification within the box.
    * Support `digitalWriteFast()` on AVR
        * https://github.com/NicksonYap/digitalWriteFast
        * These GPIO functions require the pin number and output values to be
          compile-time attempted to use code generation using Python scripts,
          but they were unmaintainable.
        * Replace code generation Python scripts with C++ templates which should
          be easier to maintain.
        * Since these classes depend on an external library, the headers must be
          manually included:
            * `#include <ace_segment/hw/SoftSpiFastInterface.h>`
            * `#include <ace_segment/hw/SoftTmiFastInterface.h>`
            * `#include <ace_segment/scanning/LedMatrixDirectFast4.h>`
    * Resource consumption
        * Reduce flash consumption on AVR by 70-80%, from 4-4.3 kB down
          to 700-1200 bytes.
        * CPU benchmarks down to ~20 microseconds per each call to
          `renderFieldNow()`, using hardware SPI, or software SPI using
          `digitalWriteFast()` on AVR processors
        * Single digit microseconds on ESP32 and Teensy 3.2.
    * Brightness control using pulse width modulation (PWM)
        * The brightness of each digit is controlled independently as before.
    * Removed features
        * Blinking digits removed, can be implemented from the outside.
        * Pulsating digits removed, can be implemented from outside.
        * All `Styler` classes were removed. Too complicated for marginally
          useful features.
* 0.3.0 (2018-04-30)
    * Extract and generalize blinking and pulsing code to support custom styles.
    * Add support for going to sleep and waking up from sleep.
    * Add AutoBenchmark sketch to auto generate CPU benchmarks for various
      configurations.
    * Support inversion of logic values caused by transistors on group pins
      which handle high current.
    * Add Renderer::clear().
    * Add Driver::finish() to blank the LED display at the end of unit tests
      or AutoBenchmark.
* 0.2.0 (2018-04-12)
    * Support `digitalWriteFast()` through `fast_driver.py` code generator.
    * Gather flash and static memory consumption numbers into README.md.
    * Add `HexWriter` class, useful when `CharWriter` is overkill.
    * Support resource creation without `DriverBuilder` by storing flag to
      indicate memory ownership.
* 0.1.0 (2018-04-10)
    * Support 74HC595 serial-to-parallel converter chip (fixes #3).
    * Add unit tests for all major layers.
    * Decouple bit rendering from the wiring of the LED display.
    * Add support for hardware SPI.
    * Add doxygen docs.
* (2018-04-02)
    * Initial upload to GitHub.
