/*
 * A program which compiles in different SegmentDisplay objects configured with
 * using different LED configurations to determine the flash and static memory
 * sizes from the output of the compiler. Set the FEATURE macro to various
 * integer to compile different algorithms.
 */

#include <Arduino.h>

// DO NOT MODIFY THIS LINE. This will be overwritten by collect.sh on each
// iteration, incrementing from 0 to 5. The Arduino IDE will compile the
// program, then the script will extract the flash and static memory usage
// numbers printed out by the Arduino compiler. The numbers will be printed on
// the STDOUT, which then can be saved to a file specific for a particular
// hardware platform, e.g. "nano.txt" or "esp8266.txt".
#define FEATURE 0

// List of features of AceSegment that we want to gather memory usage numbers.
#define FEATURE_BASELINE 0
#define FEATURE_DIRECT 1
#define FEATURE_SPLIT_SW_SPI 2
#define FEATURE_SPLIT_HW_SPI 3
#define FEATURE_MERGED_SW_SPI 4
#define FEATURE_MERGED_HW_SPI 5

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE > FEATURE_BASELINE
  #include <AceSegment.h>
  using namespace ace_segment;

  // Common to all FEATURES
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t FRAMES_PER_SECOND = 60;
  const uint8_t NUM_SUBFIELDS = 1;
  const bool COMMON_CATHODE = true;
  const bool USE_TRANSISTORS = true;
  const uint8_t latchPin = 10; // ST_CP on 74HC595
  const uint8_t dataPin = MOSI; // DS on 74HC595
  const uint8_t clockPin = SCK; // SH_CP on 74HC595
  const uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t segmentPins[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

  Hardware hardware;

  #if FEATURE == FEATURE_DIRECT
    // Common Anode, with transitions on Group pins
    LedMatrixDirect ledMatrix(
        &hardware,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/,
        NUM_DIGITS,
        digitPins,
        NUM_SEGMENTS,
        segmentPins);
  #elif FEATURE == FEATURE_SPLIT_SW_SPI
    // Common Cathode, with transistors on Group pins
    SwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    LedMatrixPartialSpi ledMatrix(
        &hardware,
        &spiAdapter,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        NUM_DIGITS,
        digitPins);
  #elif FEATURE == FEATURE_SPLIT_HW_SPI
    // Common Cathode, with transistors on Group pins
    HwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    LedMatrixPartialSpi ledMatrix(
        &hardware,
        &spiAdapter,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        NUM_DIGITS,
        digitPins);
  #elif FEATURE == FEATURE_MERGED_SW_SPI
    // Common Cathode, with transistors on Group pins
    SwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    LedMatrixFullSpi ledMatrix(
        &spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  #elif FEATURE == FEATURE_MERGED_HW_SPI
    // Common Cathode, with transistors on Group pins
    HwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    LedMatrixFullSpi ledMatrix(
        &spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  #endif

  uint8_t patterns[NUM_DIGITS];
  SegmentDisplay segmentDisplay(
      &hardware,
      &ledMatrix,
      FRAMES_PER_SECOND,
      NUM_DIGITS,
      patterns);
#endif

void setup() {
#if FEATURE == FEATURE_BASELINE
  disableCompilerOptimization = 3;

#elif FEATURE == FEATURE_DIRECT
  ledMatrix.begin();
  segmentDisplay.begin();
  segmentDisplay.writePatternAt(0, 0x3A);
  disableCompilerOptimization = patterns[1];

#elif FEATURE == FEATURE_SPLIT_SW_SPI
  spiAdapter.spiBegin();
  ledMatrix.begin();
  segmentDisplay.begin();
  segmentDisplay.writePatternAt(0, 0x3A);
  disableCompilerOptimization = patterns[1];

#elif FEATURE == FEATURE_SPLIT_HW_SPI
  spiAdapter.spiBegin();
  ledMatrix.begin();
  segmentDisplay.begin();
  segmentDisplay.writePatternAt(0, 0x3A);
  disableCompilerOptimization = patterns[1];

#elif FEATURE == FEATURE_MERGED_SW_SPI
  spiAdapter.spiBegin();
  ledMatrix.begin();
  segmentDisplay.begin();
  segmentDisplay.writePatternAt(0, 0x3A);
  disableCompilerOptimization = patterns[1];

#elif FEATURE == FEATURE_MERGED_HW_SPI
  spiAdapter.spiBegin();
  ledMatrix.begin();
  segmentDisplay.begin();
  segmentDisplay.writePatternAt(0, 0x3A);
  disableCompilerOptimization = patterns[1];

#else
  #error Unknown FEATURE
#endif

}

void loop() {
#if FEATURE > FEATURE_BASELINE
  segmentDisplay.renderFieldWhenReady();
#endif
}
