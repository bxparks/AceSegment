/*
 * Write digits 0-7 into a 8-digit LED module using the TM1638 chip, set the
 * brightness, then render it by flushing the data bits to the TM1638 controller
 * using the SimpleTmi1638Interface class from the AceTMI library.
 */

#include <Arduino.h>
#include <AceTMI.h> // SimpleTmi1638Interface
#include <AceSegment.h> // Tm1638Module

using ace_tmi::SimpleTmi1638Interface;
using ace_segment::Tm1638Module;

// Replace these with the PIN numbers of your dev board.
// The TM1638 protocol is very similar to SPI, so I often use the SPI pins.
const uint8_t CLK_PIN = SCK;
const uint8_t DIO_PIN = MOSI;
const uint8_t STB_PIN = SS;
const uint8_t NUM_DIGITS = 8;

// My TM1638 LED module contains no filtering capacitor on the DIO, CLK, and STB
// lines. So it should support the highest clock frequency using a 1 micro
// transition delay.
const uint8_t DELAY_MICROS = 1;

using TmiInterface = SimpleTmi1638Interface;
TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, DELAY_MICROS);
Tm1638Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);

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

  tmiInterface.begin();
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
