/*
 * Write digits 0-3 into an 4-digit LED module using the HT16K33 ship, set the
 * brightness, then render it by flushing the data bits to the HT16K33
 * controller over I2C using the TwoWireInterface class from the AceWire
 * library.
 */

#include <Arduino.h>
#include <Wire.h> // TwoWire, Wire
#include <AceWire.h> // TwoWireInterface
#include <AceSegment.h> // Ht16k33Module, NumberWriter

using ace_wire::TwoWireInterface;
using ace_segment::LedModule;
using ace_segment::Ht16k33Module;
using ace_segment::NumberWriter;

// Replace these with the PIN numbers of your dev board.
const uint8_t SDA_PIN = SDA;
const uint8_t SCL_PIN = SCL;
const uint8_t HT16K33_I2C_ADDRESS = 0x70;
const uint8_t NUM_DIGITS = 4;

using WireInterface = TwoWireInterface<TwoWire>;
WireInterface wireInterface(Wire);
Ht16k33Module<WireInterface, NUM_DIGITS> ledModule(
    wireInterface, HT16K33_I2C_ADDRESS);
NumberWriter<LedModule> numberWriter(ledModule);

void setup() {
  delay(1000);

  Wire.begin();
  wireInterface.begin();
  ledModule.begin();

  numberWriter.writeHexCharAt(0, 0);
  numberWriter.writeHexCharAt(1, 1);
  numberWriter.writeHexCharAt(2, 2);
  numberWriter.writeHexCharAt(3, 3);

  ledModule.setBrightness(2);

  ledModule.flush();
}

void loop() {}
