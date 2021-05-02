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
using ace_segment::HwSpiInterface;
using ace_segment::SwSpiInterface;
using ace_segment::kEightDigitRemapArray;

// Select SPI interface type.
#define SPI_INTERFACE_TYPE_HW 0
#define SPI_INTERFACE_TYPE_SW 1
#define SPI_INTERFACE_TYPE_SW_FAST 2

#if defined(EPOXY_DUINO)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_SW_FAST

  // SPI pins
  const uint8_t LATCH_PIN = A0;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_LEDCLOCK_MAX7219)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_SW

  // SPI pins
  const uint8_t LATCH_PIN = A0;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_D1MINI_LARGE_MAX7219)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_SW

  // SPI pins
  const uint8_t LATCH_PIN = D8;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_STM32_MAX7219)
  #define SPI_INTERFACE_TYPE SPI_INTERFACE_TYPE_SW

  // SPI pins
  const uint8_t LATCH_PIN = PA4;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#else
  #error Unknown environment
#endif

#if SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_SW_FAST
  #include <digitalWriteFast.h>
  #include <ace_segment/hw/SwSpiFastInterface.h>
  using ace_segment::SwSpiFastInterface;
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

#if SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_HW
  using SpiInterface = HwSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_SW
  using SpiInterface = SwSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
#elif SPI_INTERFACE_TYPE == SPI_INTERFACE_TYPE_SW_FAST
  using SpiInterface = SwSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiInterface spiInterface;
#else
  #error Unknown SPI_INTERFACE_TYPE
#endif

Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
    spiInterface, kEightDigitRemapArray);
LedDisplay display(max7219Module);

TimingStats stats;

uint8_t digitIndex = 0;
uint8_t brightness = 1;

void setup() {
  delay(1000);
#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  spiInterface.begin();
  max7219Module.begin();
}

// Use the Max7219Module::flush() method to update all digits in a single dump
// to the LED module, taking about ~170 microseconds per flush().
void loop() {
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
  display.setBrightness(brightness);
  incrementMod(brightness, (uint8_t) 16);

  // Flush the change to the LED Module, and measure the time.
  uint16_t startMicros = micros();
  max7219Module.flush();
  uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
  stats.update(elapsedMicros);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.print("ExpAvg:");
  Serial.println(stats.getExpDecayAvg());
#endif

  delay(1000);
}
