/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * A sketch that generates the min/avg/max (in microsecondes) benchmarks of
 * ScanningModule::renderFieldNow() for various configurations of LedMatrix.
 * The output is an space-separate list of numbers which can be fed into
 * `generate_table.awk` to extract a human-readable ASCII table that can be
 * pasted directly into the README.md file as a code block.
 *
 * Each runXxx() function configures the ScanningModule object and all of its
 * dependencies. It calls ScanningModule::renderFieldNow() for the number of
 * times returned by `ScanningModule::getFieldsPerSecond()` so that the entire
 * frame is sampled. The duration of that function call in microseconds is
 * collected, then printed out with the min/avg/max numbers.
 */

#include <stdio.h>
#include <Arduino.h>
#include <AceCommon.h> // TimingStats
#include <AceSegment.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_segment/hw/SoftSpiFastInterface.h>
#include <ace_segment/hw/HardSpiFastInterface.h>
#include <ace_segment/hw/SoftWireFastInterface.h>
#include <ace_segment/direct/DirectFast4Module.h>
#endif

using namespace ace_segment;
using ace_common::TimingStats;

#if ! defined(SERIAL_PORT_MONITOR)
#define SERIAL_PORT_MONITOR Serial
#endif

//------------------------------------------------------------------
// Setup for AceSegment
//------------------------------------------------------------------

const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;

const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;

#if defined(EPOXY_DUINO)
  // numbers don't matter
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {1, 2, 3, 4};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {5, 6, 7, 8, 9, 10, 11, 12};

  // TM1637
  const uint8_t CLK_PIN = 1;
  const uint8_t DIO_PIN = 2;
  const uint16_t BIT_DELAY = 100;

#elif defined(ARDUINO_ARCH_AVR)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

  // TM1637
  const uint8_t CLK_PIN = 4;
  const uint8_t DIO_PIN = 5;
  const uint16_t BIT_DELAY = 100;

#elif defined(ARDUINO_ARCH_SAMD)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};

  // TM1637
  const uint8_t CLK_PIN = 2;
  const uint8_t DIO_PIN = 3;
  const uint16_t BIT_DELAY = 100;

#elif defined(ARDUINO_ARCH_STM32)
  // I think this is the F1, because there exists a ARDUINO_ARCH_STM32F4
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};

  // TM1637
  const uint8_t CLK_PIN = 2;
  const uint8_t DIO_PIN = 3;
  const uint16_t BIT_DELAY = 100;

#elif defined(ESP8266)
  // Don't have enough pins so reuse some.
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {D4, D5, D4, D5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {D6, D7, D6, D7, D6, D7, D6, D7};

  // TM1637
  const uint8_t CLK_PIN = D4;
  const uint8_t DIO_PIN = D5;
  const uint16_t BIT_DELAY = 100;

#elif defined(ESP32)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {21, 22, 23, 24};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {12, 13, 14, 15, 16, 17, 18, 19};

  // TM1637
  const uint8_t CLK_PIN = 21;
  const uint8_t DIO_PIN = 22;
  const uint16_t BIT_DELAY = 100;

#elif defined(TEENSYDUINO)
  // Teensy 3.2
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};

  // TM1637
  const uint8_t CLK_PIN = 2;
  const uint8_t DIO_PIN = 3;
  const uint16_t BIT_DELAY = 100;

#else
  #warning Unknown hardware, using defaults which may interfere with Serial
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};

  // TM1637
  const uint8_t CLK_PIN = 2;
  const uint8_t DIO_PIN = 3;
  const uint16_t BIT_DELAY = 100;

#endif

  // 74HC595
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = MOSI; // DS on 74HC595
const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595

//------------------------------------------------------------------
// Run benchmarks.
//------------------------------------------------------------------

/** Print the result for each LedMatrix algorithm. */
static void printStats(
    const char* name,
    const TimingStats& stats,
    uint16_t numSamples) {
  SERIAL_PORT_MONITOR.print(name);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMin());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getAvg());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMax());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.println(numSamples);
}

TimingStats timingStats;

template <typename LM>
void runScanningBenchmark(const char* name, LM& scanningModule) {

  scanningModule.setPatternAt(0, 0x13);
  scanningModule.setPatternAt(0, 0x37);
  scanningModule.setPatternAt(0, 0x7F);
  scanningModule.setPatternAt(0, 0xFF);

  uint16_t numSamples = scanningModule.getFieldsPerSecond();
  timingStats.reset();
  for (uint16_t i = 0; i < numSamples; i++) {
    uint16_t startMicros = micros();
    scanningModule.renderFieldNow();
    uint16_t endMicros = micros();
    timingStats.update(endMicros - startMicros);
    yield();
  }

  printStats(name, timingStats, numSamples);
}

//-----------------------------------------------------------------------------

