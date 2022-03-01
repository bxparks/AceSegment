/*
 * Same as Hc595Demo, but using timer interrupts (through TimerOne library) to
 * render the LED display.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_HC595: SparkFun Pro Micro
 *  * AUNITER_MICRO_CUSTOM_DUAL: SparkFun Pro Micro
 *
 * The following configurations are defined, but I don't think they work because
 * <TimerOne.h> library may not be compatible with these:
 *
 *  * AUNITER_STM32_HC595: STM32 F1 Blue Pill
 *  * AUNITER_D1MINILARGE_HC595: WeMos D1 Mini ESP8266
 *  * AUNITER_ESP32_HC595: ESP32 Dev Kit v1
 *
 * CAUTION: The digitalWriteFast libraries are not interrupt-safe, so the
 * SimpleSpiFastInterface or HardSpiFastInterface classes are not
 * interrupt-safe. In this simple example, no other GPIO port mutations are
 * performed, so the act of writing to the 74HC595 chip within the interrupt
 * handler (through the Timer1 library) is probably safe. But if the application
 * code performs any other GPIO operation using the digitalWriteFast library,
 * then the interrupt handler may interrupt GPIO operation cause spurious
 * results. Interrupt-safety of the various "Fast" classes in this library may
 * be addressed in the future.
 */

#include <Arduino.h>
#include <SPI.h>
#include <AceCommon.h> // incrementMod()
#include <AceSPI.h>
#include <AceSegment.h> // Hc595Module
#include <TimerOne.h> // Timer1

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

// Type of LED Module
#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1
#define LED_DISPLAY_TYPE_MAX7219 2
#define LED_DISPLAY_TYPE_HC595 3
#define LED_DISPLAY_TYPE_DIRECT 4
#define LED_DISPLAY_TYPE_HYBRID 5
#define LED_DISPLAY_TYPE_FULL 6

// Used by LED_DISPLAY_TYPE_PARTIAL, LED_DISPLAY_TYPE_FULL,
// LED_DISPLAY_TYPE_HC595, and LED_DISPLAY_TYPE_TM1637.
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
  #define AUNITER_MICRO_HC595
#endif

#if defined(EPOXY_DUINO)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
  const uint8_t DIGIT_ON_PATTERN = kActiveLowPattern;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t* const REMAP_ARRAY = nullptr;

  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
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

  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_HC595)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
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

#elif defined(AUNITER_STM32_HC595)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
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

  // These are the primary VSPI pins for reference.
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
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 9, 5, 2
};

// Common Cathode, with transistors on Group pins
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

// Update the display with new pattern and brightness every second.
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

// Call renderFieldNow() through a timer interrupt.
void flushModule() {
  ledModule.renderFieldNow();
}

void setupTimer() {
  Timer1.initialize(ledModule.getMicrosPerField());
  Timer1.attachInterrupt(flushModule);
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
  setupTimer();
}

void loop() {
  updateDisplay();
  printStats();
}
