/*
 * A program which compiles in different ScanningDisplay objects configured with
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
#define FEATURE_SINGLE_SW_SPI 2
#define FEATURE_SINGLE_HW_SPI 3
#define FEATURE_DUAL_SW_SPI 4
#define FEATURE_DUAL_HW_SPI 5
#define FEATURE_DIRECT_FAST 6
#define FEATURE_SINGLE_SW_SPI_FAST 7
#define FEATURE_DUAL_SW_SPI_FAST 8

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE > FEATURE_BASELINE
  #include <AceSegment.h>
  #include <ace_segment/fast/LedMatrixDirectFast.h>
  #include <ace_segment/fast/SwSpiAdapterFast.h>
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
    using LedMatrix = LedMatrixDirect<Hardware>;
    LedMatrix ledMatrix(
        hardware,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/,
        NUM_DIGITS,
        digitPins,
        NUM_SEGMENTS,
        segmentPins);
  #elif FEATURE == FEATURE_SINGLE_SW_SPI
    // Common Cathode, with transistors on Group pins
    SwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    using LedMatrix = LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>;
    LedMatrix ledMatrix(
        hardware,
        spiAdapter,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        NUM_DIGITS,
        digitPins);
  #elif FEATURE == FEATURE_SINGLE_SW_SPI_FAST
    // Common Cathode, with transistors on Group pins
    using SpiAdapter = SwSpiAdapterFast<latchPin, dataPin, clockPin>;
    SpiAdapter spiAdapter;
    using LedMatrix = LedMatrixSingleShiftRegister<Hardware, SpiAdapter>;
    LedMatrix ledMatrix(
        hardware,
        spiAdapter,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        NUM_DIGITS,
        digitPins);
  #elif FEATURE == FEATURE_SINGLE_HW_SPI
    // Common Cathode, with transistors on Group pins
    HwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    using LedMatrix = LedMatrixSingleShiftRegister<Hardware, HwSpiAdapter>;
    LedMatrix ledMatrix(
        hardware,
        spiAdapter,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        NUM_DIGITS,
        digitPins);
  #elif FEATURE == FEATURE_DUAL_SW_SPI
    // Common Cathode, with transistors on Group pins
    SwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    using LedMatrix = LedMatrixDualShiftRegister<SwSpiAdapter>;
    LedMatrix ledMatrix(
        spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  #elif FEATURE == FEATURE_DUAL_SW_SPI_FAST
    // Common Cathode, with transistors on Group pins
    using SpiAdapter = SwSpiAdapterFast<latchPin, dataPin, clockPin>;
    SpiAdapter spiAdapter;
    using LedMatrix = LedMatrixDualShiftRegister<SpiAdapter>;
    LedMatrix ledMatrix(
        spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  #elif FEATURE == FEATURE_DUAL_HW_SPI
    // Common Cathode, with transistors on Group pins
    HwSpiAdapter spiAdapter(latchPin, dataPin, clockPin);
    using LedMatrix = LedMatrixDualShiftRegister<HwSpiAdapter>;
    LedMatrix ledMatrix(
        spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  #elif FEATURE == FEATURE_DIRECT_FAST
    // Common Anode, with transitions on Group pins
    using LedMatrix = LedMatrixDirectFast<
      4, 5, 6, 7,
      8, 9, 10, 16, 14, 18, 19, 15
    >;
    LedMatrix ledMatrix(
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
  #endif

  ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);
#endif

void setup() {
#if FEATURE == FEATURE_BASELINE
  disableCompilerOptimization = 3;

// In the following, I used to grab the output of patterns[] and write to
// disableCompilerOptimization to prevent the compiler from optimizing away the
// entire program. But after templating ScanningDisplay, pattterns is no longer
// accessible. But it does not matter because I realized that ScanningDisplay
// performs a digitalWrite(), which has the same effect of disabling
// optimizations.

#elif FEATURE == FEATURE_DIRECT
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#elif FEATURE == FEATURE_DIRECT_FAST
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#elif FEATURE == FEATURE_SINGLE_SW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#elif FEATURE == FEATURE_SINGLE_SW_SPI_FAST
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#elif FEATURE == FEATURE_SINGLE_HW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#elif FEATURE == FEATURE_DUAL_SW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#elif FEATURE == FEATURE_DUAL_SW_SPI_FAST
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#elif FEATURE == FEATURE_DUAL_HW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldNow();

#else
  #error Unknown FEATURE
#endif

}

void loop() {
#if FEATURE > FEATURE_BASELINE
  scanningDisplay.renderFieldWhenReady();
#endif
}
