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

// Select the TM1637Module flush() method
#define TM_FLUSH_METHOD_NORMAL 0
#define TM_FLUSH_METHOD_INCREMENTAL 1
#define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

// The TM1637 controller supports up to 6 digits.
const uint8_t PATTERNS[6] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
};

#if defined(AUNITER_MICRO_TM1637_DUAL) || defined(EPOXY_DUINO)
  const uint8_t CLK_PIN = A0;
  const uint8_t DIO1_PIN = 9;
  const uint8_t DIO2_PIN = 8;

  const uint8_t NUM_DIGITS = 4;
#else
  #error Unknown AUNITER environment
#endif

// For a SoftTmiInterface (non-fast), time to send 4 digits using flush():
//
// * 12 ms at 50 us delay, but does not work.
// * 17 ms at 75 us delay.
// * 22 ms at 100 us delay.
// * 43 ms at 200 us delay.
//
// Using flushIncremental() is about 1/2 these numbers.
const uint16_t BIT_DELAY = 100;

using TmiInterface = SoftTmiInterface;
TmiInterface tmiInterface1(CLK_PIN, DIO1_PIN, BIT_DELAY);
TmiInterface tmiInterface2(CLK_PIN, DIO2_PIN, BIT_DELAY);
Tm1637Module<TmiInterface, NUM_DIGITS> tm1637Module1(tmiInterface1);
Tm1637Module<TmiInterface, NUM_DIGITS> tm1637Module2(tmiInterface2);
LedDisplay display1(tm1637Module1);
LedDisplay display2(tm1637Module2);

void setupAceSegment() {
  tmiInterface1.begin();
  tmiInterface2.begin();
  tm1637Module1.begin();
  tm1637Module2.begin();
}

//----------------------------------------------------------------------------

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 1;

// Every second, scroll the display and change the brightness.
void updateDisplay() {
  static uint16_t prevChangeMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevChangeMillis) >= 1000) {
    prevChangeMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      display1.writePatternAt(i, PATTERNS[j]);
      display2.writePatternAt(i, PATTERNS[j]);

      // Write a decimal point every other digit, for demo purposes.
      display1.writeDecimalPointAt(i, j & 0x1);
      display2.writeDecimalPointAt(i, j & 0x1);

      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness, range from 1 to 7.
    display1.setBrightness(brightness);
    display2.setBrightness(brightness);
    incrementModOffset(brightness, (uint8_t) 7, (uint8_t) 1);
  }
}

// Every 20 ms, flushIncremental() to the LED module, which updates only a
// single digit per call, taking only ~10 ms using a 100 us delay. Each call to
// flushIncremental() updates only one digit, so to avoid making the incremental
// update distracting to the human eye, we need to call this somewhat rapidly.
// Every 20 ms seems to work pretty well.
//
// This version updates 2 TM1637 modules, for a total duration of about 20 ms.
// That should still work on an ESP8266. But if additional TM1637 modules are
// added to this, it might be necessary to add calls to yield() between calls to
// the flushIncremental() method.
void flushIncrementalModule() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 20) {
    prevFlushMillis = nowMillis;

    // Flush incrementally, and measure the time.
    uint16_t startMicros = micros();
    tm1637Module1.flushIncremental();
    tm1637Module2.flushIncremental();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
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
    tm1637Module1.flush();
    tm1637Module2.flush();
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

#if TM_FLUSH_METHOD == TM_FLUSH_METHOD_NORMAL
  flushModule();
#elif TM_FLUSH_METHOD == TM_FLUSH_METHOD_INCREMENTAL
  flushIncrementalModule();
#else
  #error Unknown TM_FLUSH_METHOD
#endif

  printStats();
}
