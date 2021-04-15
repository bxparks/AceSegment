#include <Arduino.h>
#include <AceButton.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h>
#include <ace_segment/fast/LedMatrixDirectFast.h>
#include <ace_segment/fast/SwSpiAdapterFast.h>

using ace_common::incrementMod;

//------------------------------------------------------------------
// Hardware configuration.
//------------------------------------------------------------------

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

using namespace ace_segment;
using namespace ace_button;

#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1

#define LED_MATRIX_MODE_NONE 0
#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_PARIAL_SW_SPI 2
#define LED_MATRIX_MODE_SINGLE_HW_SPI 3
#define LED_MATRIX_MODE_DUAL_SW_SPI 4
#define LED_MATRIX_MODE_DUAL_HW_SPI 5
#define LED_MATRIX_MODE_DIRECT_FAST 6
#define LED_MATRIX_MODE_SINGLE_SW_SPI_FAST 7
#define LED_MATRIX_MODE_DUAL_SW_SPI_FAST 8

// LedClock buttons are now hardwared to A2 and A3, instead of being configured
// with dip switches to either (2,3) or (8,9). Since (2,3) are used by I2C, and
// LED_MATRIX_MODE_DIRECT uses (8,9) pins for two of the LED segments/digits,
// the only spare pins are A2 and A3. All other digital pins are taken.
// Fortunately, the ATmega32U4 allows all analog pins to be used as digital
// pins.
//
// For EpoxyDuino, the actual numbers don't matter, so let's set them to (2,3)
// since I'm not sure if A2 and A3 are defined.
#if defined(EPOXY_DUINO)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  const uint8_t MODE_BUTTON_PIN = 2;
  const uint8_t CHANGE_BUTTON_PIN = 3;

#elif defined(AUNITER_LED_CLOCK_DIRECT)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT_FAST
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

#elif defined(AUNITER_LED_CLOCK_SINGLE)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SW_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_HW_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_SINGLE_SW_SPI_FAST
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

#elif defined(AUNITER_LED_CLOCK_DUAL)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_SCANNING
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SW_SPI
  //#define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_HW_SPI
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DUAL_SW_SPI_FAST
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

#elif defined(AUNITER_LED_CLOCK_TM1637)
  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  #define LED_MATRIX_MODE LED_MATRIX_MODE_NONE
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

// Use polling or interrupt.
#define USE_INTERRUPT 0

// Total field/second = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//      = 60 * 64 * 4 = 15360 fields/sec = 65 micros/field
//
// Fortunately, according to AutoBenchmark, the "fast" versions of LedMatrix can
// render a single field in about 20-30 micros.
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;
const uint8_t NUM_BRIGHTNESSES = 8;

const uint8_t levels[NUM_BRIGHTNESSES] = {
  1, 2, 4, 8,
  15, 7, 3, 2
};

const uint8_t NUM_DIGITS = 4;
const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

#if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
  // 4 digits, resistors on segments on Pro Micro.
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};
#else
  const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
  const uint8_t DATA_PIN = MOSI; // DS on 74HC595
  const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595
#endif

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_TM1637
  const uint8_t CLK_PIN = 16;
  const uint8_t DIO_PIN = 10;
  const uint16_t BIT_DELAY = 100;
#endif

// The chain of resources.
Hardware hardware;

#if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
  // Common Anode, with transitions on Group pins
  using LedMatrix = LedMatrixDirect<Hardware>;
  LedMatrix ledMatrix(
      hardware,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS,
      NUM_SEGMENTS,
      SEGMENT_PINS);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT_FAST
  // Common Anode, with transitions on Group pins
  using LedMatrix = LedMatrixDirectFast<
    4, 5, 6, 7,
    8, 9, 10, 16, 14, 18, 19, 15
  >;
  LedMatrix ledMatrix(
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_PARIAL_SW_SPI
  // Common Cathode, with transistors on Group pins
  SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS):
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SW_SPI_FAST
  // Common Cathode, with transistors on Group pins
  using SpiAdapter = SwSpiAdapterFast<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiAdapter spiAdapter;
  using LedMatrix = LedMatrixSingleShiftRegister<Hardware, SpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HW_SPI
  // Common Cathode, with transistors on Group pins
  HwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixSingleShiftRegister<Hardware, HwSpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI
  // Common Anode, with transistors on Group pins
  SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualShiftRegister<SwSpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI_FAST
  // Common Anode, with transistors on Group pins
  using SpiAdapter = SwSpiAdapterFast<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
  SpiAdapter spiAdapter;
  using LedMatrix = LedMatrixDualShiftRegister<SpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HW_SPI
  // Common Anode, with transistors on Group pins
  HwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixDualShiftRegister<HwSpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);

