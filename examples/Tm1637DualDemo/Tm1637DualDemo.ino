/*
 * A demo of *two* TM1637 LED modules, sharing a common CLK line but using 2
 * different DIO pins. Displays the digits 0 - 3, then slowly rotates the digits
 * to the left, while changing the brightness of the display.
 *
 * Results: Works great except at high levels of brightness, there is a
 * noticeable flicker of the LEDs. My guess is that the LEDs are drawing more
 * current than the Arduino MCU can provide stably, so the voltage fluctuates.
 * Maybe I need to add a smoothing capacitor across the power line? Not tested.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_MICRO_TM1637_DUAL: SparkFun Pro Micro + two 4-digit LED modules
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceTMI.h>
#include <AceSegment.h> // Tm1637Display

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_tmi::SimpleTmi1637Interface;
using ace_segment::LedModule;
using ace_segment::Tm1637Module;

// Select TM1637 protocol version, either SimpleTmi1637Interface or
// SimpleTmi1637FastInterface.
#define TMI_INTERFACE_TYPE_NORMAL 0
#define TMI_INTERFACE_TYPE_FAST 1

// Select the TM1637Module flush() method
#define TM_FLUSH_METHOD_NORMAL 0
#define TM_FLUSH_METHOD_INCREMENTAL 1

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_TM1637_DUAL
#endif

#if defined(EPOXY_DUINO)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_FAST
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

  const uint8_t CLK_PIN = A0;
  const uint8_t DIO1_PIN = 9;
  const uint8_t DIO2_PIN = 8;
  const uint8_t NUM_DIGITS = 4;

#elif defined(AUNITER_MICRO_TM1637_DUAL)
  #define TMI_INTERFACE_TYPE TMI_INTERFACE_TYPE_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

  const uint8_t CLK_PIN = A0;
  const uint8_t DIO1_PIN = 9;
  const uint8_t DIO2_PIN = 8;
  const uint8_t NUM_DIGITS = 4;

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

// For a SimpleTmi1637Interface (non-fast), time needed to send 4 digits using
// flush():
//
// * 12 ms at 50 us delay, but does not work.
// * 17 ms at 75 us delay.
// * 22 ms at 100 us delay.
// * 43 ms at 200 us delay.
//
// Using flushIncremental(), the duration is about 1/2 these numbers.
const uint8_t DELAY_MICROS = 100;

#if TMI_INTERFACE_TYPE == TMI_INTERFACE_TYPE_NORMAL
  using TmiInterface = SimpleTmi1637Interface;
  TmiInterface tmiInterface1(DIO1_PIN, CLK_PIN, DELAY_MICROS);
  TmiInterface tmiInterface2(DIO2_PIN, CLK_PIN, DELAY_MICROS);
  Tm1637Module<TmiInterface, NUM_DIGITS> ledModule1(tmiInterface1);
  Tm1637Module<TmiInterface, NUM_DIGITS> ledModule2(tmiInterface2);

#elif TMI_INTERFACE_TYPE == TMI_INTERFACE_TYPE_FAST
  #include <digitalWriteFast.h>
  #include <ace_tmi/SimpleTmi1637FastInterface.h>
  using ace_tmi::SimpleTmi1637FastInterface;
  using TmiInterface1 = SimpleTmi1637FastInterface<
      DIO1_PIN, CLK_PIN, DELAY_MICROS>;
  using TmiInterface2 = SimpleTmi1637FastInterface<
      DIO2_PIN, CLK_PIN, DELAY_MICROS>;
  TmiInterface1 tmiInterface1;
  TmiInterface2 tmiInterface2;
  Tm1637Module<TmiInterface1, NUM_DIGITS> ledModule1(tmiInterface1);
  Tm1637Module<TmiInterface2, NUM_DIGITS> ledModule2(tmiInterface2);

#else
  #error Unknown TMI_INTERFACE_TYPE
#endif

void setupAceSegment() {
  tmiInterface1.begin();
  tmiInterface2.begin();
  ledModule1.begin();
  ledModule2.begin();
}

//----------------------------------------------------------------------------

// The TM1637 controller supports up to 6 digits.
const uint8_t PATTERNS[6] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
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
      ledModule1.setPatternAt(i, pattern);
      ledModule2.setPatternAt(i, pattern);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness. The TM1637 has 8 levels of brightness [0, 7].
    ledModule1.setBrightness(brightness);
    ledModule2.setBrightness(brightness);
    incrementModOffset(brightness, (uint8_t) 8, (uint8_t) 0);
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
    ledModule1.flushIncremental();
    ledModule2.flushIncremental();
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
    ledModule1.flush();
    ledModule2.flush();
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
