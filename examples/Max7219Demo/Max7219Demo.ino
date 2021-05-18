/*
 * A simple demo of a single, 8-digit LED module using a MAX7219 chip.
 * Display the digits 0 to 7 on the module.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // Max7219Module, LedDisplay

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_segment::Max7219Module;
using ace_segment::LedDisplay;
using ace_segment::HardSpiInterface;
using ace_segment::SoftSpiInterface;
using ace_segment::kDigitRemapArray8Max7219;

// Select SPI interface type.
#define SPI_INTERFACE_TYPE_HARD_SPI 0
#define SPI_INTERFACE_TYPE_HARD_SPI_FAST 1
#define SPI_INTERFACE_TYPE_SOFT_SPI 2
#define SPI_INTERFACE_TYPE_SOFT_SPI_FAST 3

#if defined(EPOXY_DUINO)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_HARD_SPI_FAST

  // SPI pins
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_MAX7219)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_HARD_SPI_FAST

  // SPI pins
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_STM32_MAX7219)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_HARD_SPI

  // SPI pins
  const uint8_t LATCH_PIN = PA4;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_D1MINI_LARGE_MAX7219)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_HARD_SPI

  // SPI pins
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#else
  #error Unknown environment
#endif

#if SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_HARD_SPI_FAST \
    || SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_SOFT_SPI_FAST
  #include <digitalWriteFast.h>
  #include <ace_segment/hw/HardSpiFastInterface.h>
  #include <ace_segment/hw/SoftSpiFastInterface.h>
  using ace_segment::SoftSpiFastInterface;
  using ace_segment::HardSpiFastInterface;
#endif

// LED segment patterns.
const uint8_t NUM_DIGITS = 8;
const uint8_t PATTERNS[NUM_DIGITS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
};

#if SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_HARD_SPI
  using SpiInterface = HardSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_HARD_SPI_FAST
  using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
#elif SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_SOFT_SPI
  using SpiInterface = SoftSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_SOFT_SPI_FAST
  using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
#else
  #error Unknown SPI_INTERFACE_TYPE
#endif

Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
    spiInterface, kDigitRemapArray8Max7219);
LedDisplay display(max7219Module);

void setupAceSegment() {
  spiInterface.begin();
  max7219Module.begin();
}

//----------------------------------------------------------------------------

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 1;

// Every second, scroll the display and change the brightness.
void updateDisplay() {
  static uint16_t prevChangeMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevChangeMillis) >= 1000) {
    prevChangeMillis = nowMillis;

    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      display.writePatternAt(i, PATTERNS[j]);
      // Write a decimal point every other digit, for demo purposes.
      display.writeDecimalPointAt(i, j & 0x1);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness. The MAX7219 has 16 levels of brightness.
    display.setBrightness(brightness);
    incrementMod(brightness, (uint8_t) 16);
  }
}

// Every 100 ms, unconditionally flush() to the LED module which updates all
// digits, including brightness. It takes only about 170 microseconds to flush
// everything to the MAX7219 over SPI, which is so fast it does not seem
// necessary to perform any incremental updates.
void flushModule() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;

    // Flush the change to the LED Module, and measure the time.
    uint16_t startMicros = micros();
    max7219Module.flush();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
  }
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

//-----------------------------------------------------------------------------

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
