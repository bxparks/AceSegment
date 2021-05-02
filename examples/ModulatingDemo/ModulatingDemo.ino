#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_segment/hw/SwSpiFastInterface.h>
#include <ace_segment/hw/SwWireFastInterface.h>
#include <ace_segment/scanning/LedMatrixDirectFast4.h>
#endif

using ace_common::incrementMod;
using namespace ace_segment;

#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_HC595_DUAL 3
#define LED_DISPLAY_TYPE_HC595_SINGLE 4
#define LED_DISPLAY_TYPE_BARE 5

#define LED_MATRIX_MODE_NONE 0
#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_DIRECT_FAST 2
#define LED_MATRIX_MODE_SINGLE_SW_SPI 3
#define LED_MATRIX_MODE_SINGLE_HW_SPI 4
#define LED_MATRIX_MODE_SINGLE_SW_SPI_FAST 5
#define LED_MATRIX_MODE_DUAL_SW_SPI 6
#define LED_MATRIX_MODE_DUAL_HW_SPI 7
#define LED_MATRIX_MODE_DUAL_SW_SPI_FAST 8

//------------------------------------------------------------------
// Hardware environment configuration.
//------------------------------------------------------------------

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

const uint8_t NUM_DIGITS = 4;
const uint8_t NUM_SEGMENTS = 8;

#if defined(EPOXY_DUINO)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT

#elif defined(AUNITER_LED_CLOCK_DIRECT)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST

#elif defined(AUNITER_LED_CLOCK_SINGLE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SW_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HW_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SW_SPI_FAST

#elif defined(AUNITER_LED_CLOCK_DUAL)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SW_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HW_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SW_SPI_FAST

#elif defined(AUNITER_LED_CLOCK_HC595_DUAL)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595_DUAL
  #define LED_MATRIX_MODE LED_MATRIX_MODE_NONE

#elif defined(AUNITER_LED_CLOCK_HC595_SINGLE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595_SINGLE
  #define LED_MATRIX_MODE LED_MATRIX_MODE_NONE

#elif defined(AUNITER_LED_CLOCK_BARE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_BARE
  #define LED_MATRIX_MODE LED_MATRIX_MODE_NONE

#endif

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

// Total field/second
//      = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//      = 60 * 16 * 4 = 3840 fields/sec = 260 micros/field
//
// According to AutoBenchmark, almost all versions of ScanningModule with
// various LedMatrix can render a single field in less than this on 16 MHz AVR
// processor. The combination of (ScanningModule + LedMatrixDualShiftRegister +
// SwSpiInterface) is the exception.
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
    const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
    const uint8_t DATA_PIN = MOSI; // DS on 74HC595
    const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595
  #endif

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_DUAL
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_SINGLE
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_BARE
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#endif

// The chain of resources.
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
    // Common Anode, with transitors on Group pins
    using LedMatrix = LedMatrixDirect<>;
    LedMatrix ledMatrix(
        LedMatrix::kActiveLowPattern /*elementOnPattern*/,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
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
        LedMatrix::kActiveLowPattern /*elementOnPattern*/,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SW_SPI
    // Common Cathode, with transistors on Group pins
    SwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixSingleShiftRegister<SwSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS):
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SW_SPI_FAST
    // Common Cathode, with transistors on Group pins
    using SpiInterface = SwSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixSingleShiftRegister<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HW_SPI
    // Common Cathode, with transistors on Group pins
    HwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixSingleShiftRegister<HwSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI
    // Common Anode, with transistors on Group pins
    SwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualShiftRegister<SwSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI_FAST
    // Common Anode, with transistors on Group pins
    using SpiInterface = SwSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    using LedMatrix = LedMatrixDualShiftRegister<SpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/);
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HW_SPI
    // Common Anode, with transistors on Group pins
    HwSpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualShiftRegister<HwSpiInterface>;
    LedMatrix ledMatrix(
        spiInterface,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/);

  #else
    #error Unsupported LED_MATRIX_MODE
  #endif

  // 16 levels of brightness, need render-fields/second of 60*4*16 = 3840.
  ScanningModule<LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      modulatingModule(ledMatrix, FRAMES_PER_SECOND);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_DUAL
  // Common Anode, with transistors on Group pins
  using SpiInterface = SwSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  DualHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> modulatingModule(
      spiInterface,
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_SINGLE
  // Common Cathode, with transistors on Group pins
  using SpiInterface = SwSpiInterface;
  SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  SingleHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> modulatingModule(
      spiInterface,
      LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_BARE
  // Common Anode, with transitors on Group pins
  BareModule<NUM_DIGITS, NUM_SUBFIELDS> modulatingModule(
      LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
      LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      SEGMENT_PINS,
      DIGIT_PINS);

#endif

// Setup the various resources.
void setupAceSegment() {

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SW_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI_FAST
    spiInterface.begin();
  #endif

  ledMatrix.begin();
  modulatingModule.begin();

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_DUAL
  spiInterface.begin();
  modulatingModule.begin();
  modulatingModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595_SINGLE
  spiInterface.begin();
  modulatingModule.begin();
  modulatingModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_BARE
  modulatingModule.begin();
  modulatingModule.setBrightness(1); // 0-1

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
  LedDisplay modulatingDisplay(modulatingModule);
  NumberWriter numberWriter(modulatingDisplay);
  numberWriter.writeHexCharAt(0, 0);
  numberWriter.writeHexCharAt(1, 1);
  numberWriter.writeHexCharAt(2, 2);
  numberWriter.writeHexCharAt(3, 3);
}

void setBrightnesses(int i) {
  uint8_t brightness0 = BRIGHTNESS_LEVELS[i % NUM_BRIGHTNESSES];
  uint8_t brightness1 = BRIGHTNESS_LEVELS[(i+1) % NUM_BRIGHTNESSES];
  uint8_t brightness2 = BRIGHTNESS_LEVELS[(i+2) % NUM_BRIGHTNESSES];
  uint8_t brightness3 = BRIGHTNESS_LEVELS[(i+3) % NUM_BRIGHTNESSES];
  modulatingModule.setBrightnessAt(0, brightness0);
  modulatingModule.setBrightnessAt(1, brightness1);
  modulatingModule.setBrightnessAt(2, brightness2);
  modulatingModule.setBrightnessAt(3, brightness3);
}

void pulseDisplay() {
  static uint8_t i = 0;
  static unsigned long lastUpdateTime = millis();

  unsigned long now = millis();
  if (now - lastUpdateTime > 200) {
    lastUpdateTime = now;
    incrementMod(i, NUM_BRIGHTNESSES);
    setBrightnesses(i);
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
    modulatingModule.renderFieldWhenReady();
  #endif
}
