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
#include <ace_segment/hw/FastSwSpiInterface.h>
#include <ace_segment/scanning/LedMatrixDirectFast.h>
#include <ace_segment/tm1637/Tm1637DriverFast.h>
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

// Common Anode, with transistors on Group pins
void runDirect() {
  using LedMatrix = LedMatrixDirect<>;
  LedMatrix ledMatrix(
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS,
      NUM_SEGMENTS,
      SEGMENT_PINS);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(direct)",
      scanningModule);
  runScanningBenchmark("Scanning(direct,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
// Common Anode, with transistors on Group pins
void runDirectFast() {
  using LedMatrix = LedMatrixDirectFast<
      4, 5, 6, 7,
      8, 9, 10, 16, 14, 18, 19, 15>;
  LedMatrix ledMatrix(
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(direct_fast)",
      scanningModule);
  runScanningBenchmark("Scanning(direct_fast,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
}
#endif

// Common Cathode, with transistors on Group pins
void runSingleShiftRegisterSwSpi() {
  SwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleShiftRegister<SwSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(single,sw_spi)",
      scanningModule);
  runScanningBenchmark("Scanning(single,sw_spi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
  spiInterface.end();
}

// Common Cathode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runSingleShiftRegisterSwSpiFast() {
  using SpiInterface = FastSwSpiInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
  using LedMatrix = LedMatrixSingleShiftRegister<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(single,sw_spi_fast)",
      scanningModule);
  runScanningBenchmark("Scanning(single,sw_spi_fast,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
  spiInterface.end();
}
#endif

// Common Cathode, with transistors on Group pins
void runSingleShiftRegisterHwSpi() {
  HwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleShiftRegister<HwSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(single,hw_spi)",
      scanningModule);
  runScanningBenchmark("Scanning(single,hw_spi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
  spiInterface.end();
}

// Common Anode, with transistors on Group pins
void runDualShiftRegisterSwSpi() {
  SwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualShiftRegister<SwSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(dual,sw_spi)", scanningModule);
  runScanningBenchmark("Scanning(dual,sw_spi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
  spiInterface.end();
}

// Common Anode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runDualShiftRegisterSwSpiFast() {
  using SpiInterface = FastSwSpiInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
  using LedMatrix = LedMatrixDualShiftRegister<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(dual,sw_spi_fast)", scanningModule);
  runScanningBenchmark("Scanning(dual,sw_spi_fast,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
  spiInterface.end();
}
#endif

// Common Anode, with transistors on Group pins
void runDualShiftRegisterHwSpi() {
  HwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualShiftRegister<HwSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  ScanningModule<LedMatrix, NUM_DIGITS, 1> scanningModule(
      ledMatrix, FRAMES_PER_SECOND);
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningModuleSubfields(ledMatrix, FRAMES_PER_SECOND);

  spiInterface.begin();
  ledMatrix.begin();
  scanningModule.begin();
  scanningModuleSubfields.begin();
  runScanningBenchmark("Scanning(dual,hw_spi)", scanningModule);
  runScanningBenchmark("Scanning(dual,hw_spi,subfields)",
      scanningModuleSubfields);
  scanningModuleSubfields.end();
  scanningModule.end();
  ledMatrix.end();
  spiInterface.end();
}

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

// Tm1637Module(Normal)
void runTm1637DisplayNormal() {
  using Driver = Tm1637Driver;
  Driver driver(CLK_PIN, DIO_PIN, BIT_DELAY);
  Tm1637Module<Driver, NUM_DIGITS> tm1637Module(driver);

  driver.begin();
  tm1637Module.begin();
  runTm1637Benchmark("Tm1637(Normal)", tm1637Module);
  tm1637Module.end();
  driver.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
// Tm1637Module(Fast)
void runTm1637DisplayFast() {
  using Driver = Tm1637DriverFast<CLK_PIN, DIO_PIN, BIT_DELAY>;
  Driver driver;
  Tm1637Module<Driver, NUM_DIGITS> tm1637Module(driver);

  driver.begin();
  tm1637Module.begin();
  runTm1637Benchmark("Tm1637(Fast)", tm1637Module);
  tm1637Module.end();
  driver.end();
}
#endif

void runBenchmarks() {
  runDirect();
  runSingleShiftRegisterSwSpi();
  runSingleShiftRegisterHwSpi();
  runDualShiftRegisterSwSpi();
  runDualShiftRegisterHwSpi();

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runDirectFast();
  runSingleShiftRegisterSwSpiFast();
  runDualShiftRegisterSwSpiFast();
#endif

  runTm1637DisplayNormal();

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runTm1637DisplayFast();
#endif
}

void printSizeOf() {
  SERIAL_PORT_MONITOR.print(F("sizeof(SwSpiInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(SwSpiInterface));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(FastSwSpiInterface<1,2,3>): "));
  SERIAL_PORT_MONITOR.println(sizeof(FastSwSpiInterface<1,2,3>));
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(HwSpiInterface): "));
  SERIAL_PORT_MONITOR.println(sizeof(HwSpiInterface));

  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirect<>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirect<>));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirectFast<0..3, 0..7>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirectFast<
      2, 3, 4, 5,
      6, 7, 8, 9, 10, 11, 12, 13>));
#endif

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixSingleShiftRegister<SwSpiInterface>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(LedMatrixSingleShiftRegister<SwSpiInterface>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixDualShiftRegister<HwSpiInterface>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(LedMatrixDualShiftRegister<HwSpiInterface>));

  SERIAL_PORT_MONITOR.print(F("sizeof(LedDisplay): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedDisplay));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(ScanningModule<LedMatrixBase, 4, 1>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(ScanningModule<LedMatrixBase, 4, 1>));

  SERIAL_PORT_MONITOR.print(F("sizeof(Tm1637Module<Tm1637Driver, 4>): "));
  SERIAL_PORT_MONITOR.println(sizeof(Tm1637Module<Tm1637Driver, 4>));

  SERIAL_PORT_MONITOR.print(F("sizeof(NumberWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(NumberWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(ClockWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(ClockWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(CharWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(CharWriter));

  SERIAL_PORT_MONITOR.print(F("sizeof(StringWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(StringWriter));
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
