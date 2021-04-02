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
 * SegmentDisplay::renderField() for various configurations of LedMatrix. The
 * output is an space-separate list of numbers which can be fed into
 * `generate_table.awk` to extract a human-readable ASCII table that can be
 * pasted directly into the README.md file as a code block.
 *
 * Each DriverConfig configures the SegmentDisplay stack and all of its
 * dependencies. It calls SegmentDisplay::displayField() a number of times
 * (NUM_FIELD_SAMPLES is 1800), then retrieves the TimingStats from the
 * SegmentDisplay, and prints out the min/avg/max numbers.
 */

#include <stdio.h>
#include <Arduino.h>
#include <AceCommon.h> // TimingStats
#include <AceSegment.h>
#include <ace_segment/fast/LedMatrixDirectFast.h>
#include <ace_segment/fast/SwSpiAdapterFast.h>

using namespace ace_segment;
using ace_common::TimingStats;

#if ! defined(SERIAL_PORT_MONITOR)
#define SERIAL_PORT_MONITOR Serial
#endif

//------------------------------------------------------------------
// Setup for AceSegment
//------------------------------------------------------------------

const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 1;

const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;

#if defined(EPOXY_DUINO)
  // numbers don't matter
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {1, 2, 3, 4};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {5, 6, 7, 8, 9, 10, 11, 12};
#elif defined(ARDUINO_ARCH_AVR)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};
#elif defined(ARDUINO_ARCH_SAMD)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};
#elif defined(ARDUINO_ARCH_STM32)
  // I think this is the F1, because there exists a ARDUINO_ARCH_STM32F4
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};
#elif defined(ESP8266)
  // Don't have enough pins so reuse some.
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {D4, D5, D4, D5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {D6, D7, D6, D7, D6, D7, D6, D7};
#elif defined(ESP32)
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {21, 22, 23, 24};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {12, 13, 14, 15, 16, 17, 18, 19};
#elif defined(TEENSYDUINO)
  // Teensy 3.2
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};
#else
  #warning Unknown hardware, using defaults which may interfere with Serial
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {6, 7, 8, 9, 10, 11, 12, 13};
#endif

const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = MOSI; // DS on 74HC595
const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595

Hardware hardware;

//------------------------------------------------------------------
// Run benchmarks.
//------------------------------------------------------------------

// Each frame has NUM_DIGITS * NUM_SUBFIELDS fields. At 60 frames/second and 4
// digits, and NUM_FIELD_SAMPLES=1, we want at least 240 samples. Let's grab
// about 1/2 second worth.
const uint16_t NUM_FIELD_SAMPLES = 1200;

/** Print the result for each LedMatrix algorithm. */
static void printStats(const char* name, TimingStats& stats) {
  SERIAL_PORT_MONITOR.print(name);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMin());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getAvg());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMax());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.println(NUM_FIELD_SAMPLES);
}

TimingStats timingStats;

template <typename SD>
void runBenchmark(const char* name, SD& segmentDisplay) {

  segmentDisplay.writePatternAt(0, 0x13);
  segmentDisplay.writePatternAt(0, 0x37);
  segmentDisplay.writePatternAt(0, 0x7F);
  segmentDisplay.writePatternAt(0, 0xFF);

  for (uint16_t i = 0; i < NUM_FIELD_SAMPLES; i++) {
    segmentDisplay.renderField();
    yield();
  }

  printStats(name, timingStats);
  timingStats.reset();
}

