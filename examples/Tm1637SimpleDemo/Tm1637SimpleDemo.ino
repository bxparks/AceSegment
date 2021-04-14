#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // Tm1637Display

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_segment::Tm1637Display;

//#if ! defined(AUNITER_LED_CLOCK_TM1637)
//#error Compatible only with env:ledclock_tm1636 configuration.
//#endif

const uint8_t CLK_PIN = 16;
const uint8_t DIO_PIN = 10;

const uint8_t PATTERNS[4] = {
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
};

// Takes about 12 ms to send 4 digits at 50 us delay, but does not work.
// Takes about 17 ms to send 4 digits at 75 us delay.
// Takes about 22 ms to send 4 digits at 100 us delay.
// Takes about 43 ms to send 4 digits at 200 us delay.
const uint16_t bitDelay = 100;
Tm1637Display<4> display(CLK_PIN, DIO_PIN, bitDelay);

TimingStats stats;

uint8_t digitIndex = 0;
uint8_t brightness = 1;

void setup() {
  delay(1000);
#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  display.begin();
}

#if 0

void loop() {
  // Update the display
  uint8_t j = digitIndex;
  for (uint8_t i = 0; i < 4; ++i) {
    display.setPatternAt(i, PATTERNS[j]);
    incrementMod(j, (uint8_t) 4);
  }
  incrementMod(digitIndex, (uint8_t) 4);

  // Update the brightness
  display.setBrightness(brightness);
  incrementModOffset(brightness, (uint8_t) 7, (uint8_t) 1);

  // Flush the change to the LED Module, and measure the time.
  uint16_t startMicros = micros();
  display.flush();
  uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
  stats.update(elapsedMicros);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.print("ExpAvg:");
  Serial.println(stats.getExpDecayAvg());
#endif

  delay(1000);
}

#else

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
    for (uint8_t i = 0; i < 4; ++i) {
      display.writePatternAt(i, PATTERNS[j]);
      incrementMod(j, (uint8_t) 4);
    }
    incrementMod(digitIndex, (uint8_t) 4);

    // Update the brightness
    display.setBrightness(brightness);
    incrementModOffset(brightness, (uint8_t) 7, (uint8_t) 1);
  }

  // Every 10 ms, incrementally flush() to the LED module.
  if (nowMillis - prevFlushMillis > 10) {
    prevFlushMillis = nowMillis;

    // Flush incrementally, and measure the time.
    uint16_t startMicros = micros();
    display.flushIncremental();
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
