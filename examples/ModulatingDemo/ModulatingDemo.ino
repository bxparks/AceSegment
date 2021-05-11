/*
 * Demo the ability to control the brightness of each digit separately. Requires
 * one of the following configurations:
 *
 *  * ScanningModule + LedMatrixDirect
 *  * ScanningModule + LedMatrixDirectFast4
 *  * ScanningModule + LedMatrixSingleHc595
 *  * ScanningModule + LedMatrixDualHc595
 *  * DirectModule
 *  * DirectFast4Module
 *  * SingleHc595Module
 *  * DualHc595Module
 */
#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_segment/hw/SoftSpiFastInterface.h>
#include <ace_segment/hw/HardSpiFastInterface.h>
#include <ace_segment/scanning/LedMatrixDirectFast4.h>
#endif

using ace_common::incrementMod;
using namespace ace_segment;

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

//------------------------------------------------------------------
// Hardware environment configuration.
//------------------------------------------------------------------

#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_DIRECT 3
#define LED_DISPLAY_TYPE_HC595_SINGLE 4
#define LED_DISPLAY_TYPE_HC595_DUAL 5

// Used by LED_DISPLAY_TYPE_SCANNING
#define LED_MATRIX_MODE_NONE 0
#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_DIRECT_FAST 2
#define LED_MATRIX_MODE_SINGLE_SOFT_SPI 3
#define LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST 4
#define LED_MATRIX_MODE_SINGLE_HARD_SPI 5
#define LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST 6
#define LED_MATRIX_MODE_DUAL_SOFT_SPI 7
#define LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST 8
#define LED_MATRIX_MODE_DUAL_HARD_SPI 9
#define LED_MATRIX_MODE_DUAL_HARD_SPI_FAST 10

// Used by LED_DISPLAY_TYPE_DIRECT
#define DIRECT_INTERFACE_TYPE_NORMAL 0
#define DIRECT_INTERFACE_TYPE_FAST_4 1

// Used by LED_DISPLAY_TYPE_HC595_SINGLE and LED_DISPLAY_TYPE_HC595_DUAL
#define INTERFACE_TYPE_SOFT_SPI 0
#define INTERFACE_TYPE_SOFT_SPI_FAST 1
#define INTERFACE_TYPE_HARD_SPI 2
#define INTERFACE_TYPE_HARD_SPI_FAST 3

const uint8_t NUM_SEGMENTS = 8;

#if defined(EPOXY_DUINO)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI_FAST

#elif defined(AUNITER_MICRO_SCANNING_DIRECT)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

#elif defined(AUNITER_MICRO_SCANNING_SINGLE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST

#elif defined(AUNITER_MICRO_SCANNING_DUAL)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  const uint8_t NUM_DIGITS = 4;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t HC595_SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t HC595_DIGIT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;

  // Choose one of the following variants:
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HARD_SPI_FAST

#elif defined(AUNITER_MICRO_CUSTOM_DIRECT)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_DIRECT
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_NORMAL
  #define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_FAST

#elif defined(AUNITER_MICRO_CUSTOM_SINGLE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595_SINGLE
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST

#elif defined(AUNITER_MICRO_CUSTOM_DUAL)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595_DUAL
  const uint8_t NUM_DIGITS = 4;
  const uint8_t HC595_BYTE_ORDER = kByteOrderDigitHighSegmentLow;
  const uint8_t HC595_SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t HC595_DIGIT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST

#elif defined(AUNITER_MICRO_HC595)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595_DUAL
  const uint8_t NUM_DIGITS = 8;
  const uint8_t HC595_BYTE_ORDER = kByteOrderSegmentHighDigitLow;
  const uint8_t HC595_SEGMENT_ON_PATTERN = LedMatrixBase::kActiveLowPattern;
  const uint8_t HC595_DIGIT_ON_PATTERN = LedMatrixBase::kActiveHighPattern;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SOFT_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST

#endif

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

// Total fields/second
//      = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//      = 60 * 16 * 4
//      = 3840 fields/sec
//      => 260 micros/field
//
// According to AutoBenchmark, almost all versions of ScanningModule with
// various LedMatrix can render a single field in less than this on 16 MHz AVR
// processor. The combination of (ScanningModule + LedMatrixDualHc595 +
// SoftSpiInterface) is the exception.
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t NUM_BRIGHTNESSES = 8;
const uint8_t BRIGHTNESS_LEVELS[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 7, 3, 2
};

// Define GPIO pins.
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
    // 4 digits, resistors on segments on Pro Micro.
    const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
    const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

  #else
    const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
    const uint8_t LATCH_PIN = A0; // ST_CP on 74HC595
    const uint8_t DATA_PIN = MOSI; // DS on 74HC595
    const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595
  #endif

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_SINGLE
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t LATCH_PIN = A0;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_DUAL
  const uint8_t LATCH_PIN = A0;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#endif