#elif LED_MATRIX_MODE == LED_MATRIX_MODE_NONE
  // Do nothing

#else
  #error Unsupported LED_MATRIX_MODE
#endif

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  // 1-bit brightness
  ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS> display(
      hardware, ledMatrix, FRAMES_PER_SECOND);

  // 16 levels of brightness, need render-fields/second of 60*4*16 = 3840.
  ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
      scanningDisplayModulating(hardware, ledMatrix, FRAMES_PER_SECOND);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_TM1637
  using TmDriver = Tm1637Driver;
  TmDriver driver(CLK_PIN, DIO_PIN, BIT_DELAY);
  Tm1637Display<TmDriver, 4> display(driver);

#else
  #error Unknown LED_DISPLAY_TYPE
#endif

NumberWriter numberWriter(display);
ClockWriter clockWriter(display);
CharWriter charWriter(display);
StringWriter stringWriter(charWriter);

// Setup the various resources.
void setupAceSegment() {
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_TM1637
  driver.begin();
  display.begin();
#else
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_PARIAL_SW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_HW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_SINGLE_SW_SPI_FAST \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_HW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_DUAL_SW_SPI_FAST
    spiAdapter.begin();
  #endif

  ledMatrix.begin();
  display.begin();
  scanningDisplayModulating.begin();
#endif

#if USE_INTERRUPT == 1
  setupInterupt(display.getFieldsPerSecond());
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

#if USE_INTERRUPT == 1
// interrupt handler for timer 2
ISR(TIMER2_COMPA_vect) {
  if (demoMode == DEMO_MODE_PULSE) {
    scanningDisplayModulating.renderFieldNow();
  } else {
    display.renderFieldNow();
  }
}
#endif

//------------------------------------------------------------------
// Configurations for AceSegmentDemo
//------------------------------------------------------------------

// State of loop, whether paused or not.
const uint8_t DEMO_LOOP_MODE_AUTO = 0;
const uint8_t DEMO_LOOP_MODE_PAUSED = 1;
uint8_t demoLoopMode = DEMO_LOOP_MODE_AUTO;

// Selection of demo.
const uint8_t DEMO_MODE_CLOCK = 0;
const uint8_t DEMO_MODE_HEX_NUMBERS = 1;
const uint8_t DEMO_MODE_UNSIGNED_DEC_NUMBERS = 2;
const uint8_t DEMO_MODE_SIGNED_DEC_NUMBERS = 3;
const uint8_t DEMO_MODE_CHAR = 4;
const uint8_t DEMO_MODE_STRINGS = 5;
const uint8_t DEMO_MODE_SCROLL = 6;
const uint8_t DEMO_MODE_PULSE = 7;
const uint8_t DEMO_MODE_SPIN = 8;
const uint8_t DEMO_MODE_SPIN_2 = 9;
const uint8_t DEMO_MODE_COUNT = 10;
uint8_t prevDemoMode = DEMO_MODE_COUNT - 1;
uint8_t demoMode = DEMO_MODE_UNSIGNED_DEC_NUMBERS;

static const uint16_t DEMO_INTERNAL_DELAY[DEMO_MODE_COUNT] = {
  100, // DEMO_MODE_CLOCK
  10, // DEMO_MODE_HEX_NUMBERS
  10, // DEMO_MODE_UNSIGNED_DEC_NUMBERS
  10, // DEMO_MODE_SIGNED_DEC_NUMBERS
  200, // DEMO_MODE_CHAR
  500, // DEMO_MODE_STRINGS
  500, // DEMO_MODE_SCROLL
  200, // DEMO_MODE_PULSE
  100, // DEMO_MODE_SPIN
  100, // DEMO_MODE_SPIN2
};


//-----------------------------------------------------------------------------

void writeHexNumbers() {
  static uint16_t w = 0;

  numberWriter.writeHexWordAt(0, w);
  w++;
}

//-----------------------------------------------------------------------------

void writeUnsignedDecNumbers() {
  static uint16_t w = 0;

  uint8_t written = numberWriter.writeUnsignedDecimalAt(0, w, -3);
  numberWriter.clearToEnd(written);
  incrementMod(w, (uint16_t) 2000);
}

//-----------------------------------------------------------------------------

void writeSignedDecNumbers() {
  static int16_t w = -999;

  numberWriter.writeSignedDecimalAt(0, w, 4);
  w++;
  if (w > 999) w = -999;
}

//-----------------------------------------------------------------------------

void writeClock() {
  static uint8_t hh = 0;
  static uint8_t mm = 0;

  clockWriter.writeHourMinute(hh, mm);

  incrementMod(mm, (uint8_t)60);
  if (mm == 0) {
    incrementMod(hh, (uint8_t)60);
  }
}

//-----------------------------------------------------------------------------

void writeChars() {
  static uint8_t b = 0;

  numberWriter.writeHexByteAt(0, b);
  charWriter.writeCharAt(2, '-');
  charWriter.writeCharAt(3, b);

  incrementMod(b, CharWriter::kNumCharacters);
}

//-----------------------------------------------------------------------------

void writeStrings() {
  static const char* STRINGS[] = {
    "0123",
    "1.123",
    "2.1 ",
    "3.2.3.4.",
    "4bc.d",
    ".1.2..3",
    "brian"
  };
  static const uint8_t NUM_STRINGS = sizeof(STRINGS) / sizeof(STRINGS[0]);
  static uint8_t i = 0;

  stringWriter.writeStringAt(0, STRINGS[i]);

  incrementMod(i, NUM_STRINGS);
}

void scrollString(const char* s) {
  static uint8_t i = 0;

  stringWriter.writeStringAt(0, &s[i], true /* padRight */);
  incrementMod(i, (uint8_t) strlen(s));
}

//-----------------------------------------------------------------------------

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING

void setupPulseDisplay() {
  NumberWriter numberWriter(scanningDisplayModulating);
  numberWriter.writeHexCharAt(0, 1);
  numberWriter.writeHexCharAt(1, 2);
  numberWriter.writeHexCharAt(2, 3);
  numberWriter.writeHexCharAt(3, 4);
}

void setBrightnesses(int i) {
  uint8_t brightness0 = levels[i % NUM_BRIGHTNESSES];
  uint8_t brightness1 = levels[(i+1) % NUM_BRIGHTNESSES];
  uint8_t brightness2 = levels[(i+2) % NUM_BRIGHTNESSES];
  uint8_t brightness3 = levels[(i+3) % NUM_BRIGHTNESSES];
  scanningDisplayModulating.setBrightnessAt(0, brightness0);
  scanningDisplayModulating.setBrightnessAt(1, brightness1);
  scanningDisplayModulating.setBrightnessAt(2, brightness2);
  scanningDisplayModulating.setBrightnessAt(3, brightness3);
}

void pulseDisplay() {
  static uint8_t i = 0;

  if (prevDemoMode != demoMode) {
    setupPulseDisplay();
  }

  i++;
  setBrightnesses(i);
}

#endif

//-----------------------------------------------------------------------------

static const uint8_t NUM_SPIN_PATTERNS = 3;

const uint8_t SPIN_PATTERNS[NUM_SPIN_PATTERNS][4] PROGMEM = {
  { 0x10, 0x01, 0x08, 0x02 },  // Frame 0
  { 0x20, 0x08, 0x01, 0x04 },  // Frame 1
  { 0x09, 0x00, 0x00, 0x09 },  // Frame 2
};

void spinDisplay() {
  static uint8_t i = 0;
  const uint8_t* patterns = SPIN_PATTERNS[i];
  display.writePatternsAt_P(0, patterns, 4);

  incrementMod(i, NUM_SPIN_PATTERNS);
}

//-----------------------------------------------------------------------------

static const uint8_t NUM_SPIN_PATTERNS_2 = 6;

const uint8_t SPIN_PATTERNS_2[NUM_SPIN_PATTERNS_2][4] PROGMEM = {
  { 0x03, 0x03, 0x03, 0x03 },  // Frame 0
  { 0x06, 0x06, 0x06, 0x06 },  // Frame 1
  { 0x0c, 0x0c, 0x0c, 0x0c },  // Frame 2
  { 0x18, 0x18, 0x18, 0x18 },  // Frame 3
  { 0x30, 0x30, 0x30, 0x30 },  // Frame 4
  { 0x21, 0x21, 0x21, 0x21 },  // Frame 5
};

void spinDisplay2() {
  static uint8_t i = 0;
  const uint8_t* patterns = SPIN_PATTERNS_2[i];
  display.writePatternsAt_P(0, patterns, 4);

  incrementMod(i, NUM_SPIN_PATTERNS_2);
}

//-----------------------------------------------------------------------------

/** Display the demo pattern selected by demoMode. */
void updateDemo() {
  if (demoMode == DEMO_MODE_CLOCK) {
    writeClock();
  } else if (demoMode == DEMO_MODE_HEX_NUMBERS) {
    writeHexNumbers();
  } else if (demoMode == DEMO_MODE_UNSIGNED_DEC_NUMBERS) {
    writeUnsignedDecNumbers();
  } else if (demoMode == DEMO_MODE_SIGNED_DEC_NUMBERS) {
    writeSignedDecNumbers();
  } else if (demoMode == DEMO_MODE_CHAR) {
    writeChars();
  } else if (demoMode == DEMO_MODE_STRINGS) {
    writeStrings();
  } else if (demoMode == DEMO_MODE_SCROLL) {
    scrollString("   Angela is the best.");
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
  } else if (demoMode == DEMO_MODE_PULSE) {
    pulseDisplay();
#endif
  } else if (demoMode == DEMO_MODE_SPIN) {
    spinDisplay();
  } else if (demoMode == DEMO_MODE_SPIN_2) {
    spinDisplay2();
  }
}

/** Go to the next demo */
void nextDemo() {
  prevDemoMode = demoMode;
  incrementMod(demoMode, DEMO_MODE_COUNT);
  if (demoMode == DEMO_MODE_PULSE) {
    incrementMod(demoMode, DEMO_MODE_COUNT);
  }

  display.clear();
  #if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
    scanningDisplayModulating.clear();
  #endif

  updateDemo();
}

/** Loop within a single demo. */
void demoLoop() {
  //static uint16_t iter = 0;
  static unsigned long lastUpdateTime = millis();

  uint16_t demoInternalDelay = DEMO_INTERNAL_DELAY[demoMode];

  unsigned long now = millis();
  if (now - lastUpdateTime > demoInternalDelay) {
    lastUpdateTime = now;
    if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
      updateDemo();
    }

    /*
    if (iter++ >= 100) {
      incrementMod(demoMode, DEMO_MODE_COUNT);
      iter = 0;
    }
    */
  }
}

