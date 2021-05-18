/*
 * A demo of a 4-digit bare LED module with digit and segment pins connected
 * directly to the microcontroller. Uses ScanningModule and LedMatrixDirect
  * and configures them manually, instead of using DirectModule.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // ScanningModule, LedDisplay

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_segment/hw/SoftSpiFastInterface.h>
#include <ace_segment/hw/SoftWireFastInterface.h>
#include <ace_segment/scanning/LedMatrixDirectFast4.h>
#endif

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_segment::LedMatrixDirect;
using ace_segment::LedMatrixDirectFast4;
using ace_segment::ScanningModule;
using ace_segment::LedDisplay;

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// LedMatrix wiring modes.
#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_DIRECT_FAST 6

#if defined(EPOXY_DUINO)
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

#elif defined(AUNITER_MICRO_DIRECT)
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

#else
  #error Unknown environment
#endif

// LED segment patterns.
const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;
// Pin numbers
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

// Total fields/second
//    = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//    = 60 * 16 * 4
//    = 3840 fields/sec
//    => 260 micros/field
//
// Fortunately, according to AutoBenchmark, the "fast" versions of LedMatrix can
// render a single field in about 20-30 micros.
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 7, 3, 2
};

// Common Anode, with transitors on Group pins
#if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
  using LedMatrix = LedMatrixDirect<>;
  LedMatrix ledMatrix(
      LedMatrix::kActiveLowPattern /*elementOnPattern*/,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      NUM_SEGMENTS,
      SEGMENT_PINS,
      NUM_DIGITS,
      DIGIT_PINS);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT_FAST
  using LedMatrix = LedMatrixDirectFast4<
    8, 9, 10, 16, 14, 18, 19, 15,
    4, 5, 6, 7
  >;
  LedMatrix ledMatrix(
      LedMatrix::kActiveLowPattern /*elementOnPattern*/,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/);
#endif

ScanningModule<LedMatrix, NUM_DIGITS> ledModule(ledMatrix, FRAMES_PER_SECOND);
LedDisplay display(ledModule);

// LedDisplay patterns
const uint8_t PATTERNS[NUM_DIGITS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
};

void setupAceSegment() {
  ledMatrix.begin();
  ledModule.begin();
}

//----------------------------------------------------------------------------

// loop() state variables
TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 1;
uint16_t prevUpdateMillis = 0;

#if ENABLE_SERIAL_DEBUG >= 1
uint16_t prevStatsMillis = 0;
#endif

void updateDisplay() {
  // Update the display
  uint8_t j = digitIndex;
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    display.writePatternAt(i, PATTERNS[j]);
    // Write a decimal point every other digit, for demo purposes.
    display.writeDecimalPointAt(i, j & 0x1);
    incrementMod(j, (uint8_t) NUM_DIGITS);
  }
  incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

  // Update the brightness
  display.setBrightness(brightness);
  incrementMod(brightness, NUM_BRIGHTNESSES);
}

// Call renderFieldWhenReady() as fast as possible. It uses an internal timer to
// do the actual rendering when ready. Limit timing samples to every 10 ms to
// limit number of samples over 5 seconds to less than UINT16_MAX (i.e. 65535).
void flushModule() {
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;

    // Flush the change to the LED Module, and measure the time.
    uint16_t startMicros = micros();
    ledModule.renderFieldWhenReady();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
  } else {
    ledModule.renderFieldWhenReady();
  }
#else
    ledModule.renderFieldWhenReady();
#endif
}

// Every 5 seconds, print stats about how long flush() took.
void printStats() {
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevStatsMillis;

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

//----------------------------------------------------------------------------

void setup() {
  delay(1000);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  setupAceSegment();
}

// Use the Max7219Module::flush() method to update all digits in a single dump
// to the LED module, taking about ~170 microseconds per flush().
void loop() {
  updateDisplay();
  flushModule();
  printStats();
}
