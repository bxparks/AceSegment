/*
 * A simple demo of a single TM1637 LED module, with the digits 0-3 to 0-5
 * scrolling to the left every second.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // Tm1636Module, LedDisplay

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using namespace ace_segment;

// Select driver version, either normal digitalWrite() or digitalWriteFast()
#define TM16137_DRIVER_TYPE_NORMAL 0
#define TM16137_DRIVER_TYPE_FAST 1
#define TM16137_DRIVER_TYPE TM16137_DRIVER_TYPE_FAST

#if TM16137_DRIVER_TYPE == TM16137_DRIVER_TYPE_FAST
  #include <digitalWriteFast.h>
  #include <ace_segment/tm1637/Tm1637DriverFast.h> // Tm1637DriverFast
  using ace_segment::Tm1637DriverFast;
#endif

//#if ! defined(AUNITER_LED_CLOCK_TM1637)
//#error Compatible only with env:ledclock_tm1636 configuration.
//#endif

const uint8_t CLK_PIN = 10;
const uint8_t DIO_PIN = 9;

#if defined(AUNITER_LED_CLOCK_TM1637) || defined(EPOXY_DUINO)
  const uint8_t NUM_DIGITS = 4;
  const uint8_t PATTERNS[NUM_DIGITS] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
  };
#elif defined(AUNITER_LED_CLOCK_TM1637_6)
  const uint8_t NUM_DIGITS = 6;
  const uint8_t PATTERNS[NUM_DIGITS] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
  };
#else
  #error Unknown AUNITER environment
#endif

// For a Tm1637Driver (non-fast), time to send 4 digits:
// * 12 ms at 50 us delay, but does not work.
// * 17 ms at 75 us delay.
// * 22 ms at 100 us delay.
// * 43 ms at 200 us delay.
constexpr uint16_t BIT_DELAY = 100;

#if TM16137_DRIVER_TYPE == TM16137_DRIVER_TYPE_NORMAL
  using Driver = Tm1637Driver;
  Driver driver(CLK_PIN, DIO_PIN, BIT_DELAY);
#elif TM16137_DRIVER_TYPE == TM16137_DRIVER_TYPE_FAST
  using Driver = Tm1637DriverFast<CLK_PIN, DIO_PIN, BIT_DELAY>;
  Driver driver;
#else
  #error Unknown TM16137_DRIVER_TYPE
#endif

#if defined(AUNITER_LED_CLOCK_TM1637) || defined(EPOXY_DUINO)
  const uint8_t* const remapArray = nullptr;
#elif defined(AUNITER_LED_CLOCK_TM1637_6)
  const uint8_t* const remapArray = ace_segment::kSixDigitRemapArray;
#endif

Tm1637Module<Driver, NUM_DIGITS> tm1637Module(driver, remapArray);
LedDisplay display(tm1637Module);

TimingStats stats;

uint8_t digitIndex = 0;
uint8_t brightness = 1;

void setup() {
  delay(1000);
#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  driver.begin();
  tm1637Module.begin();
}

#if 0

// This version of loop() uses the Tm1636Display.flush() method to update all
// digits in a single dump to the LED module, taking ~22ms per call.
void loop() {
  // Update the display
  uint8_t j = digitIndex;
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    display.setPatternAt(i, PATTERNS[j]);
    incrementMod(j, (uint8_t) NUM_DIGITS);
  }
  incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

  // Update the brightness
  display.setBrightness(brightness);
  incrementModOffset(brightness, (uint8_t) 7, (uint8_t) 1);

  // Flush the change to the LED Module, and measure the time.
  uint16_t startMicros = micros();
  tm1637Module.flush();
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
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      display.writePatternAt(i, PATTERNS[j]);
      // Write a decimal point every other
      display.writeDecimalPointAt(i, j & 0x1);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness
    display.setBrightness(brightness);
    incrementModOffset(brightness, (uint8_t) 7, (uint8_t) 1);
  }

  // Every 10 ms, incrementally flush() to the LED module.
  if (nowMillis - prevFlushMillis > 10) {
    prevFlushMillis = nowMillis;

    // Flush incrementally, and measure the time.
    uint16_t startMicros = micros();
    tm1637Module.flushIncremental();
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
