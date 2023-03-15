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
 * A sketch that generates the min/avg/max duration (in microsecondes) of the
 * rendering logic of various module classes (e.g. Tm1637Module, Max7219Module,
 * Ht16k33Module, Hc595Module). See the generated README.md for more
 * information.
 *
 * For accurate I2C timing information, an HT16K33 LED module must be attached
 * to the I2C bus. Otherwise, some I2C libraries will detect the NACK from the
 * non-existent device and return early, causing the timing numbers to be
 * artificially small.
 */

#include <Arduino.h>
#include <SPI.h> // SPIClass
#include <Wire.h> // TwoWire
#include <AceCommon.h> // TimingStats
#include <AceSPI.h>
#include <AceTMI.h>
#include <AceWire.h>
#include <AceSegment.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_spi/SimpleSpiFastInterface.h>
#include <ace_spi/HardSpiFastInterface.h>
#include <ace_tmi/SimpleTmi1637FastInterface.h>
#include <ace_tmi/SimpleTmi1638FastInterface.h>
#include <ace_wire/SimpleWireFastInterface.h>
#include <ace_segment/direct/DirectFast4Module.h>
#endif

using namespace ace_spi;
using namespace ace_tmi;
using namespace ace_wire;
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

const uint8_t BIT_DELAY = 100;
const uint8_t BIT_DELAY_SHORT = 5;
const uint8_t BIT_DELAY_TM1638 = 1;

#if defined(EPOXY_DUINO)
  // numbers don't matter
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {1, 2, 3, 4};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {5, 6, 7, 8, 9, 10, 11, 12};

  // TM1637, TM1638
  const uint8_t CLK_PIN = 1;
  const uint8_t DIO_PIN = 2;
  const uint8_t STB_PIN = 3;

#elif defined(ARDUINO_ARCH_AVR)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

  // TM1637, TM1638
  const uint8_t CLK_PIN = 4;
  const uint8_t DIO_PIN = 5;
  const uint8_t STB_PIN = 6;

#elif defined(ARDUINO_ARCH_STM32)
  // I think this is the F1, because there exists a ARDUINO_ARCH_STM32F4
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};

  // TM1637, TM1638
  const uint8_t CLK_PIN = 2;
  const uint8_t DIO_PIN = 3;
  const uint8_t STB_PIN = 4;

#elif defined(ESP8266)
  // Don't have enough pins so reuse some.
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {D4, D5, D4, D5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {D6, D7, D6, D7, D6, D7, D6, D7};

  // TM1637, TM1638
  const uint8_t CLK_PIN = D4;
  const uint8_t DIO_PIN = D5;
  const uint8_t STB_PIN = D6;

#elif defined(ESP32)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {21, 22, 23, 24};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {12, 13, 14, 15, 16, 17, 18, 19};

  // TM1637, TM1638
  const uint8_t CLK_PIN = 21;
  const uint8_t DIO_PIN = 22;
  const uint8_t STB_PIN = 23;

#elif defined(TEENSYDUINO)
  // Teensy 3.2
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};

  // TM1637, TM1638
  const uint8_t CLK_PIN = 2;
  const uint8_t DIO_PIN = 3;
  const uint8_t STB_PIN = 4;

#else
  #warning Unknown hardware, using defaults which may interfere with Serial
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};

  // TM1637, TM1638
  const uint8_t CLK_PIN = 2;
  const uint8_t DIO_PIN = 3;
  const uint8_t STB_PIN = 4;

#endif

// 74HC595
const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = MOSI; // DS on 74HC595
const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595

// HT16K33
const uint8_t SDA_PIN = SDA;
const uint8_t SCL_PIN = SCL;
const uint8_t DELAY_MICROS = 1;

//------------------------------------------------------------------
// Run benchmarks.
//------------------------------------------------------------------