// Common Anode, with transistors on Group pins
void runDirect() {
  DirectModule<NUM_DIGITS> scanningModule(
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      SEGMENT_PINS,
      DIGIT_PINS);
  DirectModule<NUM_DIGITS, NUM_SUBFIELDS> scanningModuleSubfields(
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      SEGMENT_PINS,
      DIGIT_PINS);

  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Direct", scanningModule);
  runScanningBenchmark("Direct(subfields)", scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
// Common Anode, with transistors on Group pins
void runDirectFast4() {
  DirectFast4Module<
      8, 9, 10, 16, 14, 18, 19, 15, // segment pins
      4, 5, 6, 7, // digit pins
      NUM_DIGITS
  > scanningModule(
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND);

  DirectFast4Module<
      8, 9, 10, 16, 14, 18, 19, 15, // segment pins
      4, 5, 6, 7, // digit pins
      NUM_DIGITS,
      NUM_SUBFIELDS
  > scanningModuleSubfields(
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND);

  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("DirectFast4", scanningModule);
  runScanningBenchmark("DirectFast4(subfields)", scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
}
#endif

//-----------------------------------------------------------------------------

// Common Cathode, with transistors on Group pins
void runSingleHc595SoftSpi() {
  using SpiInterface = SoftSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  SingleHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  SingleHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  > scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("SingleHc595(SoftSpi)", scanningModule);
  runScanningBenchmark("SingleHc595(SoftSpi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

// Common Cathode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runSingleHc595SoftSpiFast() {
  using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;

  SingleHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  SingleHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  > scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("SingleHc595(SoftSpiFast)", scanningModule);
  runScanningBenchmark("SingleHc595(SoftSpiFast,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

// Common Cathode, with transistors on Group pins
void runSingleHc595HardSpi() {
  using SpiInterface = HardSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  SingleHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  SingleHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  > scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("SingleHc595(HardSpi)", scanningModule);
  runScanningBenchmark("SingleHc595(HardSpi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
// Common Cathode, with transistors on Group pins
void runSingleHc595HardSpiFast() {
  using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;

  SingleHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  SingleHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  >
  scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("SingleHc595(HardSpiFast)", scanningModule);
  runScanningBenchmark("SingleHc595(HardSpiFast,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

//-----------------------------------------------------------------------------

// Common Anode, with transistors on Group pins
void runDualHc595SoftSpi() {
  using SpiInterface = SoftSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  DualHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  DualHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  > scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("DualHc595(SoftSpi)", scanningModule);
  runScanningBenchmark("DualHc595(SoftSpi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

// Common Anode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runDualHc595SoftSpiFast() {
  using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;

  DualHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  DualHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  >
  scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("DualHc595(SoftSpiFast)", scanningModule);
  runScanningBenchmark("DualHc595(SoftSpiFast,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

// Common Anode, with transistors on Group pins
void runDualHc595HardSpi() {
  using SpiInterface = HardSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  DualHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  DualHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  >
  scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("DualHc595(HardSpi)", scanningModule);
  runScanningBenchmark("DualHc595(HardSpi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
// Common Anode, with transistors on Group pins
void runDualHc595HardSpiFast() {
  using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;

  DualHc595Module<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  DualHc595Module<
      SpiInterface, NUM_DIGITS, NUM_SUBFIELDS
  >
  scanningModuleSubfields(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("DualHc595(HardSpiFast)", scanningModule);
  runScanningBenchmark("DualHc595(HardSpiFast,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

//-----------------------------------------------------------------------------

template <typename LM>
void runTm1637Benchmark(const char* name, LM& ledModule) {
  ledModule.setPatternAt(0, 0x13);
  ledModule.setPatternAt(0, 0x37);
  ledModule.setPatternAt(0, 0x7F);
  ledModule.setPatternAt(0, 0xFF);

  timingStats.reset();
  const uint16_t numSamples = 20;
  for (uint16_t i = 0; i < numSamples; ++i) {
    uint16_t startMicros = micros();
    ledModule.flush();
    uint16_t endMicros = micros();
    timingStats.update(endMicros - startMicros);
    yield();
  }

  printStats(name, timingStats, numSamples);
}

void runTm1637ModuleSoftWire() {
  using WireInterface = SoftWireInterface;
  WireInterface wireInterface(CLK_PIN, DIO_PIN, BIT_DELAY);
  Tm1637Module<WireInterface, NUM_DIGITS> tm1637Module(wireInterface);

  wireInterface.begin();
  tm1637Module.begin();
  runTm1637Benchmark("Tm1637(SoftWire)", tm1637Module);
  tm1637Module.end();
  wireInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runTm1637ModuleSoftWireFast() {
  using WireInterface = SoftWireFastInterface<CLK_PIN, DIO_PIN, BIT_DELAY>;
  WireInterface wireInterface;
  Tm1637Module<WireInterface, NUM_DIGITS> tm1637Module(wireInterface);

  wireInterface.begin();
  tm1637Module.begin();
  runTm1637Benchmark("Tm1637(SoftWireFast)", tm1637Module);
  tm1637Module.end();
  wireInterface.end();
}
#endif

//-----------------------------------------------------------------------------

template <typename LM>
void runMax7219Benchmark(const char* name, LM& ledModule) {
  ledModule.setPatternAt(0, 0x13);
  ledModule.setPatternAt(0, 0x37);
  ledModule.setPatternAt(0, 0x7F);
  ledModule.setPatternAt(0, 0xFF);

  timingStats.reset();
  const uint16_t numSamples = 20;
  for (uint16_t i = 0; i < numSamples; ++i) {
    uint16_t startMicros = micros();
    ledModule.flush();
    uint16_t endMicros = micros();
    timingStats.update(endMicros - startMicros);
    yield();
  }

  printStats(name, timingStats, numSamples);
}

void runMax7219SoftSpi() {
  using SpiInterface = SoftSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
      spiInterface, kEightDigitRemapArray);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark("Max7219(SoftSpi)", max7219Module);
  max7219Module.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runMax7219SoftSpiFast() {
  using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
  Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
      spiInterface, kEightDigitRemapArray);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark("Max7219(SoftSpiFast)", max7219Module);
  max7219Module.end();
  spiInterface.end();
}
#endif

void runMax7219HardSpi() {
  using SpiInterface = HardSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
      spiInterface, kEightDigitRemapArray);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark("Max7219(HardSpi)", max7219Module);
  max7219Module.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runMax7219HardSpiFast() {
  using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
  Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
      spiInterface, kEightDigitRemapArray);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark("Max7219(HardSpiFast)", max7219Module);
  max7219Module.end();
  spiInterface.end();
}
#endif

//-----------------------------------------------------------------------------

void runBenchmarks() {
  runDirect();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runDirectFast4();
#endif

  runSingleHc595SoftSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runSingleHc595SoftSpiFast();
#endif

  runSingleHc595HardSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runSingleHc595HardSpiFast();
#endif

  runDualHc595SoftSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runDualHc595SoftSpiFast();
#endif

  runDualHc595HardSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runDualHc595HardSpiFast();
#endif

  runTm1637ModuleSoftWire();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runTm1637ModuleSoftWireFast();
#endif

  runMax7219SoftSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runMax7219SoftSpiFast();
#endif

  runMax7219HardSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runMax7219HardSpiFast();
#endif
}

//-----------------------------------------------------------------------------

void printSizeOf() {
  SERIAL_PORT_MONITOR.print(F("sizeof(SoftWireInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(SoftWireInterface));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(SoftWireFastInterface<4, 5, 100>): "));
  SERIAL_PORT_MONITOR.println(sizeof(SoftWireFastInterface<4, 5, 100>));
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(SoftSpiInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(SoftSpiInterface));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(SoftSpiFastInterface<11, 12, 13>): "));
  SERIAL_PORT_MONITOR.println(sizeof(SoftSpiFastInterface<11, 12, 13>));
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(HardSpiInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(HardSpiInterface));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(HardSpiFastInterface<11, 12, 13>): "));
  SERIAL_PORT_MONITOR.println(sizeof(HardSpiFastInterface<11, 12, 13>));
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirect<>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirect<>));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirectFast4<6..13, 2..5>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirectFast4<
      6, 7, 8, 9, 10, 11, 12, 13,
      2, 3, 4, 5
  >));
#endif

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixSingleHc595<SoftSpiInterface>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixSingleHc595<SoftSpiInterface>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixDualHc595<HardSpiInterface>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDualHc595<HardSpiInterface>));

  SERIAL_PORT_MONITOR.print(F("sizeof(LedModule): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedModule));

  SERIAL_PORT_MONITOR.print( F("sizeof(ScanningModule<LedMatrixBase, 4>): "));
  SERIAL_PORT_MONITOR.println( sizeof(ScanningModule<LedMatrixBase, 4>));

  SERIAL_PORT_MONITOR.print( F("sizeof(DirectModule<4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(DirectModule<4>));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print( F("sizeof(DirectFast4Module<...>): "));
  SERIAL_PORT_MONITOR.println(sizeof(DirectFast4Module<
      8, 9, 10, 16, 14, 18, 19, 15, // segment pins
      4, 5, 6, 7, // digit pins
      NUM_DIGITS
  >));
#endif

  SERIAL_PORT_MONITOR.print(
      F("sizeof(SingleHc595Module<SoftSpiInterface, 4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(SingleHc595Module<SoftSpiInterface, 4>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(DualHc595Module<SoftSpiInterface, 4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(DualHc595Module<SoftSpiInterface, 4>));

  SERIAL_PORT_MONITOR.print(F("sizeof(Tm1637Module<SoftWireInterface, 4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Tm1637Module<SoftWireInterface, 4>));

  SERIAL_PORT_MONITOR.print(F("sizeof(Max7219Module<SoftSpiInterface, 8>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Max7219Module<SoftSpiInterface, 8>));

  SERIAL_PORT_MONITOR.print(F("sizeof(LedDisplay): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedDisplay));

  SERIAL_PORT_MONITOR.print(F("sizeof(NumberWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(NumberWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(ClockWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(ClockWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(TemperatureWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(TemperatureWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(CharWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(CharWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(StringWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(StringWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(StringScroller): "));
  SERIAL_PORT_MONITOR.println(sizeof(StringScroller));
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait for Leonardo/Micro

  SERIAL_PORT_MONITOR.println(F("SIZEOF"));
  printSizeOf();

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));
  runBenchmarks();

  SERIAL_PORT_MONITOR.println("END");

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
