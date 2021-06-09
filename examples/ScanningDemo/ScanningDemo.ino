/*
 * A demo of various LED modules using the ScanningModule and LedMatrixXxx
 * classes. Supports the following environments:
 *
 *  * env:micro_scanning_direct
 *  * env:micro_scanning_single
 *  * env:micro_scanning_dual
 */

#include <Arduino.h>
#include <SPI.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // ScanningModule, LedDisplay

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  #include <digitalWriteFast.h>
  #include <ace_segment/hw/HardSpiFastInterface.h>
  #include <ace_segment/hw/SoftSpiFastInterface.h>
  #include <ace_segment/scanning/LedMatrixDirectFast4.h>
  using ace_segment::HardSpiFastInterface;
  using ace_segment::SoftSpiFastInterface;
#endif

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_segment::LedMatrixDirect;
using ace_segment::LedMatrixDirectFast4;
using ace_segment::LedMatrixSingleHc595;
using ace_segment::LedMatrixDualHc595;
using ace_segment::ScanningModule;
using ace_segment::LedDisplay;
using ace_segment::kByteOrderDigitHighSegmentLow;
using ace_segment::kActiveLowPattern;
using ace_segment::kActiveHighPattern;

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// LedMatrix wiring modes.
#define LED_MATRIX_MODE_NONE 0
#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_DIRECT_FAST 2
#define LED_MATRIX_MODE_SINGLE_SOFT_SPI 3
#define LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST 4
#define LED_MATRIX_MODE_SINGLE_HARD_SPI 5
#define LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST 6
#define LED_MATRIX_MODE_DUAL_SOFT_SPI 7
#define LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST 8
#define LED_MATRIX_MODE_DUAL_HARD_SPI 9
#define LED_MATRIX_MODE_DUAL_HARD_SPI_FAST 10

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_SCANNING_DIRECT
#endif

#if defined(EPOXY_DUINO)
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

  // LED segment patterns.
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_SCANNING_DIRECT)
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

  // LED segment patterns.
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_SCANNING_SINGLE)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_SCANNING_DUAL)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveLowPattern;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#else
  #error Unknown environment
#endif

// Total fields/second
//    = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//    = 60 * 16 * 4
//    = 3840 fields/sec
//    => 260 micros/field
//
// Fortunately, according to AutoBenchmark, the "fast" versions of LedMatrix can
// render a single field in about 20-30 micros.
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 7, 3, 2
};

#if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
  // Common Anode, with transitors on Group pins
  using LedMatrix = LedMatrixDirect<>;
  LedMatrix ledMatrix(
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/,
      NUM_SEGMENTS,
      SEGMENT_PINS,
      NUM_DIGITS,
      DIGIT_PINS);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT_FAST
  // Common Anode, with transitors on Group pins
  using LedMatrix = LedMatrixDirectFast4<
    8, 9, 10, 16, 14, 18, 19, 15,
    4, 5, 6, 7
  >;
  LedMatrix ledMatrix(
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI
  // Common Cathode, with transistors on Group pins
  SoftSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleHc595<SoftSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveHighPattern /*elementOnPattern*/,
      kActiveHighPattern /*groupOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS):

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST
  // Common Cathode, with transistors on Group pins
  using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
  using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveHighPattern /*elementOnPattern*/,
      kActiveHighPattern /*groupOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI
  // Common Cathode, with transistors on Group pins
  SPIClass& spiInstance = SPI;
  HardSpiInterface<SPIClass> spiInterface(
      spiInstance, LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleHc595<HardSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveHighPattern /*elementOnPattern*/,
      kActiveHighPattern /*groupOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST
  // Common Cathode, with transistors on Group pins
  SPIClass& spiInstance = SPI;
  using SpiInterface = HardSpiFastInterface<
      SPIClass, LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface(spiInstance);
  using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveHighPattern /*elementOnPattern*/,
      kActiveHighPattern /*groupOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI
  // Common Anode, with transistors on Group pins
  SoftSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualHc595<SoftSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/,
      HC595_BYTE_ORDER);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
  // Common Anode, with transistors on Group pins
  using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
  using LedMatrix = LedMatrixDualHc595<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/,
      HC595_BYTE_ORDER);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI
  // Common Anode, with transistors on Group pins
  SPIClass& spiInstance = SPI;
  HardSpiInterface<SPIClass> spiInterface(
      spiInstance, LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualHc595<HardSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/,
      HC595_BYTE_ORDER);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
  // Common Anode, with transistors on Group pins
  SPIClass& spiInstance = SPI;
  using SpiInterface = HardSpiFastInterface<
      SPIClass, LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface(spiInstance);
  using LedMatrix = LedMatrixDualHc595<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/,
      HC595_BYTE_ORDER);

#else
  #error Unsupported LED_MATRIX_MODE
#endif

// NUM_SUBFIELDS levels of brightness.
ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
    ledModule(ledMatrix, FRAMES_PER_SECOND);
LedDisplay display(ledModule);

// LedDisplay patterns
const uint8_t PATTERNS[NUM_DIGITS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
};

void setupAceSegment() {
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
    spiInstance.begin();
  #endif

  #if LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
    spiInterface.begin();
  #endif

  ledMatrix.begin();
  ledModule.begin();
}

//----------------------------------------------------------------------------

// loop() state variables
TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightnessIndex = 0;

void updateDisplay() {
  static uint16_t prevUpdateMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevUpdateMillis) >= 1000) {
    prevUpdateMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      display.writePatternAt(i, PATTERNS[j]);
      // Write a decimal point every other digit, for demo purposes.
      display.writeDecimalPointAt(i, j & 0x1);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness
    uint8_t brightness = BRIGHTNESS_LEVELS[brightnessIndex];
    display.setBrightness(brightness);
    incrementMod(brightnessIndex, NUM_BRIGHTNESSES);
  }
}

// Call renderFieldWhenReady() as fast as possible. It uses an internal timer to
// do the actual rendering when ready. Limit timing samples to every 10 ms to
// limit number of samples over 5 seconds to less than UINT16_MAX (i.e. 65535).
void flushModule() {
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevSampleMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevSampleMillis) >= 100) {
    prevSampleMillis = nowMillis;

    uint16_t startMicros = micros();
    ledModule.renderFieldWhenReady();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
  } else {
    ledModule.renderFieldWhenReady();
  }
#else
    ledModule.renderFieldWhenReady();
#endif
}

// Every 5 seconds, print stats about how long flush() took.
void printStats() {
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevStatsMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevStatsMillis) >= 5000) {
    prevStatsMillis = nowMillis;

    Serial.print("min/avg/max:");
    Serial.print(stats.getMin());
    Serial.print('/');
    Serial.print(stats.getAvg());
    Serial.print('/');
    Serial.println(stats.getMax());
    stats.reset();
  }
#endif
}

//----------------------------------------------------------------------------

void setup() {
  delay(1000);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  setupAceSegment();
}

void loop() {
  updateDisplay();
  flushModule();
  printStats();
}
