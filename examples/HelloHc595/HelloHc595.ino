/*
 * Write digits 0-7 into an 8-digit LED module using two 74HC595 shift register
 * chips, set the brightness, then render it by flushing the data bits to the
 * 74HC595 controllers over SPI using the HardSpiInterface class from the AceSPI
 * library. The rendering must be multiplexed in the loop() because the 74HC595
 * turns on only a single segment of each digit at any given time. We have to
 * strobe through all the segments faster than the human vision response time
 * (~16 micros) to give the illusion of illuminating the entire display.
 */

#include <Arduino.h>
#include <SPI.h> // SPIClass, SPI
#include <AceSPI.h> // HardSpiInterface
#include <AceSegment.h> // Hc595Module

using ace_spi::HardSpiInterface;
using ace_segment::LedModule;
using ace_segment::Hc595Module;
using ace_segment::kDigitRemapArray8Hc595;
using ace_segment::kByteOrderSegmentHighDigitLow;
using ace_segment::kActiveLowPattern;
using ace_segment::kActiveHighPattern;

// Replace these with the PIN numbers of your dev board.
const uint8_t LATCH_PIN = 10;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t NUM_DIGITS = 8;

const uint8_t SEGMENT_ON_PATTERN = kActiveLowPattern;
const uint8_t DIGIT_ON_PATTERN = kActiveHighPattern;
const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
const uint8_t* const REMAP_ARRAY = kDigitRemapArray8Hc595;
const uint8_t NUM_SUBFIELDS = 1;
const uint8_t FRAMES_PER_SECOND = 60;

using SpiInterface = HardSpiInterface<SPIClass>;
SpiInterface spiInterface(SPI, LATCH_PIN);
Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
    spiInterface,
    SEGMENT_ON_PATTERN,
    DIGIT_ON_PATTERN,
    FRAMES_PER_SECOND,
    HC595_BYTE_ORDER,
    REMAP_ARRAY
);

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

  // Brightness not supported when NUM_SUBFIELDS == 1.
  // ledModule.setBrightness(2);
}

void loop() {
  ledModule.renderFieldWhenReady();
}
