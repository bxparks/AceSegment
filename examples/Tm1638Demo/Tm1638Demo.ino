/*
 * A demo of a single TM1638 LED module, with the digits [0,3] or [0,5]
 * scrolling to the left every second, and the brightness changing each
 * iteration.
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
using ace_segment::Tm1638Module;

// Set to 1 to get diagnostic info.
#define ENABLE_SERIAL_DEBUG 0

// Select TM1638 protocol version, either SimpleTmi1638Interface or
// SimpleTmi1638FastInterface.
#define TMI_INTERFACE_TYPE_NORMAL 0
#define TMI_INTERFACE_TYPE_FAST 1

// Number of digits in the TM1638 module.
const uint8_t NUM_DIGITS = 8;

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
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_FAST

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

// The TM1638 controller supports up to 8 digits.
const uint8_t PATTERNS[8] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
};

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 0; // [0, 7] with 0 being dimmest

// Every second, scroll the display and change the brightness.
void updateDisplay() {
  static uint16_t prevChangeMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevChangeMillis) >= 1000) {
    prevChangeMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      // Write a decimal point every other digit, for demo purposes.
      uint8_t pattern = PATTERNS[j] | ((j & 0x1) ? 0x80 : 0x00);
      ledModule.setPatternAt(i, pattern);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness. The TM1638 has 8 levels of brightness [0, 7].
    ledModule.setBrightness(brightness);
    incrementModOffset(brightness, (uint8_t) 8, (uint8_t) 0);
  }
}

// Every 100 ms, unconditionally flush() to the LED module which updates all
// digits, including brightness.
void flushModule() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;

    // Flush and measure the time.
    uint16_t startMicros = micros();
    ledModule.flush();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
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
}

void loop() {
  updateDisplay();
  flushModule();
  printStats();
}
