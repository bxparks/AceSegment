/*
 * A demo of the 8-digit LED module from diymore.cc/robotdyn.com, or a custom
 * LED module with the segment pins and digit pins connected to two 74HC595
 * shift register chips. Displays the digits 0 to 3 or 0 to 7, then slowly
 * rotates the digits to the left, while incrementing the brightness of the
 * display. Uses the Hc595Module class.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_HC595: SparkFun Pro Micro + diymore.cc LED module
 *  * AUNITER_MICRO_CUSTOM_DUAL: SparkFun Pro Micro + Custom LED module
 *  * AUNITER_SAMD_HC595: SAMD21 M0 Mini + diymore.cc LED module
 *  * AUNITER_STM32_HC595: STM32 F1 Blue Pill
 *  * AUNITER_D1MINI_LARGE_HC595: WeMos D1 Mini ESP8266
 *  * AUNITER_ESP32_HC595: ESP32 Dev Kit v1
 */

#include <Arduino.h>
#include <SPI.h>
#include <AceCommon.h> // incrementMod()
#include <AceSPI.h>
#include <AceSegment.h> // Hc595Module

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_spi::HardSpiInterface;
using ace_spi::SimpleSpiInterface;
using ace_segment::LedModule;
using ace_segment::Hc595Module;
using ace_segment::kDigitRemapArray8Hc595;
using ace_segment::kByteOrderDigitHighSegmentLow;
using ace_segment::kByteOrderSegmentHighDigitLow;
using ace_segment::kActiveLowPattern;
using ace_segment::kActiveHighPattern;

// Select interface protocol.
#define INTERFACE_TYPE_SIMPLE_SPI 0
#define INTERFACE_TYPE_SIMPLE_SPI_FAST 1
#define INTERFACE_TYPE_HARD_SPI 2
#define INTERFACE_TYPE_HARD_SPI_FAST 3
#define INTERFACE_TYPE_SIMPLE_TMI 4
#define INTERFACE_TYPE_SIMPLE_TMI_FAST 5

//----------------------------------------------------------------------------
// Hardware environment configuration.
//----------------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_HC595
#endif

#if defined(EPOXY_DUINO)
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveLowPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t* const REMAP_ARRAY = nullptr;

  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_HC595)
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_CUSTOM_DUAL)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveLowPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t* const REMAP_ARRAY = nullptr;

  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_SAMD_HC595)
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_STM32_HC595)
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI

  // This dev board uses the default SPI1 pins.
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

  // These are the secondary SPI2 pins for reference.
  // const uint8_t LATCH_PIN = PB12;
  // const uint8_t DATA_PIN = PB15;
  // const uint8_t CLOCK_PIN = PB13;
  // SPIClass spiInstance(DATA_PIN, PB14 /*miso*/, CLOCK_PIN);

#elif defined(AUNITER_D1MINI_LARGE_HC595)
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_ESP32_HC595)
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI

  // This dev board uses the secondary HSPI pins.
  const uint8_t LATCH_PIN = 15;
  const uint8_t DATA_PIN = 13;
  const uint8_t CLOCK_PIN = 14;
  SPIClass spiInstance(HSPI);

  // These are the default primary VSPI pins for reference.
  // const uint8_t LATCH_PIN = SS;
  // const uint8_t DATA_PIN = MOSI;
  // const uint8_t CLOCK_PIN = SCK;
  // SPIClass& spiInstance = SPI;

#else
  #error Unknown environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

#if INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST \
    || INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
  #include <digitalWriteFast.h>
  #include <ace_spi/SimpleSpiFastInterface.h>
  #include <ace_spi/HardSpiFastInterface.h>
  using ace_spi::SimpleSpiFastInterface;
  using ace_spi::HardSpiFastInterface;
#endif

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

#if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
  using SpiInterface = SimpleSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
#elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(spiInstance, LATCH_PIN);
#elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
  SpiInterface spiInterface(spiInstance);
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

// LED patterns
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
#if INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI \
    || INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
  spiInstance.begin();
#endif

  spiInterface.begin();
  ledModule.begin();
}

//----------------------------------------------------------------------------

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
  if ((uint16_t) (nowMillis - prevSampleMillis) >= 10) {
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
