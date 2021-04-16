# Changelog

* Unreleased
    * Add `Tm1637Display` class to support LED modules using the TM1637 chip.
    * Simplify `LedDisplay` base class API.
    * Add `TemperatureWriter`.
    * Add `remapArray` to `Tm1637Display::begin()` to support logical reordering
      of the LED digits to support certain 6-digit LED modules which are
      physically wired out of order.
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
        * SpiAdapter
            * `SwSpiAdapter`
            * `HwSpiAdapter`
        * LedMatrix
            * `LedMatrixDirect`
            * `LedMatrixSingleShiftRegister`
            * `LedMatrixDualShiftRegister`
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
            * `#include <ace_segment/hw/SwSpiAdapterFast.h>`
            * `#include <ace_segment/scanning/LedMatrixDirectFast.h>`
            * `#include <ace_segment/tm1637/Tm1637DriverFast.h>`
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
