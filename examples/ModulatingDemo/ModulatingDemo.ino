/*
 * Demo the ability to control the brightness of each digit separately.
 * Write the digits 0 to 7, with each digit at slightly different brightness
 * levels, then rotates the digits to the left, giving the appearance of a
 * modulating waves moving to the left.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_CUSTOM_DIRECT: Pro Micro + Custom Direct module
 *  * AUNITER_MICRO_CUSTOM_SINGLE: Pro Micro + Custom Single 74HC595 module
 *  * AUNITER_MICRO_CUSTOM_DUAL: Pro Micro + Custom Dual 74HC595 module
 *  * AUNITER_MICRO_HC595: Pro Micro + diymore.cc 74HC595 module
 *  * AUNITER_SAMD_HC595: SAMD21 M0 Mini + diymore.cc 74HC595 module
 *  * AUNITER_STM32_HC595: STM32 F1 Blue Pill+ diymore.cc 74HC595 module
 *  * AUNITER_D1MINI_LARGE_HC595: WeMos D1 Mini + diymore.cc 74HC595 module
 *  * AUNITER_ESP32_HC595: ESP32 dev kit v1 + diymore.cc 74HC595 module
 */
#include <Arduino.h>
#include <SPI.h>
#include <AceCommon.h> // incrementMod()
#include <AceSPI.h>
#include <AceSegment.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  #include <digitalWriteFast.h>
  #include <ace_spi/SimpleSpiFastInterface.h>
  #include <ace_spi/HardSpiFastInterface.h>
  #include <ace_segment/direct/DirectFast4Module.h>
  using ace_spi::SimpleSpiFastInterface;
  using ace_spi::HardSpiFastInterface;
  using ace_segment::DirectFast4Module;
#endif

using ace_common::incrementMod;
using ace_spi::HardSpiInterface;
using ace_segment::LedModule;
using ace_segment::kByteOrderSegmentHighDigitLow;
using ace_segment::kDigitRemapArray8Hc595;
using ace_segment::kActiveLowPattern;
using ace_segment::kActiveHighPattern;
using ace_segment::DirectModule;
using ace_segment::HybridModule;
using ace_segment::Hc595Module;

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

// Type of LED Module
#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1
#define LED_DISPLAY_TYPE_MAX7219 2
#define LED_DISPLAY_TYPE_HC595 3
#define LED_DISPLAY_TYPE_DIRECT 4
#define LED_DISPLAY_TYPE_HYBRID 5
// FULL is similar to HC595, but using my own custom 4-digit module, instead of
// the off-the-shelf 8-digit module.
#define LED_DISPLAY_TYPE_FULL 6

// Used by LED_DISPLAY_TYPE_DIRECT
#define DIRECT_INTERFACE_TYPE_NORMAL 0
#define DIRECT_INTERFACE_TYPE_FAST_4 1

// Used by LED_DISPLAY_TYPE_HYBRID, LED_DISPLAY_TYPE_FULL, and
// LED_DISPLAY_TYPE_HC595.
#define INTERFACE_TYPE_SIMPLE_SPI 0
#define INTERFACE_TYPE_SIMPLE_SPI_FAST 1
#define INTERFACE_TYPE_HARD_SPI 2
#define INTERFACE_TYPE_HARD_SPI_FAST 3

//------------------------------------------------------------------
// Hardware environment configuration.
//------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_HC595
#endif

#if defined(EPOXY_DUINO)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 4;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = nullptr;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_CUSTOM_DIRECT)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_DIRECT
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;

  // Choose one of the following variants:
  //#define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_NORMAL
  #define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_FAST_4
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_CUSTOM_SINGLE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HYBRID
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_CUSTOM_DUAL)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  const uint8_t NUM_DIGITS = 4;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveLowPattern;
  const uint8_t* const REMAP_ARRAY = nullptr;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_STM32_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI

  // This dev board uses the primary SPI1 pins.
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

  // These are the secondar SPI2 pins for reference.
  // const uint8_t LATCH_PIN = PB12;
  // const uint8_t DATA_PIN = PB15;
  // const uint8_t CLOCK_PIN = PB13;
  // SPIClass spiInstance(DATA_PIN, PB14 /*miso*/, CLOCK_PIN);

#elif defined(AUNITER_D1MINI_LARGE_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_ESP32_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
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

  // These are the secondary VSPI pins for reference.
  // const uint8_t LATCH_PIN = SS;
  // const uint8_t DATA_PIN = MOSI;
  // const uint8_t CLOCK_PIN = SCK;
  // SPIClass& spiInstance = SPI;

#else
  #error Unsupported AUNITER environment

#endif

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

// LED segment patterns for up to 8 digits.
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

// Total fields/second
//      = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//      = 60 * 16 * 4
//      = 3840 fields/sec
//      => 260 micros/field
//
// According to AutoBenchmark, almost all versions of ScanningModule with
// various LedMatrix can render a single field in less than this on 16 MHz AVR
// processor. The combination of (ScanningModule + LedMatrixDualHc595 +
// SimpleSpiInterface) is the exception.
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 7, 3, 2
};

// The chain of resources.
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  // Common Anode, with transitors on Group pins
  #if DIRECT_INTERFACE_TYPE == DIRECT_INTERFACE_TYPE_NORMAL
    DirectModule<NUM_DIGITS, NUM_SUBFIELDS> ledModule(
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        SEGMENT_PINS,
        DIGIT_PINS);
  #else
    DirectFast4Module<
        8, 9, 10, 16, 14, 18, 19, 15, // segment pins
        4, 5, 6, 7, // digit pins
        NUM_DIGITS,
        NUM_SUBFIELDS
    > ledModule(
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND);
  #endif

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID
  // Common Cathode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(SPI);
  #endif
  HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL \
    || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595

  // Common Anode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(SPI);
  #endif
  Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
      spiInterface,
      SEGMENT_ON_PATTERN,
      DIGIT_ON_PATTERN,
      FRAMES_PER_SECOND,
      HC595_BYTE_ORDER,
      REMAP_ARRAY
  );

#else
  #error Unknown LED_DISPLAY_TYPE

#endif

// Setup the various resources.
void setupAceSegment() {
#if INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI \
    || INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
  spiInstance.begin();
#endif

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL \
    || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID \
    || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595
  spiInterface.begin();
#endif

  ledModule.begin();
  ledModule.setBrightness(1); // 0-1
}

//------------------------------------------------------------------
// Configurations for ModulationDemo
//------------------------------------------------------------------

void setupPulseDisplay() {
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    ledModule.setPatternAt(i, PATTERNS[i]);
  }
}

void setBrightnesses(int brightnessIndex) {
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    uint8_t brightness = BRIGHTNESS_LEVELS[
        (brightnessIndex + i) % NUM_BRIGHTNESSES];
    ledModule.setBrightnessAt(i, brightness);
  }
}

// Change the brightness every 200 ms.
void pulseDisplay() {
  static uint8_t brightnessIndex;
  static uint16_t lastUpdateMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - lastUpdateMillis) >= 200) {
    lastUpdateMillis = nowMillis;
    incrementMod(brightnessIndex, NUM_BRIGHTNESSES);
    setBrightnesses(brightnessIndex);
  }
}

void flushModule() {
  ledModule.renderFieldWhenReady();
}

//-----------------------------------------------------------------------------

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
#endif

  setupAceSegment();
  setupPulseDisplay();
}

void loop() {
  pulseDisplay();
  flushModule();
}