/** Print the result for each LedMatrix algorithm. */
static void printStats(
    const __FlashStringHelper* name,
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
void runScanningBenchmark(const __FlashStringHelper* name, LM& scanningModule) {

  for (uint8_t i = 0; i < scanningModule.size(); ++i) {
    scanningModule.setPatternAt(i, i);
  }

  // Sample for 10 frames
  uint16_t numSamples = scanningModule.getFieldsPerFrame() * 10;
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
// Direct LED Modules
//-----------------------------------------------------------------------------

// Common Anode, with transistors on Group pins
void runDirect() {
  DirectModule<NUM_DIGITS> scanningModule(
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      SEGMENT_PINS,
      DIGIT_PINS);
  DirectModule<NUM_DIGITS, NUM_SUBFIELDS> scanningModuleSubfields(
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      SEGMENT_PINS,
      DIGIT_PINS);

  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Direct(4)"), scanningModule);
  runScanningBenchmark(F("Direct(4,subfields)"), scanningModuleSubfields);
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
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND);

  DirectFast4Module<
      8, 9, 10, 16, 14, 18, 19, 15, // segment pins
      4, 5, 6, 7, // digit pins
      NUM_DIGITS,
      NUM_SUBFIELDS
  > scanningModuleSubfields(
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND);

  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("DirectFast4(4)"), scanningModule);
  runScanningBenchmark(F("DirectFast4(4,subfields)"), scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
}
#endif

//-----------------------------------------------------------------------------
// Hybrid 74HC595 and Direct LED Modules.
//-----------------------------------------------------------------------------

// Common Cathode, with transistors on Group pins
void runHybridSimpleSpi() {
  using SpiInterface = SimpleSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  HybridModule<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModuleSubfields(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hybrid(4,SimpleSpi)"), scanningModule);
  runScanningBenchmark(F("Hybrid(4,SimpleSpi,subfields)"),
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

// Common Cathode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runHybridSimpleSpiFast() {
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;

  HybridModule<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModuleSubfields(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hybrid(4,SimpleSpiFast)"), scanningModule);
  runScanningBenchmark(F("Hybrid(4,SimpleSpiFast,subfields)"),
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

// Common Cathode, with transistors on Group pins
void runHybridHardSpi() {
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(SPI, LATCH_PIN);

  HybridModule<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModuleSubfields(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hybrid(4,HardSpi)"), scanningModule);
  runScanningBenchmark(
      F("Hybrid(4,HardSpi,subfields)"), scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
// Common Cathode, with transistors on Group pins
void runHybridHardSpiFast() {
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
  SpiInterface spiInterface(SPI);

  HybridModule<SpiInterface, NUM_DIGITS> scanningModule(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModuleSubfields(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hybrid(4,HardSpiFast)"), scanningModule);
  runScanningBenchmark(F("Hybrid(4,HardSpiFast,subfields)"),
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// 74HC595 LED Modules
//-----------------------------------------------------------------------------

// Common Anode, with transistors on Group pins
void runHc595SimpleSpi() {
  using SpiInterface = SimpleSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);

  Hc595Module<SpiInterface, 8> scanningModule(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow
  );

  Hc595Module<SpiInterface, 8, NUM_SUBFIELDS> scanningModuleSubfields(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow
  );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hc595(8,SimpleSpi)"), scanningModule);
  runScanningBenchmark(
      F("Hc595(8,SimpleSpi,subfields)"), scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

// Common Anode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runHc595SimpleSpiFast() {
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;

  Hc595Module<SpiInterface, 8> scanningModule(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow
  );

  Hc595Module<SpiInterface, 8, NUM_SUBFIELDS>
      scanningModuleSubfields(
          spiInterface,
          kActiveLowPattern /*segmentOnPattern*/,
          kActiveLowPattern /*digitOnPattern*/,
          FRAMES_PER_SECOND,
          kByteOrderDigitHighSegmentLow
      );

  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hc595(8,SimpleSpiFast)"), scanningModule);
  runScanningBenchmark(F("Hc595(8,SimpleSpiFast,subfields)"),
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

// Common Anode, with transistors on Group pins
void runHc595HardSpi() {
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(SPI, LATCH_PIN);

  Hc595Module<SpiInterface, 8> scanningModule(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow
  );

  Hc595Module<SpiInterface, 8, NUM_SUBFIELDS> scanningModuleSubfields(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow
  );

  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hc595(8,HardSpi)"), scanningModule);
  runScanningBenchmark(
      F("Hc595(8,HardSpi,subfields)"), scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
// Common Anode, with transistors on Group pins
void runHc595HardSpiFast() {
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
  SpiInterface spiInterface(SPI);

  Hc595Module<SpiInterface, 8> scanningModule(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow
  );

  Hc595Module<SpiInterface, 8, NUM_SUBFIELDS> scanningModuleSubfields(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow
  );

  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark(F("Hc595(8,HardSpiFast)"), scanningModule);
  runScanningBenchmark(F("Hc595(8,HardSpiFast,subfields)"),
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  spiInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// TM1637 LED Modules
//-----------------------------------------------------------------------------

static uint8_t kTm1637Patterns[6] = {
  0x13, 0x35, 0x57, 0x79, 0x9B, 0xBD
};

template <typename LM>
void runTm1637Benchmark(
    const __FlashStringHelper* name,
    LM& ledModule,
    uint8_t numDigits,
    bool useFlushIncremental
) {
  // Run through 10 full cycles to eliminate any variances.
  // The flush() method sends all digits (and brightness) to the LED module in a
  // single call. The flushIncremental() sends only a single digit or the
  // brightness, so a full cycle takes (numDigits + 1).
  const uint16_t numSamples = useFlushIncremental
      ? (numDigits + 1) * 10
      : 10;

  timingStats.reset();
  for (uint16_t i = 0; i < numSamples; ++i) {

    // Update patterns and brightness to mark them dirty.
    for (uint8_t i = 0; i < numDigits; ++i) {
      ledModule.setPatternAt(i, kTm1637Patterns[i]);
    }
    ledModule.setBrightness(1);

    uint16_t startMicros;
    uint16_t endMicros;
    if (useFlushIncremental) {
      startMicros = micros();
      ledModule.flushIncremental();
      endMicros = micros();
    } else {
      startMicros = micros();
      ledModule.flush();
      endMicros = micros();
    }
    timingStats.update(endMicros - startMicros);
    yield();
  }

  printStats(name, timingStats, numSamples);
}

void runTm1637SimpleTmi() {
  using TmiInterface = SimpleTmi1637Interface;
  TmiInterface tmiInterface(DIO_PIN, CLK_PIN, BIT_DELAY);
  tmiInterface.begin();

  Tm1637Module<TmiInterface, 4> tm1637Module(tmiInterface);
  tm1637Module.begin();
  runTm1637Benchmark(
      F("Tm1637(4,SimpleTmi1637,100us)"), tm1637Module, 4, false);
  runTm1637Benchmark(
      F("Tm1637(4,SimpleTmi1637,100us,incremental)"), tm1637Module, 4, true);
  tm1637Module.end();

  tmiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runTm1637SimpleTmiFast() {
  using TmiInterface = SimpleTmi1637FastInterface<DIO_PIN, CLK_PIN, BIT_DELAY>;
  TmiInterface tmiInterface;
  tmiInterface.begin();

  Tm1637Module<TmiInterface, 4> tm1637Module(tmiInterface);
  tm1637Module.begin();
  runTm1637Benchmark(
      F("Tm1637(4,SimpleTmi1637Fast,100us)"), tm1637Module, 4, false);
  runTm1637Benchmark(
      F("Tm1637(4,SimpleTmi1637Fast,100us,incremental)"),
      tm1637Module, 4, true);
  tm1637Module.end();

  tmiInterface.end();
}
#endif

// Use 5 micros instead of 100 micros.
void runTm1637SimpleTmiShort() {
  using TmiInterface = SimpleTmi1637Interface;
  TmiInterface tmiInterface(DIO_PIN, CLK_PIN, BIT_DELAY_SHORT);
  tmiInterface.begin();

  Tm1637Module<TmiInterface, 4> tm1637Module(tmiInterface);
  tm1637Module.begin();
  runTm1637Benchmark(F("Tm1637(4,SimpleTmi1637,5us)"), tm1637Module, 4, false);
  runTm1637Benchmark(
      F("Tm1637(4,SimpleTmi1637,5us,incremental)"), tm1637Module, 4, true);
  tm1637Module.end();

  tmiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runTm1637SimpleTmiFastShort() {
  using TmiInterface = SimpleTmi1637FastInterface<
      DIO_PIN, CLK_PIN, BIT_DELAY_SHORT>;
  TmiInterface tmiInterface;
  tmiInterface.begin();

  Tm1637Module<TmiInterface, 4> tm1637Module(tmiInterface);
  tm1637Module.begin();
  runTm1637Benchmark(
      F("Tm1637(4,SimpleTmi1637Fast,5us)"), tm1637Module, 4, false);
  runTm1637Benchmark(
      F("Tm1637(4,SimpleTmi1637Fast,5us,incremental)"), tm1637Module, 4, true);
  tm1637Module.end();

  tmiInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// TM1638 LED Modules
//-----------------------------------------------------------------------------

static uint8_t kTm1638Patterns[8] = {
  0x13, 0x35, 0x57, 0x79, 0x9B, 0xBD, 0xDE, 0xEF
};

template <typename LM>
void runTm1638Benchmark(
    const __FlashStringHelper* name,
    LM& ledModule,
    uint8_t numDigits
) {
  // Run through 10 full cycles to eliminate any variances.
  const uint16_t numSamples = 10;

  timingStats.reset();
  for (uint16_t i = 0; i < numSamples; ++i) {
    // Update patterns and brightness to mark them dirty.
    for (uint8_t i = 0; i < numDigits; ++i) {
      ledModule.setPatternAt(i, kTm1638Patterns[i]);
    }
    ledModule.setBrightness(1);

    uint16_t startMicros;
    uint16_t endMicros;
    startMicros = micros();
    ledModule.flush();
    endMicros = micros();
    timingStats.update(endMicros - startMicros);
  }

  printStats(name, timingStats, numSamples);
}

void runTm1638SimpleTmi() {
  using TmiInterface = SimpleTmi1638Interface;
  TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY_TM1638);
  tmiInterface.begin();

  Tm1638Module<TmiInterface, 8> tm1638Module(tmiInterface);
  tm1638Module.begin();
  runTm1638Benchmark(F("Tm1638(8,SimpleTmi1638,1us)"), tm1638Module, 8);
  tm1638Module.end();

  tmiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runTm1638SimpleTmiFast() {
  using TmiInterface = SimpleTmi1638FastInterface<
      DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY_TM1638>;
  TmiInterface tmiInterface;
  tmiInterface.begin();

  Tm1638Module<TmiInterface, 8> tm1638Module(tmiInterface);
  tm1638Module.begin();
  runTm1638Benchmark(F("Tm1638(8,SimpleTmi1638Fast,1us)"), tm1638Module, 8);
  tm1638Module.end();

  tmiInterface.end();
}
#endif

void runTm1638AnodeSimpleTmi() {
  using TmiInterface = SimpleTmi1638Interface;
  TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY_TM1638);
  tmiInterface.begin();

  Tm1638AnodeModule<TmiInterface, 8> tm1638Module(tmiInterface);
  tm1638Module.begin();
  runTm1638Benchmark(F("Tm1638Anode(8,SimpleTmi1638,1us)"), tm1638Module, 8);
  tm1638Module.end();

  tmiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runTm1638AnodeSimpleTmiFast() {
  using TmiInterface = SimpleTmi1638FastInterface<
      DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY_TM1638>;
  TmiInterface tmiInterface;
  tmiInterface.begin();

  Tm1638AnodeModule<TmiInterface, 8> tm1638Module(tmiInterface);
  tm1638Module.begin();
  runTm1638Benchmark(
      F("Tm1638Anode(8,SimpleTmi1638Fast,1us)"), tm1638Module, 8);
  tm1638Module.end();

  tmiInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// MAX7219 LED Modules
//-----------------------------------------------------------------------------

template <typename LM>
void runMax7219Benchmark(const __FlashStringHelper* name, LM& ledModule) {
  ledModule.setPatternAt(0, 0x13);
  ledModule.setPatternAt(1, 0x37);
  ledModule.setPatternAt(2, 0x7F);
  ledModule.setPatternAt(3, 0xFF);

  timingStats.reset();
  const uint16_t numSamples = 20;
  for (uint16_t i = 0; i < numSamples; ++i) {
    uint16_t startMicros = micros();
    ledModule.flush();
    uint16_t endMicros = micros();
    timingStats.update(endMicros - startMicros);
  }

  printStats(name, timingStats, numSamples);
}

void runMax7219SimpleSpi() {
  using SpiInterface = SimpleSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  Max7219Module<SpiInterface, 8> max7219Module(
      spiInterface, kDigitRemapArray8Max7219);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark(F("Max7219(8,SimpleSpi)"), max7219Module);
  max7219Module.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runMax7219SimpleSpiFast() {
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
  Max7219Module<SpiInterface, 8> max7219Module(
      spiInterface, kDigitRemapArray8Max7219);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark(F("Max7219(8,SimpleSpiFast)"), max7219Module);
  max7219Module.end();
  spiInterface.end();
}
#endif

void runMax7219HardSpi() {
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(SPI, LATCH_PIN);
  Max7219Module<SpiInterface, 8> max7219Module(
      spiInterface, kDigitRemapArray8Max7219);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark(F("Max7219(8,HardSpi)"), max7219Module);
  max7219Module.end();
  spiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runMax7219HardSpiFast() {
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
  SpiInterface spiInterface(SPI);
  Max7219Module<SpiInterface, 8> max7219Module(
      spiInterface, kDigitRemapArray8Max7219);

  spiInterface.begin();
  max7219Module.begin();
  runMax7219Benchmark(F("Max7219(8,HardSpiFast)"), max7219Module);
  max7219Module.end();
  spiInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// HT16K33 LED Modules
//-----------------------------------------------------------------------------

const uint8_t HT16K33_I2C_ADDRESS = 0x70;

template <typename LM>
void runHt16k33Benchmark(const __FlashStringHelper* name, LM& ledModule) {
  ledModule.setPatternAt(0, 0x13);
  ledModule.setPatternAt(1, 0x37);
  ledModule.setPatternAt(2, 0x7F);
  ledModule.setPatternAt(3, 0xFF);

  timingStats.reset();
  const uint16_t numSamples = 20;
  for (uint16_t i = 0; i < numSamples; ++i) {
    uint16_t startMicros = micros();
    ledModule.flush();
    uint16_t endMicros = micros();
    timingStats.update(endMicros - startMicros);
  }

  printStats(name, timingStats, numSamples);
}

void runHt16k33TwoWire100() {
  using WireInterface = TwoWireInterface<TwoWire>;
  WireInterface wireInterface(Wire);
  Ht16k33Module<WireInterface, 4> ht16k33Module(
      wireInterface, HT16K33_I2C_ADDRESS);

  Wire.begin();
  Wire.setClock(100000L);
  wireInterface.begin();
  ht16k33Module.begin();
  runHt16k33Benchmark(F("Ht16k33(4,TwoWire,100kHz)"), ht16k33Module);
  ht16k33Module.end();
  wireInterface.end();
}

void runHt16k33TwoWire400() {
  using WireInterface = TwoWireInterface<TwoWire>;
  WireInterface wireInterface(Wire);
  Ht16k33Module<WireInterface, 4> ht16k33Module(
      wireInterface, HT16K33_I2C_ADDRESS);

  Wire.begin();
  Wire.setClock(400000L);
  wireInterface.begin();
  ht16k33Module.begin();
  runHt16k33Benchmark(F("Ht16k33(4,TwoWire,400kHz)"), ht16k33Module);
  ht16k33Module.end();
  wireInterface.end();
}

void runHt16k33SimpleWire() {
  using WireInterface = SimpleWireInterface;
  WireInterface wireInterface(
      SDA_PIN, SCL_PIN, DELAY_MICROS);
  Ht16k33Module<WireInterface, 4> ht16k33Module(
      wireInterface, HT16K33_I2C_ADDRESS);

  wireInterface.begin();
  ht16k33Module.begin();
  runHt16k33Benchmark(F("Ht16k33(4,SimpleWire,1us)"), ht16k33Module);
  ht16k33Module.end();
  wireInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runHt16k33SimpleWireFast() {
  using WireInterface = SimpleWireFastInterface<SDA_PIN, SCL_PIN, DELAY_MICROS>;
  WireInterface wireInterface;
  Ht16k33Module<WireInterface, 4> ht16k33Module(
      wireInterface, HT16K33_I2C_ADDRESS);

  wireInterface.begin();
  ht16k33Module.begin();
  runHt16k33Benchmark(F("Ht16k33(4,SimpleWireFast,1us)"), ht16k33Module);
  ht16k33Module.end();
  wireInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// runBenchmarks()
//-----------------------------------------------------------------------------

void runBenchmarks() {
  runDirect();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runDirectFast4();
#endif

  // HybridModule
  runHybridHardSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runHybridHardSpiFast();
#endif
  runHybridSimpleSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runHybridSimpleSpiFast();
#endif

  // Hc595Module
  runHc595HardSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runHc595HardSpiFast();
#endif
  runHc595SimpleSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runHc595SimpleSpiFast();
#endif

  // Tm1637Module
  runTm1637SimpleTmi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runTm1637SimpleTmiFast();
#endif
  runTm1637SimpleTmiShort();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runTm1637SimpleTmiFastShort();
#endif

  // Tm1638Module
  runTm1638SimpleTmi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runTm1638SimpleTmiFast();
#endif

  // Tm1638AnodeModule
  runTm1638AnodeSimpleTmi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runTm1638AnodeSimpleTmiFast();
#endif

  // Max7219Module
  runMax7219HardSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runMax7219HardSpiFast();
#endif
  runMax7219SimpleSpi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runMax7219SimpleSpiFast();
#endif

  // Ht16k33Module
  runHt16k33TwoWire100();
  runHt16k33TwoWire400();
  runHt16k33SimpleWire();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runHt16k33SimpleWireFast();
#endif
}

//-----------------------------------------------------------------------------
// sizeof()
//-----------------------------------------------------------------------------

void printSizeOf() {
  // LedMatrixDirect, LedMatrixDirectFast

  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirect<>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirect<>));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirectFast4<6..13, 2..5>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirectFast4<
      6, 7, 8, 9, 10, 11, 12, 13,
      2, 3, 4, 5
  >));
#endif

  // LedMatrix*, ScanningModule

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixSingleHc595<SimpleSpiInterface>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixSingleHc595<SimpleSpiInterface>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixDualHc595<HardSpiInterface>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(LedMatrixDualHc595<HardSpiInterface<SPIClass>>));

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

  // HybridModule, Hc595Module, Tm1637Module, Max7219Module, Ht16k33Module

  SERIAL_PORT_MONITOR.print(F("sizeof(HybridModule<SimpleSpiInterface, 4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(HybridModule<SimpleSpiInterface, 4>));

  SERIAL_PORT_MONITOR.print(F("sizeof(Hc595Module<SimpleSpiInterface, 8>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Hc595Module<SimpleSpiInterface, 8>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(Tm1637Module<SimpleTmi1637Interface, 4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Tm1637Module<SimpleTmi1637Interface, 4>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(Tm1637Module<SimpleTmi1637Interface, 6>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Tm1637Module<SimpleTmi1637Interface, 6>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(Tm1638Module<SimpleTmi1638Interface, 8>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Tm1638Module<SimpleTmi1638Interface, 8>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(Tm1638AnodeModule<SimpleTmi1638Interface, 8>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(Tm1638AnodeModule<SimpleTmi1638Interface, 8>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(Max7219Module<SimpleSpiInterface, 8>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Max7219Module<SimpleSpiInterface, 8>));

  SERIAL_PORT_MONITOR.print(F("sizeof(Ht16k33Module<TwoWireInterface, 4>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(Ht16k33Module<TwoWireInterface<TwoWire>, 4>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(Ht16k33Module<SimpleWireInterface, 4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Ht16k33Module<SimpleWireInterface, 4>));
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

  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
