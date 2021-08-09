/*
 * Write digits 0-7 into an 8-digit LED module using the MAX7219, set the
 * brightness, then render it by flushing the data bits to the MAX7219
 * controller over SPI using the HardSpiInterface class from the AceSPI library.
 */

#include <Arduino.h>
#include <SPI.h> // SPIClass, SPI
#include <AceSPI.h> // HardSpiInterface
#include <AceSegment.h> // Max7219Module

using ace_spi::HardSpiInterface;
using ace_segment::LedModule;
using ace_segment::Max7219Module;
using ace_segment::kDigitRemapArray8Max7219;

// Replace these with the PIN numbers of your dev board.
const uint8_t LATCH_PIN = 10;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t NUM_DIGITS = 8;

using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(SPI, LATCH_PIN);
Max7219Module<SpiInterface, NUM_DIGITS> ledModule(
    spiInterface, kDigitRemapArray8Max7219);

// LED segment patterns.
const uint8_t NUM_PATTERNS = 10;
const uint8_t PATTERNS[NUM_PATTERNS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
};

void setup() {
  delay(1000);

  SPI.begin();
  spiInterface.begin();
  ledModule.begin();

  ledModule.setPatternAt(0, PATTERNS[0]);
  ledModule.setPatternAt(1, PATTERNS[1]);
  ledModule.setPatternAt(2, PATTERNS[2]);
  ledModule.setPatternAt(3, PATTERNS[3]);
  ledModule.setPatternAt(4, PATTERNS[4]);
  ledModule.setPatternAt(5, PATTERNS[5]);
  ledModule.setPatternAt(6, PATTERNS[6]);
  ledModule.setPatternAt(7, PATTERNS[7]);

  ledModule.setBrightness(2);

  ledModule.flush();
}

void loop() {}
