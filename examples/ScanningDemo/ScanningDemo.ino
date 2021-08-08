/*
 * A demo of low-level ScanningModule and LedMatrixXxx classes. This is an
 * advanced demo, most users of the AceSegment library will want to use
 * DirectModule, HybridModule, and Hc595Module classes instead. Displays the
 * digits 0 to 3, then slowly rotates the digits to the left, while incrementing
 * the brightness of the entire LED module.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_CUSTOM_DIRECT: Pro Micro + custom direct LED module
 *  * AUNITER_MICRO_CUSTOM_SINGLE: Pro Micro + custom single 74HC595 module
 *  * AUNITER_MICRO_CUSTOM_DUAL: Pro Micro + custom dual 74HC595 module
 */

#include <Arduino.h>
#include <SPI.h>
#include <AceCommon.h> // incrementMod()
#include <AceSPI.h>
#include <AceSegment.h> // ScanningModule

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  #include <digitalWriteFast.h>
  #include <ace_spi/HardSpiFastInterface.h>
  #include <ace_spi/SimpleSpiFastInterface.h>
  #include <ace_segment/scanning/LedMatrixDirectFast4.h>
  using ace_spi::HardSpiFastInterface;
  using ace_spi::SimpleSpiFastInterface;
#endif

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_spi::SimpleSpiInterface;
using ace_spi::HardSpiInterface;
using ace_segment::LedModule;
using ace_segment::LedMatrixDirect;
using ace_segment::LedMatrixDirectFast4;
using ace_segment::LedMatrixSingleHc595;
using ace_segment::LedMatrixDualHc595;
using ace_segment::ScanningModule;
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
#define LED_MATRIX_MODE_SINGLE_SIMPLE_SPI 3
#define LED_MATRIX_MODE_SINGLE_SIMPLE_SPI_FAST 4
#define LED_MATRIX_MODE_SINGLE_HARD_SPI 5
#define LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST 6
#define LED_MATRIX_MODE_DUAL_SIMPLE_SPI 7
#define LED_MATRIX_MODE_DUAL_SIMPLE_SPI_FAST 8
#define LED_MATRIX_MODE_DUAL_HARD_SPI 9
#define LED_MATRIX_MODE_DUAL_HARD_SPI_FAST 10

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_CUSTOM_DIRECT
#endif

#if defined(EPOXY_DUINO)
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

  // LED segment patterns.
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_CUSTOM_DIRECT)
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

  // LED segment patterns.
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_CUSTOM_SINGLE)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SIMPLE_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SIMPLE_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_CUSTOM_DUAL)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveLowPattern;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SIMPLE_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SIMPLE_SPI_FAST
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

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SIMPLE_SPI
  // Common Cathode, with transistors on Group pins
  SimpleSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleHc595<SimpleSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveHighPattern /*elementOnPattern*/,
      kActiveHighPattern /*groupOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SIMPLE_SPI_FAST
  // Common Cathode, with transistors on Group pins
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
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
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(spiInstance, LATCH_PIN);
  using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveHighPattern /*elementOnPattern*/,
      kActiveHighPattern /*groupOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST
  // Common Cathode, with transistors on Group pins
  SPIClass& spiInstance = SPI;
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
  SpiInterface spiInterface(spiInstance);
  using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveHighPattern /*elementOnPattern*/,
      kActiveHighPattern /*groupOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SIMPLE_SPI
  // Common Anode, with transistors on Group pins
  SimpleSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualHc595<SimpleSpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/,
      HC595_BYTE_ORDER);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SIMPLE_SPI_FAST
  // Common Anode, with transistors on Group pins
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
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
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(spiInstance, LATCH_PIN);
  using LedMatrix = LedMatrixDualHc595<SpiInterface>;
  LedMatrix ledMatrix(
      spiInterface,
      kActiveLowPattern /*elementOnPattern*/,
      kActiveLowPattern /*groupOnPattern*/,
      HC595_BYTE_ORDER);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
  // Common Anode, with transistors on Group pins
  SPIClass& spiInstance = SPI;
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
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

// LED patterns
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

  #if LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SIMPLE_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SIMPLE_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SIMPLE_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SIMPLE_SPI_FAST \
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

// Update the LedModule with new pattern and brightness every second.
void updateDisplay() {
  static uint16_t prevUpdateMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevUpdateMillis) >= 1000) {
    prevUpdateMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      // Write a decimal point every other digit, for demo purposes.
      uint8_t pattern = PATTERNS[j] | ((j & 0x1) ? 0x80 : 0x00);
      ledModule.setPatternAt(i, pattern);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness
    uint8_t brightness = BRIGHTNESS_LEVELS[brightnessIndex];
    ledModule.setBrightness(brightness);
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
