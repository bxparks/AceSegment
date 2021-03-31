#include <Arduino.h>
#include <AceButton.h>
#include <AceCommon.h> // incrementMod()
#include <AceSegment.h>

//------------------------------------------------------------------
// Hardware configuration.
//------------------------------------------------------------------

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

using namespace ace_segment;
using namespace ace_button;

#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_PARIAL_SW_SPI 2
#define LED_MATRIX_MODE_PARTIAL_HW_SPI 3
#define LED_MATRIX_MODE_FULL_SW_SPI 4
#define LED_MATRIX_MODE_FULL_HW_SPI 5

// LedClock buttons can be configured to use either pins (2, 3) or (8, 9)
// through DIP switches. On the Pro Micro, (2, 3) are used for I2C. The LED
// Module using Direct Digit Pins are wired to use (8, 9), so we must use (2,
// 3) with that LED Module.
const uint8_t MODE_BUTTON_PIN = 2;
const uint8_t CHANGE_BUTTON_PIN = 3;

#if defined(EPOXY_DUINO)
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
#elif defined(AUNITER_LED_CLOCK_DIRECT)
  #define LED_MATRIX_MODE LED_MATRIX_MODE_DIRECT
#elif defined(AUNITER_LED_CLOCK_PARTIAL)
  #define LED_MATRIX_MODE LED_MATRIX_MODE_PARTIAL_HW_SPI
#elif defined(AUNITER_LED_CLOCK_FULL)
  #define LED_MATRIX_MODE LED_MATRIX_MODE_FULL_HW_SPI
#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

// Use polling or interrupt.
#define USE_INTERRUPT 0

// Print Stats. If USE_INTERRUPT is 0 (so using renderWhenReady()), writing to
// Serial can cause the display to flicker due to timing jitters. Turning stats
// off will make the display as smooth as the interrupt version.
#define PRINT_STATS 0

// Transistors on the digits or segments which do NOT have the resistors.
// Common Cathode or Anode
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 1;

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
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_PARIAL_SW_SPI
  // Common Cathode, with transistors on Group pins
  SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixPartialSpi<Hardware, SwSpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS):
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_PARTIAL_HW_SPI
  // Common Cathode, with transistors on Group pins
  HwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixPartialSpi<Hardware, HwSpiAdapter>;
  LedMatrix ledMatrix(
      hardware,
      spiAdapter,
      LedMatrix::kActiveHighPattern /*groupOnPattern*/,
      LedMatrix::kActiveHighPattern /*elementOnPattern*/,
      NUM_DIGITS,
      DIGIT_PINS);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_FULL_SW_SPI
  // Common Anode, with transistors on Group pins
  SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixFullSpi<SwSpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
#elif LED_MATRIX_MODE == LED_MATRIX_MODE_FULL_HW_SPI
  // Common Anode, with transistors on Group pins
  HwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  using LedMatrix = LedMatrixFullSpi<HwSpiAdapter>;
  LedMatrix ledMatrix(
      spiAdapter,
      LedMatrix::kActiveLowPattern /*groupOnPattern*/,
      LedMatrix::kActiveLowPattern /*elementOnPattern*/);
#else
  #error Unsupported LED_MATRIX_MODE
#endif

SegmentDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS> segmentDisplay(
    hardware, ledMatrix, FRAMES_PER_SECOND);

HexWriter hexWriter(segmentDisplay);
ClockWriter clockWriter(segmentDisplay);
CharWriter charWriter(segmentDisplay);
StringWriter stringWriter(charWriter);

#if USE_INTERRUPT == 1
// interrupt handler for timer 2
ISR(TIMER2_COMPA_vect) {
  segmentDisplay.renderField();
}
#endif

// Setup the various resources.
void setupAceSegment() {
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_PARIAL_SW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_PARTIAL_HW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_FULL_SW_SPI \
      || LED_MATRIX_MODE == LED_MATRIX_MODE_FULL_HW_SPI
    spiAdapter.begin();
  #endif

    ledMatrix.begin();
    segmentDisplay.begin();

#if USE_INTERRUPT == 1
  // set up Timer 2
  uint8_t timerCompareValue =
      (long) F_CPU / 1024 / segmentDisplay.getFieldsPerSecond() - 1;
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
#endif
}

//------------------------------------------------------------------
// Configurations for AceSegmentDemo
//------------------------------------------------------------------

const uint8_t DEMO_LOOP_MODE_AUTO = 0;
const uint8_t DEMO_LOOP_MODE_PAUSED = 1;
uint8_t demoLoopMode = DEMO_LOOP_MODE_AUTO;

const uint8_t DEMO_MODE_COUNT = 5;
const uint8_t DEMO_MODE_CLOCK = 0;
const uint8_t DEMO_MODE_HEX = 1;
const uint8_t DEMO_MODE_CHAR = 2;
const uint8_t DEMO_MODE_STRINGS = 3;
const uint8_t DEMO_MODE_SCROLL = 4;

