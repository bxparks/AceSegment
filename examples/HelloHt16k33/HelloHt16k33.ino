/*
 * Write digits 0-3 into an 4-digit LED module using the HT16K33 ship, set the
 * brightness, then render it by flushing the data bits to the HT16K33
 * controller over I2C using the TwoWireInterface class from the AceWire
 * library.
 */

#include <Arduino.h>
#include <Wire.h> // TwoWire, Wire
#include <AceWire.h> // TwoWireInterface
#include <AceSegment.h> // Ht16k33Module

using ace_wire::TwoWireInterface;
using ace_segment::LedModule;
using ace_segment::Ht16k33Module;

// Replace these with the PIN numbers of your dev board.
const uint8_t SDA_PIN = SDA;
const uint8_t SCL_PIN = SCL;
const uint8_t HT16K33_I2C_ADDRESS = 0x70;
const uint8_t NUM_DIGITS = 4;

using WireInterface = TwoWireInterface<TwoWire>;
WireInterface wireInterface(Wire);
Ht16k33Module<WireInterface, NUM_DIGITS> ledModule(
    wireInterface, HT16K33_I2C_ADDRESS);

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

  Wire.begin();
  wireInterface.begin();
  ledModule.begin();

  ledModule.setPatternAt(0, PATTERNS[0]);
  ledModule.setPatternAt(1, PATTERNS[1]);
  ledModule.setPatternAt(2, PATTERNS[2]);
  ledModule.setPatternAt(3, PATTERNS[3]);

  ledModule.setBrightness(2);

  ledModule.flush();
}

void loop() {}
