#include <Arduino.h>
#include <AceButton.h>
#include <AceSegment.h>
#ifdef __AVR__
  #include "FastDirectDriver.h"
  #include "FastSerialDriver.h"
  #include "FastSpiDriver.h"
#endif

using namespace ace_segment;
using namespace ace_button;

//------------------------------------------------------------------
// Configurations for AceButton
//------------------------------------------------------------------

const uint8_t LOOP_MODE_PAUSE = 0;
const uint8_t LOOP_MODE_STEP = 1;
const uint8_t LOOP_MODE_AUTO_RENDER = 2;
uint8_t loopMode = LOOP_MODE_AUTO_RENDER;

// Configuration for AceButton, to support Single-Step

#if defined(EPOXY_DUINO)
  const uint8_t MODE_BUTTON_PIN = 8;
  const uint8_t CHANGE_BUTTON_PIN = 9;
#elif defined(AUNITER_LED_CLOCK)
  const uint8_t MODE_BUTTON_PIN = 8;
  const uint8_t CHANGE_BUTTON_PIN = 9;
#else
  #error Unknown AUNITER environment
#endif

AceButton button;

void handleEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventReleased:
      if (loopMode == LOOP_MODE_AUTO_RENDER) {
        loopMode = LOOP_MODE_PAUSE;
        Serial.println(F("handleEvent(): paused"));
      } else {
        Serial.println(F("handleEvent(): stepping"));
        loopMode = LOOP_MODE_STEP;
      }
      break;
    case AceButton::kEventLongPressed:
      Serial.println(F("handleEvent(): auto render"));
      loopMode = LOOP_MODE_AUTO_RENDER;
      break;
  }
}

void setupAceButton() {
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  button.setEventHandler(handleEvent);
  button.init(MODE_BUTTON_PIN);
  ButtonConfig* config = button.getButtonConfig();
  config->setFeature(ButtonConfig::kFeatureLongPress);
  config->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
}

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

#define DRIVER_MODE_NONE 0
#define DRIVER_MODE_DIRECT_DIGIT 1
#define DRIVER_MODE_DIRECT_FAST_DIGIT 2
#define DRIVER_MODE_DIRECT_SEGMENT 3
#define DRIVER_MODE_SERIAL_DIGIT 4
#define DRIVER_MODE_SERIAL_FAST_DIGIT 5
#define DRIVER_MODE_SPI_DIGIT 6
#define DRIVER_MODE_SPI_FAST_DIGIT 7
#define DRIVER_MODE_MERGED_SERIAL_DIGIT 8
#define DRIVER_MODE_MERGED_SPI_DIGIT 9

// Use polling or interrupt.
#define USE_INTERRUPT 0

// Print Stats. If USE_INTERRUPT is 0 (so using renderWhenReady()), writing to
// Serial can cause the display to flicker due to timing jitters. Turning stats
// off will make the display as smooth as the interrupt version.
#define PRINT_STATS 0

// Define the Driver to use. Use DRIVER_MODE_NONE to get flash/static
// consumption without any AceSegment code. Then set to the other modes to get
// flash/static memory usage.
//#define DRIVER_MODE DRIVER_MODE_MERGED_SPI_DIGIT
#define DRIVER_MODE DRIVER_MODE_MERGED_SERIAL_DIGIT

// Transistors on the digits or segments which do NOT have the resistors.
#define USE_TRANSISTORS true

// Common Cathode or Anode
#define COMMON_CATHODE true

const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 1;

const uint16_t STATS_RESET_INTERVAL = 1200;

const uint8_t NUM_DIGITS = 4;
const uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};

const uint8_t NUM_SEGMENTS = 8;

#if DRIVER_MODE == DRIVER_MODE_DIRECT_DIGIT || \
    DRIVER_MODE == DRIVER_MODE_DIRECT_SEGMENT || \
    DRIVER_MODE == DRIVER_MODE_DIRECT_FAST_DIGIT
  // 4 digits, resistors on segments on Pro Micro.
  const uint8_t segmentPins[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};
#else
  const uint8_t latchPin = 10; // ST_CP on 74HC595
  const uint8_t dataPin = MOSI; // DS on 74HC595
  const uint8_t clockPin = SCK; // SH_CP on 74HC595
#endif

#if DRIVER_MODE > DRIVER_MODE_NONE
  // Set up the chain of resources and their dependencies.
  DimmablePattern dimmablePatterns[NUM_DIGITS];

  // The chain of resources.
  Hardware* hardware;
  Driver* driver;
  Renderer* renderer;
  CharWriter* charWriter;
  HexWriter* hexWriter;
  ClockWriter* clockWriter;
  StringWriter* stringWriter;

  #if USE_INTERRUPT == 1
  // interrupt handler for timer 2
  ISR(TIMER2_COMPA_vect) {
    renderer->renderField();
  }
  #endif