// Common Anode, with transistors on Group pins
void runDirect() {
  using LedMatrix = LedMatrixDirect<Hardware>;
  LedMatrix ledMatrix(
      hardware,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS,
      NUM_SEGMENTS,
      SEGMENT_PINS);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("direct", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
}

#if defined(ARDUINO_ARCH_AVR)
// Common Anode, with transistors on Group pins
void runDirectFast() {
  using LedMatrix = LedMatrixDirectFast<
      4, 5, 6, 7,
      8, 9, 10, 16, 14, 18, 19, 15>;
  LedMatrix ledMatrix(
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("direct_fast", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
}
#endif

// Common Cathode, with transistors on Group pins
void runSingleShiftRegisterSwSpi() {
  SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  spiAdapter.begin();
  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("single_sw_spi", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
  spiAdapter.end();
}

// Common Cathode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR)
void runSingleShiftRegisterSwSpiFast() {
  using SpiAdapter = SwSpiAdapterFast<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiAdapter spiAdapter;
  using LedMatrix = LedMatrixSingleShiftRegister<Hardware, SpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  spiAdapter.begin();
  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("single_sw_spi_fast", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
  spiAdapter.end();
}
#endif

// Common Cathode, with transistors on Group pins
void runSingleShiftRegisterHwSpi() {
  HwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleShiftRegister<Hardware, HwSpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  spiAdapter.begin();
  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("single_hw_spi", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
  spiAdapter.end();
}

// Common Anode, with transistors on Group pins
void runDualShiftRegisterSwSpi() {
  SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualShiftRegister<SwSpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  spiAdapter.begin();
  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("dual_sw_spi", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
  spiAdapter.end();
}

// Common Anode, with transistors on Group pins
#if defined(ARDUINO_ARCH_AVR)
void runDualShiftRegisterSwSpiFast() {
  using SpiAdapter = SwSpiAdapterFast<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiAdapter spiAdapter;
  using LedMatrix = LedMatrixDualShiftRegister<SpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  spiAdapter.begin();
  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("dual_sw_spi_fast", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
  spiAdapter.end();
}
#endif

// Common Anode, with transistors on Group pins
void runDualShiftRegisterHwSpi() {
  HwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualShiftRegister<HwSpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
      hardware, ledMatrix, FRAMES_PER_SECOND, &timingStats);

  spiAdapter.begin();
  ledMatrix.begin();
  segmentDisplay.begin();
  runBenchmark("dual_hw_spi", segmentDisplay);
  segmentDisplay.end();
  ledMatrix.end();
  spiAdapter.end();
}

void runBenchmarks() {
  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));

  runDirect();
  runSingleShiftRegisterSwSpi();
  runSingleShiftRegisterHwSpi();
  runDualShiftRegisterSwSpi();
  runDualShiftRegisterHwSpi();

#if defined(ARDUINO_ARCH_AVR)
  runDirectFast();
  runSingleShiftRegisterSwSpiFast();
  runDualShiftRegisterSwSpiFast();
#endif
}

void printSizeOf() {
  SERIAL_PORT_MONITOR.println(F("SIZEOF"));

  SERIAL_PORT_MONITOR.print(F("sizeof(Hardware): "));
  SERIAL_PORT_MONITOR.println(sizeof(Hardware));

  SERIAL_PORT_MONITOR.print(F("sizeof(SwSpiAdapter): "));
  SERIAL_PORT_MONITOR.println(sizeof(SwSpiAdapter));

#if defined(ARDUINO_ARCH_AVR)
  SERIAL_PORT_MONITOR.print(F("sizeof(SwSpiAdapterFast<1,2,3>): "));
  SERIAL_PORT_MONITOR.println(sizeof(SwSpiAdapterFast<1,2,3>));
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(HwSpiAdapter): "));
  SERIAL_PORT_MONITOR.println(sizeof(HwSpiAdapter));

  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirect<Hardware>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirect<Hardware>));

#if defined(ARDUINO_ARCH_AVR)
  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirectFast<0..3, 0..7>): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirectFast<
      2, 3, 4, 5,
      6, 7, 8, 9, 10, 11, 12, 13>));
#endif

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(LedMatrixDualShiftRegister<HwSpiAdapter>));

  SERIAL_PORT_MONITOR.print(F("sizeof(LedDisplay): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedDisplay));

  SERIAL_PORT_MONITOR.print(
      F("sizeof(SegmentDisplay<Hardware, LedMatrixBase, 4, 1>): "));
  SERIAL_PORT_MONITOR.println(
      sizeof(SegmentDisplay<Hardware, LedMatrixBase, 4, 1>));

  SERIAL_PORT_MONITOR.print(F("sizeof(HexWriter): "));
  SERIAL_PORT_MONITOR.println(sizeof(HexWriter));

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

  printSizeOf();
  runBenchmarks();
  SERIAL_PORT_MONITOR.println("END");

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}
