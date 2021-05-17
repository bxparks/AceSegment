/*
 * A demo of a 4-digit LED module with the segment pins connected to a 74HC595
 * shift register and the digit pins also connected to a 74HC595 shift regstier.
 * Uses the Hc595Module class.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // Hc595Module, LedDisplay

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_segment/hw/SoftSpiFastInterface.h>
#include <ace_segment/hw/HardSpiFastInterface.h>
#include <ace_segment/hw/SoftWireFastInterface.h>
using ace_segment::SoftSpiFastInterface;
using ace_segment::HardSpiFastInterface;
#endif

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using namespace ace_segment;

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// Type of LED Module
#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1
#define LED_DISPLAY_TYPE_MAX7219 2
#define LED_DISPLAY_TYPE_HC595 3
#define LED_DISPLAY_TYPE_DIRECT 4
#define LED_DISPLAY_TYPE_HYBRID 5
#define LED_DISPLAY_TYPE_FULL 6

// Used by LED_DISPLAY_TYPE_PARTIAL and LED_DISPLAY_TYPE_FULL
#define INTERFACE_TYPE_SOFT_SPI 0
#define INTERFACE_TYPE_SOFT_SPI_FAST 1
#define INTERFACE_TYPE_HARD_SPI 2
#define INTERFACE_TYPE_HARD_SPI_FAST 3
#define INTERFACE_TYPE_SOFT_WIRE 4
#define INTERFACE_TYPE_SOFT_WIRE_FAST 5

#if defined(EPOXY_DUINO)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  const uint8_t NUM_DIGITS = 4;
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  const uint8_t SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t* const REMAP_ARRAY = nullptr;

#elif defined(AUNITER_MICRO_CUSTOM_DUAL)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  #define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  const uint8_t NUM_DIGITS = 4;
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  const uint8_t SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t* const REMAP_ARRAY = nullptr;

#elif defined(AUNITER_MICRO_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t NUM_DIGITS = 8;
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  const uint8_t SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = LedMatrixBase::kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

#elif defined(AUNITER_STM32_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  const uint8_t NUM_DIGITS = 8;
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  const uint8_t SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = LedMatrixBase::kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

#elif defined(AUNITER_D1MINI_LARGE_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  const uint8_t NUM_DIGITS = 8;
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  const uint8_t SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = LedMatrixBase::kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

#else
  #error Unknown environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

// LED segment patterns.
const uint8_t NUM_SEGMENTS = 8;

// Total fields/second
//    = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//    = 60 * 16 * 4
//    = 3840 fields/sec
//    => 260 micros/field
//
// Fortunately, according to AutoBenchmark, the "fast" versions of LedMatrix can
// render a single field in about 20-30 micros.
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 1;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 9, 5, 2
};

// Common Cathode, with transistors on Group pins
#if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI
  using SpiInterface = SoftSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI_FAST
  using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
#elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
  using SpiInterface = HardSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
  using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
#else
  #error Unknown INTERFACE_TYPE
#endif
Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
    spiInterface,
    SEGMENT_ON_PATTERN,
    DIGIT_ON_PATTERN,
    FRAMES_PER_SECOND,
    HC595_BYTE_ORDER,
    REMAP_ARRAY
);
LedDisplay display(ledModule);

// LedDisplay patterns
const uint8_t PATTERNS[8] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
};

void setupAceSegment() {
  spiInterface.begin();
  ledModule.begin();
}

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightnessIndex = 0;

// Update the display with new pattern and brightness every second.
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

// Call renderFieldWhenReady() as fast as possible. It keeps an internal timer
// that performs the actual rendering when ready.
void flushModule() {
  uint16_t startMicros = micros();
  ledModule.renderFieldWhenReady();
  uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
  stats.update(elapsedMicros);
}

// Every 5 seconds, print stats about how long flushModule() took.
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