// Demo mode.
uint8_t demoMode = DEMO_MODE_HEX;

void writeHexes() {
  static uint8_t c = 0;

  uint8_t hexHigh = (c & 0xf0) >> 4;
  uint8_t hexLow = (c & 0x0f);
  hexWriter.writeHexAt(0, hexHigh);
  hexWriter.writeHexAt(1, hexLow);
  hexWriter.writeHexAt(2, HexWriter::kMinus);
  hexWriter.writeHexAt(3, c);

  if (c < HexWriter::kNumCharacters * 2) {
    ace_common::incrementMod(c, HexWriter::kNumCharacters);
  } else {
    c = 0;
    ace_common::incrementMod(demoMode, DEMO_MODE_COUNT);
  }
}

void writeClock() {
  static uint8_t hh = 0;
  static uint8_t mm = 0;

  clockWriter.writeClock(hh, mm);

  ace_common::incrementMod(mm, (uint8_t)60);
  if (mm == 0) {
    ace_common::incrementMod(hh, (uint8_t)60);
  }
}

void writeChars() {
  static uint8_t c = 0;

  uint8_t hexHigh = (c & 0xf0) >> 4;
  uint8_t hexLow = (c & 0x0f);
  hexWriter.writeHexAt(0, hexHigh);
  hexWriter.writeHexAt(1, hexLow);
  segmentDisplay.writeDecimalPointAt(1);
  charWriter.writeCharAt(2, ' ');
  charWriter.writeCharAt(3, c);

  ace_common::incrementMod(c, CharWriter::kNumCharacters);
}

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

  ace_common::incrementMod(i, NUM_STRINGS);
}

void scrollString(const char* s) {
  static uint8_t i = 0;

  stringWriter.writeStringAt(0, &s[i], true /* padRight */);
  ace_common::incrementMod(i, (uint8_t) strlen(s));
}

/** Display the demo pattern selected by demoMode. */
void displayDemo() {
  if (demoMode == DEMO_MODE_CLOCK) {
    writeClock();
  } else if (demoMode == DEMO_MODE_HEX) {
    writeHexes();
  } else if (demoMode == DEMO_MODE_CHAR) {
    writeChars();
  } else if (demoMode == DEMO_MODE_STRINGS) {
    writeStrings();
  } else if (demoMode == DEMO_MODE_SCROLL) {
    scrollString("   Angela is the best.");
  }
}

/** Go to the next demo */
void nextDemo() {
  ace_common::incrementMod(demoMode, DEMO_MODE_COUNT);
  displayDemo();
}

/** Loop within a single demo. */
void demoLoop() {
  //static uint16_t iter = 0;
  static unsigned long lastUpdateTime = millis();

#if PRINT_STATS == 1
  static unsigned long stopWatchStart = lastUpdateTime;
  static uint32_t loopCount = 0;
  static uint16_t lastStatsCounter = 0;
#endif

  unsigned long now = millis();
  if (now - lastUpdateTime > 500) {
    lastUpdateTime = now;
    if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
      displayDemo();
    }

    /*
    if (iter++ >= 100) {
      ace_common::incrementMod(demoMode, DEMO_MODE_COUNT);
      iter = 0;
    }
    */
  }
}

#if PRINT_STATS == 1
void printStats() {
  // Print out statistics every N seconds.
  unsigned long elapsedTime = now - stopWatchStart;
  if (elapsedTime >= 2000) {
    uint32_t elapsedCount = stats.getCounter() - lastStatsCounter;
    lastStatsCounter = stats.getCounter();
    uint16_t renderDurationAverage = stats.getAvg();
    uint16_t renderDurationMin = stats.getMin();
    uint16_t renderDurationMax = stats.getMax();

    if (ENABLE_SERIAL_DEBUG >= 1) {
      Serial.print(F("loops: "));
      Serial.print(loopCount);
      Serial.print(F("; renders: "));
      Serial.print(elapsedCount);
      Serial.print(F("; t: "));
      Serial.print(elapsedTime / 1000);
      Serial.print(F("s; fields/s: "));
      Serial.print((uint32_t) elapsedCount * 1000 / elapsedTime);
      Serial.print(F("Hz; min: "));
      Serial.print(renderDurationMin);
      Serial.print(F("us; avg: "));
      Serial.print(renderDurationAverage);
      Serial.print(F("us; max: "));
      Serial.print(renderDurationMax);
      Serial.println(F("us"));
    }

    stopWatchStart = now;
    loopCount = 0;
  } else {
    loopCount++;
  }
}
#endif

void singleStep() {
  segmentDisplay.renderField();
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
          displayDemo();
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

  displayDemo();
}

void loop() {
  if (renderMode == RENDER_MODE_AUTO) {
    #if USE_INTERRUPT == 0
      segmentDisplay.renderFieldWhenReady();
    #endif
  }

  if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
    demoLoop();
  }

  modeButton.check();
  changeButton.check();
}
