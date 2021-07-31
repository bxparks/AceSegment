/*
 * Write digits 0-3 into a 4-digit LED module using the TM1637 chip, set the
 * brightness, then render it by flushing the data bits to the TM1637 controller
 * using the SimpleTmiInterface class from the AceTMI library.
 */

#include <Arduino.h>
#include <AceTMI.h> // SimpleTmiInterface
#include <AceSegment.h> // Tm1637Module, NumberWriter

using ace_tmi::SimpleTmiInterface;
using ace_segment::Tm1637Module;
using ace_segment::NumberWriter;

// Replace these with the PIN numbers of your dev board.
const uint8_t CLK_PIN = A0;
const uint8_t DIO_PIN = 9;
const uint8_t NUM_DIGITS = 4;

// Many TM1637 LED modules contain 10 nF capacitors on their DIO and CLK lines
// which are unreasonably high. This forces a 100 microsecond delay between
// bit transitions. If you remove those capacitors, you can set this as low as
// 1-5 micros.
const uint8_t DELAY_MICROS = 100;

using TmiInterface = SimpleTmiInterface;
TmiInterface tmiInterface(DIO_PIN, CLK_PIN, DELAY_MICROS);
Tm1637Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);
NumberWriter numberWriter(ledModule);

void setupAceSegment() {
}

void setup() {
  delay(1000);

  tmiInterface.begin();
  ledModule.begin();

  numberWriter.writeHexCharAt(0, 0);
  numberWriter.writeHexCharAt(1, 1);
  numberWriter.writeHexCharAt(2, 2);
  numberWriter.writeHexCharAt(3, 3);

  ledModule.setBrightness(2);

  ledModule.flush();
}

void loop() {}
