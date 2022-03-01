/*
 * A demo of a 6-segment TM1637 LED module with 6 buttons. The button value is
 * read through Tm1637Module::readButtons(), and the value is displayed on the
 * LED.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_TM1637_6B: SparkFun Pro Micro + 6-digit LED module
 *  * AUNITER_STM32_TM1637_6B: STM32 F1 Blue Pill + 6-digit LED module
 *  * AUNITER_D1MINI_LARGE_TM1637_6B: WeMos D1 Mini ESP8266
 *  * AUNITER_ESP32_TM1637_6B: ESP32 Dev Kit v1
 *
 * The expected display when each button is pressed is the following, going from
 * left most button (0) to right (5):
 *
 *  * No buttons: "FF-0-0"
 *  * Button 0: "F7-1-0"
 *  * Button 1: "F6-1-1"
 *  * Button 2: "F5-1-2"
 *  * Button 3: "F4-1-3"
 *  * Button 4: "F3-1-4"
 *  * Button 5: "F2-1-5"
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceTMI.h>
#include <AceSegment.h> // Tm1637Module

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_tmi::SimpleTmi1637Interface;
using ace_segment::LedModule;
using ace_segment::Tm1637Module;
using ace_segment::kDigitRemapArray6Tm1637;

// Select TM1637 protocol version, either SimpleTmi1637Interface or
// SimpleTmi1637FastInterface.
#define TMI_INTERFACE_TYPE_NORMAL 0
#define TMI_INTERFACE_TYPE_FAST 1

// Select the TM1637Module flush() method.
#define TM_FLUSH_METHOD_NORMAL 0
#define TM_FLUSH_METHOD_INCREMENTAL 1

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_TM1637
#endif

#if defined(EPOXY_DUINO)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  const uint8_t CLK_PIN = A0;
  const uint8_t DIO_PIN = 9;
  const uint8_t NUM_DIGITS = 4;

#elif defined(AUNITER_MICRO_TM1637_6B)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_FAST

  const uint8_t CLK_PIN = A0;
  const uint8_t DIO_PIN = 9;
  const uint8_t NUM_DIGITS = 6;

#elif defined(AUNITER_STM32_TM1637_6B)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  const uint8_t CLK_PIN = PB3;
  const uint8_t DIO_PIN = PB4;
  const uint8_t NUM_DIGITS = 6;

#elif defined(AUNITER_D1MINI_LARGE_TM1637_6B)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  const uint8_t CLK_PIN = D5;
  const uint8_t DIO_PIN = D7;
  const uint8_t NUM_DIGITS = 6;

#elif defined(AUNITER_ESP32_TM1637_6B)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL

  const uint8_t CLK_PIN = 14;
  const uint8_t DIO_PIN = 13;
  const uint8_t NUM_DIGITS = 6;

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

// Some amount delay is necessary when using the digitalWriteFast versions
// probably because of capacitance on the DIO and CLK lines.
const uint8_t DELAY_MICROS = 5;

#if TMI_INTERFACE_TYPE == TMI_INTERFACE_TYPE_NORMAL
  using TmiInterface = SimpleTmi1637Interface;
  TmiInterface tmiInterface(DIO_PIN, CLK_PIN, DELAY_MICROS);
#elif TMI_INTERFACE_TYPE == TMI_INTERFACE_TYPE_FAST
  #include <digitalWriteFast.h>
  #include <ace_tmi/SimpleTmi1637FastInterface.h>
  using ace_tmi::SimpleTmi1637FastInterface;

  using TmiInterface = SimpleTmi1637FastInterface<
      DIO_PIN, CLK_PIN, DELAY_MICROS>;
  TmiInterface tmiInterface;
#else
  #error Unknown TMI_INTERFACE_TYPE
#endif

Tm1637Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);

void setupAceSegment() {
  tmiInterface.begin();
  ledModule.begin();
}

//----------------------------------------------------------------------------

// Patterns for the 16 hexadecimal digits.
const uint8_t PATTERNS[16] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, /* 8 */
  0b01101111, /* 9 */
  0b01110111, /* A */
  0b01111100, /* b */
  0b00111001, /* C */
  0b01011110, /* d */
  0b01111001, /* E */
  0b01110001, /* F */
};

// Just a minum "-" sign.
const uint8_t DASH_PATTERN = 0b01000000;

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 0; // [0, 7] with 0 being dimmest

// Decode hexadecimal
static void decodeHex(uint8_t data, uint8_t* hi, uint8_t* lo) {
  *hi = (data & 0xf0) >> 4;
  *lo = (data & 0x0f);
}

// Decode the button code. The raw button data is inverted (i.e. 0 when a
// button is pressed). So we invert the bit pattern.
static void decodeButton(uint8_t data, uint8_t* kn, uint8_t* sn) {
  data = ~data;
  *kn = (data & 0b00011000) >> 3;
  *sn = data & 0b00000111;
}

// Every 100 ms, read the button, and the display its value.
void updateDisplay() {
  static uint16_t prevChangeMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevChangeMillis) >= 100) {
    prevChangeMillis = nowMillis;

    // Read the button and decode the kn and sn.
    uint8_t data = ledModule.readButtons();
    uint8_t kn, sn, hi, lo;
    decodeHex(data, &hi, &lo);
    decodeButton(data, &kn, &sn);

    // Display the button result.
    ledModule.setPatternAt(0, PATTERNS[hi]);
    ledModule.setPatternAt(1, PATTERNS[lo]);
    ledModule.setPatternAt(2, DASH_PATTERN);
    ledModule.setPatternAt(3, PATTERNS[kn]);
    ledModule.setPatternAt(4, DASH_PATTERN);
    ledModule.setPatternAt(5, PATTERNS[sn]);

    // Update the brightness. The TM1637 has 8 levels of brightness [0, 7].
    ledModule.setBrightness(brightness);
  }
}

// Every 100 ms, unconditionally flush() to the LED module which updates all
// digits, including brightness, taking about 22 ms (4 digits) to 28 ms
// (6-digits) using a 100 us delay.
void flushModule() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;

    // Flush incrementally, and measure the time.
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
