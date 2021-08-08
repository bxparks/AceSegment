/*
 * Write digits 0-7 into an 8-digit LED module using the MAX7219, set the
 * brightness, then render it by flushing the data bits to the MAX7219
 * controller over SPI using the HardSpiInterface class from the AceSPI library.
 */

#include <Arduino.h>
#include <SPI.h> // SPIClass, SPI
#include <AceSPI.h> // HardSpiInterface
#include <AceSegment.h> // Max7219Module, NumberWriter

using ace_spi::HardSpiInterface;
using ace_segment::LedModule;
using ace_segment::Max7219Module;
using ace_segment::NumberWriter;
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
NumberWriter<LedModule> numberWriter(ledModule);

void setup() {
  delay(1000);

  SPI.begin();
  spiInterface.begin();
  ledModule.begin();

  numberWriter.writeHexCharAt(0, 0);
  numberWriter.writeHexCharAt(1, 1);
  numberWriter.writeHexCharAt(2, 2);
  numberWriter.writeHexCharAt(3, 3);
  numberWriter.writeHexCharAt(4, 4);
  numberWriter.writeHexCharAt(5, 5);
  numberWriter.writeHexCharAt(6, 6);
  numberWriter.writeHexCharAt(7, 7);

  ledModule.setBrightness(2);

  ledModule.flush();
}

void loop() {}
