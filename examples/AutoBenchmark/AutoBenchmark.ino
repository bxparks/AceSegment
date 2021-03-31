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

#if defined(ARDUINO_ARCH_AVR)
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
  #warning Unknown hardware, using some defaults
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {2, 3, 4, 5};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {2, 3, 4, 5, 6, 7, 8, 9};
#endif

const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
const uint8_t DATA_PIN = MOSI; // DS on 74HC595
const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595

Hardware hardware;

// Common Anode, with transistors on Group pins
LedMatrixDirect ledMatrixDirect(
    &hardware,
    LedMatrix::kActiveLowPattern /*groupOnPattern*/,
    LedMatrix::kActiveLowPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS,
    NUM_SEGMENTS,
    SEGMENT_PINS);

// Common Cathode, with transistors on Group pins
SwSpiAdapter swSpiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
LedMatrixPartialSpi ledMatrixPartialSwSpi(
    &hardware,
    &swSpiAdapter,
    LedMatrix::kActiveHighPattern /*groupOnPattern*/,
    LedMatrix::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS);

// Common Cathode, with transistors on Group pins
HwSpiAdapter hwSpiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
LedMatrixPartialSpi ledMatrixPartialHwSpi(
    &hardware,
    &hwSpiAdapter,
    LedMatrix::kActiveHighPattern /*groupOnPattern*/,
    LedMatrix::kActiveHighPattern /*elementOnPattern*/,
    NUM_DIGITS,
    DIGIT_PINS);

// Common Anode, with transistors on Group pins
LedMatrixFullSpi ledMatrixFullSwSpi(
    &swSpiAdapter,
    LedMatrix::kActiveLowPattern /*groupOnPattern*/,
    LedMatrix::kActiveLowPattern /*elementOnPattern*/);

// Common Anode, with transistors on Group pins
LedMatrixFullSpi ledMatrixFullHwSpi(
    &hwSpiAdapter,
    LedMatrix::kActiveLowPattern /*groupOnPattern*/,
    LedMatrix::kActiveLowPattern /*elementOnPattern*/);

uint8_t patterns[NUM_DIGITS];

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

void runBenchmark(const char* name, LedMatrix& ledMatrix) {
  TimingStats timingStats;
  ledMatrix.begin();

  Renderer renderer(&ledMatrix, NUM_DIGITS, patterns);
  renderer.begin();

  SegmentDisplay segmentDisplay(
      &hardware,
      &renderer,
      FRAMES_PER_SECOND,
      NUM_DIGITS,
      patterns,
      nullptr /*brightnesses*/,
      &timingStats);
  segmentDisplay.begin();
  segmentDisplay.writePatternAt(0, 0x13);
  segmentDisplay.writePatternAt(0, 0x37);
  segmentDisplay.writePatternAt(0, 0x7F);
  segmentDisplay.writePatternAt(0, 0xFF);

  for (uint16_t i = 0; i < NUM_FIELD_SAMPLES; i++) {
    segmentDisplay.renderField();
    yield();
  }

  segmentDisplay.end();
  renderer.end();
  ledMatrix.end();

  printStats(name, timingStats);
  timingStats.reset();
}

void runBenchmarks() {
  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));

  runBenchmark("direct", ledMatrixDirect);
  runBenchmark("partial_sw_spi", ledMatrixPartialSwSpi);
  runBenchmark("partial_hw_spi", ledMatrixPartialHwSpi);
  runBenchmark("full_sw_spi", ledMatrixFullSwSpi);
  runBenchmark("full_hw_spi", ledMatrixFullHwSpi);
}

void printSizeOf() {
  SERIAL_PORT_MONITOR.println(F("SIZEOF"));

  SERIAL_PORT_MONITOR.print(F("sizeof(Hardware): "));
  SERIAL_PORT_MONITOR.println(sizeof(Hardware));
  SERIAL_PORT_MONITOR.print(F("sizeof(SwSpiAdapter): "));
  SERIAL_PORT_MONITOR.println(sizeof(SwSpiAdapter));
  SERIAL_PORT_MONITOR.print(F("sizeof(HwSpiAdapter): "));
  SERIAL_PORT_MONITOR.println(sizeof(HwSpiAdapter));
  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixDirect): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixDirect));
  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixPartialSpi): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixPartialSpi));
  SERIAL_PORT_MONITOR.print(F("sizeof(LedMatrixFullSpi): "));
  SERIAL_PORT_MONITOR.println(sizeof(LedMatrixFullSpi));
  SERIAL_PORT_MONITOR.print(F("sizeof(Renderer): "));
  SERIAL_PORT_MONITOR.println(sizeof(Renderer));
  SERIAL_PORT_MONITOR.print(F("sizeof(SegmentDisplay): "));
  SERIAL_PORT_MONITOR.println(sizeof(SegmentDisplay));
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
