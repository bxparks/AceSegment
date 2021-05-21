# Changelog

* Unreleased
    * HardSpiInterface
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
        * Verify that the 20 nF capacitors on the DIO and CLK lines of the
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
