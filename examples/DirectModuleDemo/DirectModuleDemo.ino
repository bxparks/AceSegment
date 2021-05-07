/*
 * A demo of a single, 4-digit, bare LED module with digit and segment pins
 * connected directly to the microcontroller.
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h> // DirectModule, LedDisplay

using ace_common::incrementMod;
using ace_common::TimingStats;
using ace_segment::LedMatrixBase;
using ace_segment::DirectModule;
using ace_segment::LedDisplay;

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1
#define LED_DISPLAY_TYPE_MAX7219 2
#define LED_DISPLAY_TYPE_HC595_DUAL 3
#define LED_DISPLAY_TYPE_HC595_SINGLE 4
#define LED_DISPLAY_TYPE_DIRECT 5

#if defined(EPOXY_DUINO)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_DIRECT

#elif defined(AUNITER_LED_CLOCK_DIRECT)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_DIRECT

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
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 7, 3, 2
};

// Common Anode, with transitors on Group pins
DirectModule<NUM_DIGITS, NUM_SUBFIELDS> ledModule(
    LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
    LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
    FRAMES_PER_SECOND,
    SEGMENT_PINS,
    DIGIT_PINS);

LedDisplay display(ledModule);

// LedDisplay patterns
const uint8_t PATTERNS[NUM_DIGITS] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
};

void setupAceSegment() {
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