#endif

#if DRIVER_MODE > DRIVER_MODE_NONE
// Create the resources.
void setupAceSegment() {
  // Hardware is exposed to the client code because it's shared between Drive
  // and Renderer.
  hardware = new Hardware();

  // Create the Driver.
#if DRIVER_MODE == DRIVER_MODE_DIRECT_DIGIT
  driver = new SplitDirectDigitDriver(
      hardware, dimmablePatterns,
      COMMON_CATHODE, USE_TRANSISTORS,
      false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
      NUM_SUBFIELDS, digitPins, segmentPins);
#elif DRIVER_MODE == DRIVER_MODE_SERIAL_DIGIT
  driver = new SplitSerialDigitDriver(
      hardware, dimmablePatterns,
      COMMON_CATHODE, USE_TRANSISTORS,
      false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
      NUM_SUBFIELDS, digitPins, latchPin, dataPin, clockPin);
#elif DRIVER_MODE == DRIVER_MODE_SPI_DIGIT
  driver = new SplitSerialDigitDriver(
      hardware, dimmablePatterns,
      COMMON_CATHODE, USE_TRANSISTORS,
      false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
      NUM_SUBFIELDS, digitPins, latchPin, dataPin, clockPin);
#elif DRIVER_MODE == DRIVER_MODE_DIRECT_SEGMENT
  driver = new SplitDirectSegmentDriver(
      hardware, dimmablePatterns, COMMON_CATHODE,
      false /* transistorsOnDigits */,
      USE_TRANSISTORS /* transistorsOnSegments */,
      NUM_DIGITS, NUM_SEGMENTS, digitPins, segmentPins);
#elif DRIVER_MODE == DRIVER_MODE_MERGED_SERIAL_DIGIT
  driver = new MergedSerialDigitDriver(
      hardware, dimmablePatterns,
      false /*commonCathode*/, USE_TRANSISTORS,
      false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
      NUM_SUBFIELDS, latchPin, dataPin, clockPin);
#elif DRIVER_MODE == DRIVER_MODE_MERGED_SPI_DIGIT
  driver = new MergedSpiDigitDriver(
      hardware, dimmablePatterns,
      false /*commonCathode*/, USE_TRANSISTORS,
      false /* transistorsOnSegments */, NUM_DIGITS, NUM_SEGMENTS,
      NUM_SUBFIELDS, latchPin, dataPin, clockPin);
#else
  #ifdef __AVR__
    #if DRIVER_MODE == DRIVER_MODE_DIRECT_FAST_DIGIT
      driver = new FastDirectDriver(
          dimmablePatterns, NUM_DIGITS, NUM_SUBFIELDS);
    #elif DRIVER_MODE == DRIVER_MODE_SERIAL_FAST_DIGIT
      driver = new FastSerialDriver(
          dimmablePatterns, NUM_DIGITS, NUM_SUBFIELDS);
    #elif DRIVER_MODE == DRIVER_MODE_SPI_FAST_DIGIT
      driver = new FastSpiDriver(
          dimmablePatterns, NUM_DIGITS, NUM_SUBFIELDS);
    #endif
  #else
    #error Unsupported platform
  #endif
#endif

  driver->configure();

  renderer = new Renderer(hardware, driver, dimmablePatterns,
      NUM_DIGITS, FRAMES_PER_SECOND, STATS_RESET_INTERVAL);
  renderer->configure();

  hexWriter = new HexWriter(renderer);
  charWriter = new CharWriter(renderer);
  stringWriter = new StringWriter(charWriter);
  clockWriter = new ClockWriter(renderer);

#if USE_INTERRUPT == 1
  // set up Timer 2
  uint8_t timerCompareValue =
      (long) F_CPU / 1024 / renderer->getFieldsPerSecond() - 1;
  Serial.print(F("Timer 2, Compare A: "));
  Serial.println(timerCompareValue);

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
#endif

//------------------------------------------------------------------
// Configurations for AceSegmentDemo
//------------------------------------------------------------------

void singleStep() {
  renderer->renderField();
}

const uint8_t DEMO_MODE_COUNT = 4;
const uint8_t DEMO_MODE_HEX = 0;
const uint8_t DEMO_MODE_CHAR = 1;
const uint8_t DEMO_MODE_SCROLL = 2;
const uint8_t DEMO_MODE_CLOCK = 3;

// Demo mode.
uint8_t demoMode = DEMO_MODE_CHAR;

void writeHexes() {
  static uint8_t c = 0;

  uint8_t hexHigh = (c & 0xf0) >> 4;
  uint8_t hexLow = (c & 0x0f);
  hexWriter->writeHexAt(0, hexHigh);
  hexWriter->writeHexAt(1, hexLow);
  hexWriter->writeHexAt(2, HexWriter::kMinus);
  hexWriter->writeHexAt(3, c);

  if (c < HexWriter::kNumCharacters * 2) {
    Util::incrementMod(c, HexWriter::kNumCharacters);
  } else {
    c = 0;
    Util::incrementMod(demoMode, DEMO_MODE_COUNT);
  }
}

void writeClock() {
  static uint8_t hh = 0;
  static uint8_t mm = 0;

  clockWriter->writeClock(hh, mm);

  Util::incrementMod(mm, (uint8_t)100);
  Util::incrementMod(hh, (uint8_t)100);
}

void writeChars() {
  static uint8_t c = 0;

  uint8_t hexHigh = (c & 0xf0) >> 4;
  uint8_t hexLow = (c & 0x0f);
  hexWriter->writeHexAt(0, hexHigh);
  hexWriter->writeHexAt(1, hexLow);
  charWriter->writeDecimalPointAt(1);
  charWriter->writeCharAt(2, ' ');
  charWriter->writeCharAt(3, c);

  Util::incrementMod(c, CharWriter::kNumCharacters);
}

void writeStrings() {
  static const char* STRINGS[] = {
    /*
    "0123",
    "1.123",
    "2.1 ",
    "3.2.3.4.",
    "4bc.d",
    ".1.2..3",
    */
    "brian"
  };

  static const uint8_t NUM_STRINGS = sizeof(STRINGS) / sizeof(STRINGS[0]);
  static uint8_t i = 0;

  stringWriter->writeStringAt(0, STRINGS[i]);

  Util::incrementMod(i, NUM_STRINGS);
}

void scrollString(const char* s) {
  static uint8_t i = 0;

  stringWriter->writeStringAt(0, &s[i], true /* padRight */);
  Util::Util::incrementMod(i, (uint8_t) strlen(s));
}

void autoRender() {
  static uint16_t iter = 0;
  static unsigned long lastUpdateTime = millis();

#if PRINT_STATS == 1
  static unsigned long stopWatchStart = lastUpdateTime;
  static uint32_t loopCount = 0;
  static uint16_t lastStatsCounter = 0;
#endif

  unsigned long now = millis();
  if (now - lastUpdateTime > 200) {
    lastUpdateTime = now;
    if (iter == 0) {
      Serial.print(F("Demo Mode: "));
      Serial.println(demoMode);
    }

    if (demoMode == DEMO_MODE_HEX) {
      writeHexes();
    } else if (demoMode == DEMO_MODE_CHAR) {
      writeChars();
    } else if (demoMode == DEMO_MODE_SCROLL) {
      scrollString("   Angela is the best.");
    } else if (demoMode == DEMO_MODE_CLOCK) {
      writeClock();
    }
    if (iter++ >= 100) {
      Util::incrementMod(demoMode, DEMO_MODE_COUNT);
      iter = 0;
    }
  }

#if PRINT_STATS == 1
  // Print out statistics every N seconds.
  unsigned long elapsedTime = now - stopWatchStart;
  if (elapsedTime >= 2000) {
  #if DRIVER_MODE > DRIVER_MODE_NONE
    ace_segment::TimingStats stats = renderer->getTimingStats();
  #else
    ace_segment::TimingStats stats;
  #endif
    uint32_t elapsedCount = stats.getCounter() - lastStatsCounter;
    lastStatsCounter = stats.getCounter();
    uint16_t renderDurationAverage = stats.getAvg();
    uint16_t renderDurationMin = stats.getMin();
    uint16_t renderDurationMax = stats.getMax();

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

    stopWatchStart = now;
    loopCount = 0;
  } else {
    loopCount++;
  }
#endif

#if DRIVER_MODE > DRIVER_MODE_NONE
#if USE_INTERRUPT == 0
  renderer->renderFieldWhenReady();
#endif
#endif
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  setupAceButton();

#if DRIVER_MODE > DRIVER_MODE_NONE
  setupAceSegment();
#endif

  Serial.println(F("setup(): end"));
}

void loop() {
  if (loopMode == LOOP_MODE_STEP) {
    singleStep();
    loopMode = LOOP_MODE_PAUSE;
  } else if (loopMode == LOOP_MODE_AUTO_RENDER) {
    autoRender();
  }

  button.check();
}
