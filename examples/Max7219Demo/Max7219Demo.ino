/*
 * A demo of an 8-digit LED module using a MAX7219 chip. Displays the digits 0
 * to 7 on the module, then slowly rotates the digits to the left, while
 * incrementing the brightness of the display. Uses the Max7219Module class.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_MAX7219: SparkFun Pro Micro
 *  * AUNITER_SAMD_HT16K33: SAMD21 M0 Mini
 *  * AUNITER_STM32_MAX7219: STM32 F1 Blue Pill
 *  * AUNITER_D1MINILARGE_MAX7219: WeMos D1 Mini ESP8266
 *  * AUNITER_ESP32_MAX7219: ESP32 Dev Kit v1
 */

#include <Arduino.h>
#include <SPI.h> // SPI, SPIClass
#include <AceCommon.h> // incrementMod()
#include <AceSPI.h>
#include <AceSegment.h> // Max7219Module

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_spi::HardSpiInterface;
using ace_spi::SimpleSpiInterface;
using ace_segment::LedModule;
using ace_segment::Max7219Module;
using ace_segment::kDigitRemapArray8Max7219;

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
  #define AUNITER_MICRO_MAX7219
#endif

#if defined(EPOXY_DUINO)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST

  // SPI pins
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_MAX7219)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST

  // SPI pins
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_SAMD_MAX7219)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI

  // SPI pins
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_STM32_MAX7219)
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

#elif defined(AUNITER_D1MINI_LARGE_MAX7219)
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI

  // SPI pins
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_ESP32_MAX7219)
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI

  // This dev board uses secondary HSPI pins.
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
  #include <ace_spi/HardSpiFastInterface.h>
  #include <ace_spi/SimpleSpiFastInterface.h>
  using ace_spi::SimpleSpiFastInterface;
  using ace_spi::HardSpiFastInterface;
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

#if INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
  using SpiInterface = HardSpiInterface<SPIClass>;
  SpiInterface spiInterface(spiInstance, LATCH_PIN);
#elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
  using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
  SpiInterface spiInterface(spiInstance);
#elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
  using SpiInterface = SimpleSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
  using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
#else
  #error Unknown INTERFACE_TYPE
#endif

Max7219Module<SpiInterface, NUM_DIGITS> ledModule(
    spiInterface, kDigitRemapArray8Max7219);

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
uint8_t brightness = 1;

// Every second, scroll the display and change the brightness.
void updateDisplay() {
  static uint16_t prevChangeMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevChangeMillis) >= 1000) {
    prevChangeMillis = nowMillis;

    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      // Write a decimal point every other digit, for demo purposes.
      uint8_t pattern = PATTERNS[j] | ((j & 0x1) ? 0x80 : 0x00);
      ledModule.setPatternAt(i, pattern);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness. The MAX7219 has 16 levels of brightness.
    ledModule.setBrightness(brightness);
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
    ledModule.flush();
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
