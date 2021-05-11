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
#define WIRE_INTERFACE_TYPE_NORMAL 0
#define WIRE_INTERFACE_TYPE_FAST 1

// Select the TM1637Module flush() method
#define TM_FLUSH_METHOD_FLUSH 0
#define TM_FLUSH_METHOD_FLUSH_INCREMENTAL 1

#if defined(EPOXY_DUINO)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_FLUSH

  const uint8_t CLK_PIN = A0;
  const uint8_t DIO_PIN = 9;

  const uint8_t NUM_DIGITS = 4;
  const uint8_t PATTERNS[NUM_DIGITS] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
  };

#elif defined(AUNITER_MICRO_TM1637)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_FAST
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_FLUSH_INCREMENTAL

  const uint8_t CLK_PIN = A0;
  const uint8_t DIO_PIN = 9;

  const uint8_t NUM_DIGITS = 4;
  const uint8_t PATTERNS[NUM_DIGITS] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
  };

#elif defined(AUNITER_MICRO_TM1637_6)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_FAST
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_FLUSH_INCREMENTAL

  const uint8_t CLK_PIN = A0;
  const uint8_t DIO_PIN = 9;

  const uint8_t NUM_DIGITS = 6;
  const uint8_t PATTERNS[NUM_DIGITS] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
  };

#elif defined(AUNITER_STM32_TM1637)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_FLUSH

  const uint8_t CLK_PIN = PB3;
  const uint8_t DIO_PIN = PB4;

  const uint8_t NUM_DIGITS = 4;
  const uint8_t PATTERNS[NUM_DIGITS] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
  };

#elif defined(AUNITER_D1MINI_LARGE_TM1637)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_FLUSH_INCREMENTAL

  const uint8_t CLK_PIN = D5;
  const uint8_t DIO_PIN = D7;

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

// For a SoftWireInterface (non-fast), time to send 4 digits:
// * 12 ms at 50 us delay, but does not work.
// * 17 ms at 75 us delay.
// * 22 ms at 100 us delay.
// * 43 ms at 200 us delay.
const uint16_t BIT_DELAY = 100;

#if WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_NORMAL
  using WireInterface = SoftWireInterface;
  WireInterface wireInterface(CLK_PIN, DIO_PIN, BIT_DELAY);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_FAST
  #include <digitalWriteFast.h>
  #include <ace_segment/hw/SoftWireFastInterface.h>
  using ace_segment::SoftWireFastInterface;

  using WireInterface = SoftWireFastInterface<CLK_PIN, DIO_PIN, BIT_DELAY>;
  WireInterface wireInterface;
#else
  #error Unknown WIRE_INTERFACE_TYPE
#endif

#if defined(AUNITER_MICRO_TM1637_6)
  const uint8_t* const remapArray = ace_segment::kDigitRemapArray6Tm1637;
#else
  const uint8_t* const remapArray = nullptr;
#endif

Tm1637Module<WireInterface, NUM_DIGITS> tm1637Module(wireInterface, remapArray);
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

  wireInterface.begin();
  tm1637Module.begin();
}

#if TM_FLUSH_METHOD == TM_FLUSH_METHOD_FLUSH

// This version of loop() uses the Tm1637Module::flush() method to update all
// digits in a single dump to the LED module, taking ~22ms per call.
void loop() {
  // Update the display
  uint8_t j = digitIndex;
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    display.writePatternAt(i, PATTERNS[j]);
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

#elif TM_FLUSH_METHOD == TM_FLUSH_METHOD_FLUSH_INCREMENTAL

// This version of loop() uses the Tm1637Module::flushIncremental() method to
// update only a single digit per call, taking only ~10 ms at 100 us delay.
void loop() {
  static uint16_t prevChangeMillis = millis();
  static uint16_t prevFlushMillis = millis();
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevStatsMillis = millis();
#endif

  // Every second, change the brightness and scroll the display.
  uint16_t nowMillis = millis();
  if (nowMillis - prevChangeMillis > 1000) {
    prevChangeMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      display.writePatternAt(i, PATTERNS[j]);
      // Write a decimal point every other digit, for demo purposes.
      display.writeDecimalPointAt(i, j & 0x1);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness, range from 1 to 7.
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

#else
#error Unknown TM_FLUSH_METHOD

#endif
