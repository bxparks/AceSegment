/*
 * Demo of 2 x TM1637 LED modules, sharing a common CLK but using 2 different
 * DIO pins.
 *
 * Results: Works great except at high levels of brightness, there is a
 * noticeable flicker of the LEDs. My guess is that the LEDs are drawing more
 * current than the Arduino MCU can provide stably, so the voltage fluctuates. I
 * think the solution is to wire the LED Modules directly to the USB 5V, instead
 * of being powered through the Arduino Vcc.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // Tm1637Display

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using namespace ace_segment;

const uint8_t CLK_PIN = 10;
const uint8_t DIO1_PIN = 9;
const uint8_t DIO2_PIN = 8;

#if defined(AUNITER_LED_CLOCK_TM1637_DUAL) || defined(EPOXY_DUINO)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t PATTERNS[NUM_DIGITS] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
  };
#else
  #error Unknown AUNITER environment
#endif

// For a SoftWireInterface (non-fast), time to send 4 digits using flush():
//
// * 12 ms at 50 us delay, but does not work.
// * 17 ms at 75 us delay.
// * 22 ms at 100 us delay.
// * 43 ms at 200 us delay.
//
// Using flushIncremental() is about 1/2 these numbers.
constexpr uint16_t BIT_DELAY = 100;

using WireInterface = SoftWireInterface;
WireInterface wireInterface1(CLK_PIN, DIO1_PIN, BIT_DELAY);
WireInterface wireInterface2(CLK_PIN, DIO2_PIN, BIT_DELAY);
Tm1637Module<WireInterface, NUM_DIGITS> module1(wireInterface1);
Tm1637Module<WireInterface, NUM_DIGITS> module2(wireInterface2);
LedDisplay display1(module1);
LedDisplay display2(module2);

TimingStats stats;

uint8_t digitIndex = 0;
uint8_t brightness = 1;

void setup() {
  delay(1000);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  wireInterface1.begin();
  wireInterface2.begin();
  module1.begin();
  module2.begin();
}

#if 0

// This version of loop() uses the Tm1636Display.flush() method to update all
// digits in a single dump to the LED module, taking ~22ms per call.
void loop() {
  // Update the display
  uint8_t j = digitIndex;
  uint8_t k = j;
  incrementMod(k, (uint8_t) NUM_DIGITS);
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    display1.writePatternAt(i, PATTERNS[j]);
    display2.writePatternAt(i, PATTERNS[k]);
    incrementMod(j, (uint8_t) NUM_DIGITS);
    incrementMod(k, (uint8_t) NUM_DIGITS);
  }
  incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

  // Update the brightness
  display1.setBrightness(brightness);
  display2.setBrightness(brightness);
  incrementModOffset(brightness, (uint8_t) 7, (uint8_t) 1);

  // Flush the change to the LED Module, and measure the time.
  uint16_t startMicros = micros();
  module1.flush();
  module2.flush();
  uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
  stats.update(elapsedMicros);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.print("ExpAvg:");
  Serial.println(stats.getExpDecayAvg());
#endif

  delay(1000);
}

#else

// This version of loop() uses the Tm1636Display.flushIncremental() method to
// update only a single digit per call, taking only ~10 ms at 100 us delay.
void loop() {
  static uint16_t prevChangeMillis = millis();
  static uint16_t prevFlushMillis = millis();
  static uint16_t prevStatsMillis = millis();

  // Every second, change the brightness and scroll the display.
  uint16_t nowMillis = millis();
  if (nowMillis - prevChangeMillis > 1000) {
    prevChangeMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    uint8_t k = j;
    incrementMod(k, (uint8_t) NUM_DIGITS);
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      display1.writePatternAt(i, PATTERNS[j]);
      display2.writePatternAt(i, PATTERNS[k]);

      // Write a decimal point every other digit.
      display1.writeDecimalPointAt(i, j & 0x1);
      display2.writeDecimalPointAt(i, k & 0x1);
      incrementMod(j, (uint8_t) NUM_DIGITS);
      incrementMod(k, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness
    display1.setBrightness(brightness);
    display2.setBrightness(brightness);
    incrementModOffset(brightness, (uint8_t) 7, (uint8_t) 1);
  }

  // Every 10 ms, incrementally flush() to the LED module.
  if (nowMillis - prevFlushMillis > 10) {
    prevFlushMillis = nowMillis;

    // Flush incrementally, and measure the time.
    uint16_t startMicros = micros();
    module1.flushIncremental();
    module2.flushIncremental();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
  }

#if ENABLE_SERIAL_DEBUG >= 1
  // Every 5 seconds, print out the statistics.
  if (nowMillis - prevStatsMillis > 5000) {
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

#endif
