/*
 * A demo of reading the 8 buttons the TM1638 LED module. It display "00000000"
 * on the 8-digit LED. When a button is pressed, the corresponding digit changes
 * to a 1, for example, "01000000". The brightess of the LED is set to the value
 * of the button (0 for the left most, 7 for the right most). Multiple buttons
 * can be pressed at the same time.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_TM1638: SparkFun Pro Micro + TM1638-8 LED module
 *  * AUNITER_STM32_TM1638: STM32 F1 Blue Pill + TM1638-8 LED module
 *  * AUNITER_D1MINI_LARGE_TM1638: WeMos D1 Mini ESP8266
 *  * AUNITER_ESP32_TM1638: ESP32 Dev Kit v1
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceTMI.h> // SimpleTmi1638Interface
#include <AceSegment.h> // Tm1638Module

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_tmi::SimpleTmi1638Interface;
using ace_segment::LedModule;
using ace_segment::Tm1638Module;

// Set to 1 to get diagnostic info.
#define ENABLE_SERIAL_DEBUG 0

// Select TM1638 protocol version, either SimpleTmi1638Interface or
// SimpleTmi1638FastInterface.
#define TMI_INTERFACE_TYPE_NORMAL 0
#define TMI_INTERFACE_TYPE_FAST 1

// Number of digits in the TM1638 module.
const uint8_t NUM_DIGITS = 8;

// Number of buttons in the TM1638 module.
const uint8_t NUM_BUTTONS = 8;

//----------------------------------------------------------------------------
// Hardware configuration.
// The TM1638 control lines are basically identically to SPI lines, so let's
// reuse the SPI pins used by the 74HC595 LED modules.
//----------------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_TM1638
#endif

#if defined(EPOXY_DUINO)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_FAST

  const uint8_t CLK_PIN = SCK;
  const uint8_t DIO_PIN = MOSI;
  const uint8_t STB_PIN = 10;

#elif defined(AUNITER_MICRO_TM1638)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  const uint8_t CLK_PIN = SCK;
  const uint8_t DIO_PIN = MOSI;
  const uint8_t STB_PIN = 10;

#elif defined(AUNITER_SAMD_TM1638)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  const uint8_t CLK_PIN = SCK;
  const uint8_t DIO_PIN = MOSI;
  const uint8_t STB_PIN = SS;

#elif defined(AUNITER_STM32_TM1638)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  // This dev board uses the primary SPI1 pins.
  const uint8_t CLK_PIN = SCK;
  const uint8_t DIO_PIN = MOSI;
  const uint8_t STB_PIN = SS;

  // These are the secondary SPI2 pins for reference.
  // const uint8_t CLK_PIN = PB13;
  // const uint8_t DIO_PIN = PB15;
  // const uint8_t STB_PIN = PB12;

#elif defined(AUNITER_D1MINI_LARGE_TM1638)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  const uint8_t CLK_PIN = SCK;
  const uint8_t DIO_PIN = MOSI;
  const uint8_t STB_PIN = SS;

#elif defined(AUNITER_ESP32_TM1638)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL
  // This dev board uses the secondary HSPI pins.
  const uint8_t CLK_PIN = 14;
  const uint8_t DIO_PIN = 13;
  const uint8_t STB_PIN = 15;

  // These are the pimary VSPI pins for reference.
  // const uint8_t CLK_PIN = SCK;
  // const uint8_t DIO_PIN = MOSI;
  // const uint8_t STB_PIN = SS;

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

// The TM1638 should be able to handle 500 kHz clock. Using a 1 micros delay
// results in 3 micros for a full cycle, or about 300 kHz, which is well within
// the specs of the TM1638.
const uint8_t DELAY_MICROS = 1;

#if TMI_INTERFACE_TYPE == TMI_INTERFACE_TYPE_NORMAL
  using Tmi1638Interface = SimpleTmi1638Interface;
  Tmi1638Interface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, DELAY_MICROS);
#elif TMI_INTERFACE_TYPE == TMI_INTERFACE_TYPE_FAST
  #include <digitalWriteFast.h>
  #include <ace_tmi/SimpleTmi1638FastInterface.h>
  using ace_tmi::SimpleTmi1638FastInterface;

  using Tmi1638Interface = SimpleTmi1638FastInterface<
      DIO_PIN, CLK_PIN, STB_PIN, DELAY_MICROS>;
  Tmi1638Interface tmiInterface;
#else
  #error Unknown TMI_INTERFACE_TYPE
#endif

Tm1638Module<Tmi1638Interface, NUM_DIGITS> ledModule(tmiInterface);

void setupAceSegment() {
  tmiInterface.begin();
  ledModule.begin();
}

//----------------------------------------------------------------------------

const uint8_t kZeroPattern = 0b00111111;
const uint8_t kOnePattern = 0b00000110;

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 0; // [0, 7] with 0 being dimmest

// This array contains the button position to its bit map position in the
// `buttonCode` returned by the Tm1637Module::readButtons() method. All 8
// buttons on this TM1638 module are on the K3 pin. They are wired to the
// KSn/SEGn lines in the following way, from left (Button0) to right (Button7):
//
//  * Button0 = S1 = KS1/SEG1 = Bit 0
//  * Button1 = S2 = KS3/SEG3 = Bit 8
//  * Button2 = S3 = KS5/SEG5 = Bit 16
//  * Button3 = S4 = KS7/SEG7 = Bit 24
//  * Button4 = S5 = KS2/SEG2 = Bit 4
//  * Button5 = S6 = KS4/SEG4 = Bit 12
//  * Button6 = S7 = KS6/SEG6 = Bit 20
//  * Button7 = S8 = KS8/SEG8 = Bit 28
//
// The KSn/SEGn values are sent out to the bus in 4 bytes, in the following
// order. The bits are encoded in LSBFIRST order, so KS1 are in b0-b3, and KS2
// are in b4-7:
//
//  * KS1/KS2 = Byte0
//  * KS3/KS4 = Byte1
//  * KS5/KS6 = Byte2
//  * KS7/KS8 = Byte3
//
// The 4 bytes are collected into a single 32-bit integer in little-endian order
// by Tm1637Module::readButtons(). In other words, the least significant byte
// appears first. We can then calculate the bit number in the 32-bit buttonCode
// that corresponds to each button as indicated in this array:
const uint8_t kButtonBitPosition[] = {
  0, 8, 16, 24, 4, 12, 20, 28
};

// Read buttons.
//  * SparkFun Pro Micro: min/avg/max:752/757/764
//  * ESP32: min/avg/max:241/243/246
uint32_t readButtons() {
  uint16_t startMicros = micros();
  uint32_t buttonCode = ledModule.readButtons();
  uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
  stats.update(elapsedMicros);
  return buttonCode;
}

// Update the display with the key scan result. Display "0" if button is
// unpressed; a "1" if corresponding button is pressed.
void updateDisplay(uint32_t buttonCode) {
  for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
    uint32_t buttonMask = (uint32_t) 0x1 << kButtonBitPosition[i];
    if (buttonCode & buttonMask) {
      ledModule.setPatternAt(i, kOnePattern);

      // Set the brightness to be the same value as the button.
      ledModule.setBrightness(i);
    } else {
      ledModule.setPatternAt(i, kZeroPattern);
    }
  }
}

// Every 100 ms, read the buttons, update the display, and flush() to the LED
// module which updates all digits, including brightness.
void update() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;
    uint32_t buttonCode = readButtons();
    updateDisplay(buttonCode);
    ledModule.flush();
  }
}

// Every 5 seconds, print stats about how long flush() or flushIncremental()
// took.
void printStats() {
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevStatsMillis;

  // Every 5 seconds, print out the statistics.
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
  ledModule.setBrightness(0);
}

void loop() {
  update();
  printStats();
}
