# Changelog

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