// The chain of resources.
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
    // Common Anode, with transitors on Group pins
    using LedMatrix = LedMatrixDirect<>;
    LedMatrix ledMatrix(
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        NUM_SEGMENTS,
        SEGMENT_PINS,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT_FAST
    // Common Anode, with transitors on Group pins
    using LedMatrix = LedMatrixDirectFast4<
      8, 9, 10, 16, 14, 18, 19, 15,
      4, 5, 6, 7
    >;
    LedMatrix ledMatrix(
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI
    // Common Cathode, with transistors on Group pins
    SoftSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixSingleHc595<SoftSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS):
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST
    // Common Cathode, with transistors on Group pins
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI
    // Common Cathode, with transistors on Group pins
    HardSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixSingleHc595<HardSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST
    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixSingleHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI
    // Common Anode, with transistors on Group pins
    SoftSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualHc595<SoftSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        HC595_BYTE_ORDER);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST
    // Common Anode, with transistors on Group pins
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixDualHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        HC595_BYTE_ORDER);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI
    // Common Anode, with transistors on Group pins
    HardSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualHc595<HardSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        HC595_BYTE_ORDER);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
    // Common Anode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixDualHc595<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*elementOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*groupOnPattern*/,
        HC595_BYTE_ORDER);

  #else
    #error Unsupported LED_MATRIX_MODE
  #endif

  // 16 levels of brightness, need render-fields/second of 60*4*16 = 3840.
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      ledModule(ledMatrix, FRAMES_PER_SECOND);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  // Common Anode, with transitors on Group pins
  #if DIRECT_INTERFACE_TYPE == DIRECT_INTERFACE_TYPE_NORMAL
    DirectModule<NUM_DIGITS, NUM_SUBFIELDS> ledModule(
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        SEGMENT_PINS,
        DIGIT_PINS);
  #else
    DirectFast4Module<
        8, 9, 10, 16, 14, 18, 19, 15, // segment pins
        4, 5, 6, 7, // digit pins
        NUM_DIGITS,
        NUM_SUBFIELDS
    > ledModule(
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND);
  #endif

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_SINGLE
  // Common Cathode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI_FAST
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #endif
  SingleHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_DUAL
  // Common Anode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SOFT_SPI_FAST
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #endif
  DualHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> ledModule(
      spiInterface,
      HC595_SEGMENT_ON_PATTERN,
      HC595_DIGIT_ON_PATTERN,
      FRAMES_PER_SECOND,
      HC595_BYTE_ORDER
  );

#endif

// Setup the various resources.
void setupAceSegment() {

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SOFT_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HARD_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SOFT_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HARD_SPI_FAST
    spiInterface.begin();
  #endif

  ledMatrix.begin();
  ledModule.begin();

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_DUAL
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_SINGLE
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#endif

#if USE_INTERRUPT == 1
  setupInterupt(ledDisplay.getFieldsPerSecond());
#endif
}

#if USE_INTERRUPT == 1
void setupInterupt(uint16_t fieldsPerSecond) {
  // set up Timer 2
  uint8_t timerCompareValue = (long) F_CPU / 1024 / fieldsPerSecond - 1;
  if (ENABLE_SERIAL_DEBUG >= 1) {
    Serial.print(F("Timer 2, Compare A: "));
    Serial.println(timerCompareValue);
  }

  noInterrupts();
  TCNT2  = 0;	// Initialize counter value to 0
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= bit(WGM21); // CTC
  TCCR2B |= bit(CS22) | bit(CS21) | bit(CS20); // prescale 1024
  TIMSK2 |= bit(OCIE2A); // interrupt on Compare A Match
  OCR2A =  timerCompareValue;
  interrupts();
}
#endif

//------------------------------------------------------------------
// Configurations for ModulationDemo
//------------------------------------------------------------------

void setupPulseDisplay() {
  LedDisplay ledDisplay(ledModule);
  NumberWriter numberWriter(ledDisplay);
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    numberWriter.writeHexCharAt(i, i);
  }
}

void setBrightnesses(int brightnessIndex) {
  for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
    uint8_t brightness = BRIGHTNESS_LEVELS[
        (brightnessIndex + i) % NUM_BRIGHTNESSES];
    ledModule.setBrightnessAt(i, brightness);
  }
}

void pulseDisplay() {
  static uint8_t brightnessIndex = 0;
  static unsigned long lastUpdateTime = millis();

  unsigned long now = millis();
  if (now - lastUpdateTime > 200) {
    lastUpdateTime = now;
    incrementMod(brightnessIndex, NUM_BRIGHTNESSES);
    setBrightnesses(brightnessIndex);
  }
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  if (ENABLE_SERIAL_DEBUG >= 1) {
    Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
    while (!Serial); // Wait until Serial is ready - Leonardo/Micro
    Serial.println(F("setup(): begin"));
  }

  setupAceSegment();
  setupPulseDisplay();

  if (ENABLE_SERIAL_DEBUG >= 1) {
    Serial.println(F("setup(): end"));
  }
}

void loop() {
  pulseDisplay();

  #if USE_INTERRUPT == 0
    ledModule.renderFieldWhenReady();
  #endif
}
