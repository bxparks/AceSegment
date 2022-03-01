/*
 * A demo of a custom 4-digit LED module with the segment pins connected to a
 * 74HC595 shift register and the digit pins connected directly to the
 * microcontroller. Displays the digits 0 to 3, then slowly rotates the digits
 * to the left, while incrementing the brightness of the display. Uses the
 * HybridModule class.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_CUSTOM_SINGLE: SparkFun Pro Micro
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSPI.h>
#include <AceSegment.h> // HybridModule

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_spi/HardSpiFastInterface.h>
#include <ace_spi/SimpleSpiFastInterface.h>
#endif

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_spi::SimpleSpiInterface;
using ace_spi::HardSpiInterface;
using ace_spi::SimpleSpiFastInterface;
using ace_spi::HardSpiFastInterface;
using ace_segment::LedModule;
using ace_segment::HybridModule;
using ace_segment::kActiveHighPattern;

// Select interface protocol.
#define INTERFACE_TYPE_SIMPLE_SPI 0
#define INTERFACE_TYPE_SIMPLE_SPI_FAST 1
#define INTERFACE_TYPE_HARD_SPI 2
#define INTERFACE_TYPE_HARD_SPI_FAST 3
#define INTERFACE_TYPE_SIMPLE_TMI 4
#define INTERFACE_TYPE_SIMPLE_TMI_FAST 5

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_CUSTOM_SINGLE
#endif

#if defined(EPOXY_DUINO)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_CUSTOM_SINGLE)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  SPIClass& spiInstance = SPI;

#else
  #error Unknown environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

// LED segment patterns.
const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;
const uint8_t LATCH_PIN = 10;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

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

// Common Cathode, with transistors on Group pins
HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
    spiInterface,
    kActiveHighPattern /*segmentOnPattern*/,
    kActiveHighPattern /*digitOnPattern*/,
    FRAMES_PER_SECOND,
    DIGIT_PINS
);

// LED patterns
const uint8_t PATTERNS[NUM_DIGITS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
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

// loop() state variables
TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightnessIndex = 0;
uint16_t prevUpdateMillis = 0;

#if ENABLE_SERIAL_DEBUG >= 1
uint16_t prevStatsMillis = 0;
#endif

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