void renderField() {
  #if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_SCANNING
    if (demoMode == DEMO_MODE_PULSE) {
      scanningDisplayModulating.renderFieldWhenReady();
    } else {
      display.renderFieldWhenReady();
    }
  #else
    display.flush();
    //display.flushIncremental();
  #endif
}

void singleStep() {
  renderField();
}

//------------------------------------------------------------------
// Configurations for AceButton
//------------------------------------------------------------------

const uint8_t RENDER_MODE_AUTO = 0;
const uint8_t RENDER_MODE_PAUSED = 1;
uint8_t renderMode = RENDER_MODE_AUTO;

// Configuration for AceButton, to support Single-Step

AceButton modeButton(MODE_BUTTON_PIN);
AceButton changeButton(CHANGE_BUTTON_PIN);

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  (void) buttonState;

  uint8_t pin = button->getPin();
  if (pin == MODE_BUTTON_PIN) {
    switch (eventType) {
      case AceButton::kEventReleased:
      case AceButton::kEventClicked:
        if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
          demoLoopMode = DEMO_LOOP_MODE_PAUSED;
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo loop paused"));
          }
        } else if (demoLoopMode == DEMO_LOOP_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo stepped"));
          }
          updateDemo();
        }
        break;

      case AceButton::kEventLongPressed:
        if (demoLoopMode == DEMO_LOOP_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo loop enabled"));
          }
          demoLoopMode = DEMO_LOOP_MODE_AUTO;
        }
        break;

      case AceButton::kEventDoubleClicked:
        if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): next demo"));
          }
          nextDemo();
        }
        break;
    }
  } else if (pin == CHANGE_BUTTON_PIN) {
    switch (eventType) {
      case AceButton::kEventReleased:
      case AceButton::kEventClicked:
        if (renderMode == RENDER_MODE_AUTO) {
          renderMode = RENDER_MODE_PAUSED;
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): paused"));
          }
        } else if (renderMode == RENDER_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): stepping"));
          }
          singleStep();
        }
        break;

      case AceButton::kEventLongPressed:
        if (renderMode == RENDER_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): switching to auto rendering"));
          }
          renderMode = RENDER_MODE_AUTO;
        }
        break;
    }
  }
}

void setupAceButton() {
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);
  ButtonConfig* config = ButtonConfig::getSystemButtonConfig();
  config->setEventHandler(handleEvent);
  config->setFeature(ButtonConfig::kFeatureLongPress);
  config->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  config->setFeature(ButtonConfig::kFeatureClick);
  config->setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  config->setFeature(ButtonConfig::kFeatureDoubleClick);
  config->setFeature(ButtonConfig::kFeatureSuppressAfterDoubleClick);
  config->setFeature(ButtonConfig::kFeatureSuppressClickBeforeDoubleClick);
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

  setupAceButton();
  setupAceSegment();

  if (ENABLE_SERIAL_DEBUG >= 1) {
    Serial.println(F("setup(): end"));
  }

  updateDemo();
}

void loop() {
  if (renderMode == RENDER_MODE_AUTO) {
    #if USE_INTERRUPT == 0
      renderField();
    #endif
  }

  if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
    demoLoop();
  }

  modeButton.check();
  changeButton.check();
}
