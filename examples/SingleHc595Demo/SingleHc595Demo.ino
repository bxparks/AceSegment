/*
 * A demo of a 4-digit LED module with segment pins connected to a 74HC595 shift
 * register and the digit pins connected directly to the microcontroller. Uses
 * the SingleHc595Module class.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // SingleHc595Module, LedDisplay

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_segment/hw/SwSpiFastInterface.h>
#include <ace_segment/hw/SwWireFastInterface.h>
#include <ace_segment/scanning/LedMatrixDirectFast4.h>
#endif

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_segment::LedMatrixBase;
using ace_segment::SwSpiFastInterface;
using ace_segment::SingleHc595Module;
using ace_segment::LedDisplay;

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1
#define LED_DISPLAY_TYPE_MAX7219 2
#define LED_DISPLAY_TYPE_HC595_DUAL 3
#define LED_DISPLAY_TYPE_HC595_SINGLE 4
#define LED_DISPLAY_TYPE_BARE 5

#if defined(EPOXY_DUINO)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595_SINGLE

#elif defined(AUNITER_LED_CLOCK_HC595_SINGLE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595_SINGLE

#else
  #error Unknown environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

// LED segment patterns.
const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;
const uint8_t LATCH_PIN = 10;
const uint8_t DATA_PIN = MOSI;
const uint8_t CLOCK_PIN = SCK;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

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
  15, 9, 5, 2
};

// Common Cathode, with transistors on Group pins
using SpiInterface = SwSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
SpiInterface spiInterface;
SingleHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
    spiInterface,
    LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
    LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
    FRAMES_PER_SECOND,
    DIGIT_PINS
);
LedDisplay display(ledModule);

// LedDisplay patterns
const uint8_t PATTERNS[NUM_DIGITS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
};

void setupAceSegment() {
  spiInterface.begin();
  ledModule.begin();
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

// loop() state variables
TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightnessIndex = 0;
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
  uint8_t brightness = BRIGHTNESS_LEVELS[brightnessIndex];
  display.setBrightness(brightness);
  incrementMod(brightnessIndex, NUM_BRIGHTNESSES);
}

void loop() {
  uint16_t nowMillis = millis();

  // Update the display with new pattern every second.
  if (nowMillis - prevUpdateMillis >= 1000) {
    prevUpdateMillis = nowMillis;
    updateDisplay();
  }

  // Use renderFieldWhenReady() to multiplex the digits in the LED module.
  uint16_t startMicros = micros();
  ledModule.renderFieldWhenReady();
  uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
  stats.update(elapsedMicros);

  // Print the stats every 5 seconds.
#if ENABLE_SERIAL_DEBUG >= 1
  if (nowMillis - prevStatsMillis >= 5000) {
    prevStatsMillis = nowMillis;
    Serial.print("ExpAvg:");
    Serial.println(stats.getExpDecayAvg());
  }
#endif
}
