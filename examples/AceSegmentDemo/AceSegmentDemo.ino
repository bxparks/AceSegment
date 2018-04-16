#include <AceSegment.h>
#include "FastDirectDriver.h"
#include "FastSerialDriver.h"
#include "FastSpiDriver.h"
using namespace ace_segment;

// Use polling or interrupt.
#define USE_INTERRUPT 0

#define DRIVER_MODE_NONE 0
#define DRIVER_MODE_DIGIT 1
#define DRIVER_MODE_MODULATING_DIGIT 2
#define DRIVER_MODE_SEGMENT 3
#define DRIVER_MODE_FAST_DIRECT 4
#define DRIVER_MODE_FAST_SERIAL 5
#define DRIVER_MODE_FAST_SPI 6

#define LED_MATRIX_MODE_DIRECT 1
#define LED_MATRIX_MODE_SERIAL 2
#define LED_MATRIX_MODE_SPI 3

#define WRITE_MODE_NONE 0
#define WRITE_MODE_HEX 1
#define WRITE_MODE_CHAR 2
#define WRITE_MODE_STRING 3
#define WRITE_MODE_SCROLL 4

// Define the Driver to use. Use DRIVER_MODE_NONE to get flash/static
// consumption without any AceSegment code. Then set to the other modes to get
// flash/static memory usage.
#define DRIVER_MODE DRIVER_MODE_DIGIT

// Applies only for DRIVER_MODE_DIGIT, DRIVER_MODE_MODULATING_DIGIT,
// DRIVER_MODE_SEGMENT. Ignored for others.
#define LED_MATRIX_MODE LED_MATRIX_MODE_SERIAL

// Type of characters to write to the LED display
#define WRITE_MODE WRITE_MODE_HEX

// Transistor drivers on digits.
#define USE_TRANSISTOR_DRIVERS 1

const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 16;

#if DRIVER_MODE == DRIVER_MODE_SEGMENT
// 4 digits, resistors on digits
const uint8_t NUM_DIGITS = 4;
const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
#else
  #if ((DRIVER_MODE == DRIVER_MODE_DIGIT \
      || DRIVER_MODE == DRIVER_MODE_MODULATING_DIGIT \
      || DRIVER_MODE == DRIVER_MODE_SEGMENT) \
        && LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT) \
      || DRIVER_MODE == DRIVER_MODE_FAST_DIRECT
  // 4 digits, resistors on segments
  const uint8_t NUM_DIGITS = 4;
  const uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
  const uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
  #else
  // 4 digits, resistors on segments, serial-to-parallel converter on segments
  const uint8_t NUM_DIGITS = 4;
  const uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t latchPin = 10; // ST_CP on 74HC595
  const uint8_t dataPin = 11; // DS on 74HC595
  const uint8_t clockPin = 13; // SH_CP on 74HC595
  #endif
#endif

#if DRIVER_MODE > DRIVER_MODE_NONE
// Set up the chain of resources and their dependencies.
DimmingDigit dimmingDigits[NUM_DIGITS];
StyledDigit styledDigits[NUM_DIGITS];

// The chain of resources.
Hardware* hardware;
Driver* driver;
Renderer* renderer;
CharWriter* charWriter;
HexWriter* hexWriter;
StringWriter* stringWriter;

#if USE_INTERRUPT == 1
// interrupt handler for timer 2
ISR(TIMER2_COMPA_vect) {
  renderer->renderField();
}
#endif

#endif

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

#if DRIVER_MODE > DRIVER_MODE_NONE
  setupAceSegment();
#endif

  Serial.println(F("setup(): end"));
}

#if DRIVER_MODE > DRIVER_MODE_NONE
// Create the resources.
void setupAceSegment() {
  // Hardware is exposed to the client code because it's shared between Drive
  // and Renderer.
  hardware = new Hardware();

  // Create the Driver.
#if DRIVER_MODE == DRIVER_MODE_DIGIT \
    || DRIVER_MODE == DRIVER_MODE_MODULATING_DIGIT
  driver = DriverBuilder(hardware)
      .setNumDigits(NUM_DIGITS)
      .setCommonCathode()
      .setResistorsOnSegments()
      .setDigitPins(digitPins)
  #if LED_MATRIX_MODE == LED_MATRIX_MODE_DIRECT
      .setSegmentDirectPins(segmentPins)
  #elif LED_MATRIX_MODE == LED_MATRIX_MODE_SERIAL
      .setSegmentSerialPins(latchPin, dataPin, clockPin)
  #else
      .setSegmentSpiPins(latchPin, dataPin, clockPin)
  #endif
  #if DRIVER_MODE == DRIVER_MODE_MODULATING_DIGIT
      .useModulatingDriver(NUM_SUBFIELDS)
  #endif
      .setDimmingDigits(dimmingDigits)
  #if USE_TRANSISTOR_DRIVERS == 1
      .useTransistorDrivers()
  #endif
      .build();
#elif DRIVER_MODE == DRIVER_MODE_SEGMENT
  driver = DriverBuilder(hardware)
      .setNumDigits(NUM_DIGITS)
      .setCommonCathode()
      .setResistorsOnDigits()
      .setDigitPins(digitPins)
      .setSegmentDirectPins(segmentPins)
      .setDimmingDigits(dimmingDigits)
  #if USE_TRANSISTOR_DRIVERS == 1
      .useTransistorDrivers()
  #endif
      .build();
#elif DRIVER_MODE == DRIVER_MODE_FAST_DIRECT
  driver = new FastDirectDriver(dimmingDigits, NUM_DIGITS, NUM_SUBFIELDS);
#elif DRIVER_MODE == DRIVER_MODE_FAST_SERIAL
  driver = new FastSerialDriver(dimmingDigits, NUM_DIGITS, NUM_SUBFIELDS);
#elif DRIVER_MODE == DRIVER_MODE_FAST_SPI
  driver = new FastSpiDriver(dimmingDigits, NUM_DIGITS, NUM_SUBFIELDS);
#endif
  driver->configure();

  // Create the Renderer.
  renderer = RendererBuilder(hardware, driver, styledDigits, NUM_DIGITS)
      .setFramesPerSecond(FRAMES_PER_SECOND)
      .build();
  renderer->configure();

#if WRITE_MODE == WRITE_MODE_HEX
  hexWriter = new HexWriter(renderer);
#elif WRITE_MODE == WRITE_MODE_CHAR
  charWriter = new CharWriter(renderer);
#elif WRITE_MODE == WRITE_MODE_STRING || WRITE_MODE == WRITE_MODE_SCROLL
  charWriter = new CharWriter(renderer);
  stringWriter = new StringWriter(charWriter);
#endif

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

void loop() {
  static unsigned long lastUpdateTime = millis();
  static unsigned long stopWatchStart = lastUpdateTime;
  static uint32_t loopCount = 0;
  static uint16_t lastStatsCounter = 0;

  unsigned long now = millis();
  if (now - lastUpdateTime > 1000) {
    lastUpdateTime = now;

#if DRIVER_MODE > DRIVER_MODE_NONE
  #if WRITE_MODE == WRITE_MODE_HEX
    writeHexes();
  #elif WRITE_MODE == WRITE_MODE_CHAR
    writeChars();
  #elif WRITE_MODE == WRITE_MODE_STRING
    writeStrings();
  #elif WRITE_MODE == WRITE_MODE_SCROLL
    scrollString("   Angela is the best.");
  #endif
#endif
  }

  // Print out statistics every N seconds.
  unsigned long elapsedTime = now - stopWatchStart;
  if (elapsedTime >= 2000) {
#if DRIVER_MODE > DRIVER_MODE_NONE
    TimingStats stats = renderer->getTimingStats();
#else
    TimingStats stats;
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

#if DRIVER_MODE > DRIVER_MODE_NONE
#if USE_INTERRUPT == 0
  renderer->renderFieldWhenReady();
#endif
#endif
}

#if DRIVER_MODE > DRIVER_MODE_NONE && WRITE_MODE > WRITE_MODE_NONE

#if WRITE_MODE == WRITE_MODE_HEX
void writeHexes() {
  static uint8_t c = 0;

  uint8_t buffer[3];
  buffer[0] = (c & 0xf0) >> 4;
  buffer[1] = (c & 0x0f);
  hexWriter->writeHexAt(0, buffer[0], StyledDigit::kStylePulseFast);
  hexWriter->writeHexAt(1, buffer[1], StyledDigit::kStylePulseSlow);
  hexWriter->writeHexAt(2, HexWriter::kMinus, StyledDigit::kStyleBlinkFast);
  hexWriter->writeHexAt(3, c, StyledDigit::kStyleBlinkSlow);

  Util::incrementMod(c, HexWriter::kNumCharacters);
}
#endif

#if WRITE_MODE == WRITE_MODE_CHAR
void writeChars() {
  static uint8_t c = 0;

  char buffer[3];
  buffer[0] = (c & 0xf0) >> 4;
  buffer[1] = (c & 0x0f);
  charWriter->writeCharAt(0, buffer[0], StyledDigit::kStylePulseFast);
  charWriter->writeCharAt(1, buffer[1], StyledDigit::kStylePulseSlow);
  charWriter->writeCharAt(2, '-', StyledDigit::kStyleBlinkFast);
  charWriter->writeCharAt(3, c, StyledDigit::kStyleBlinkSlow);

  Util::incrementMod(c, CharWriter::kNumCharacters);
}
#endif

#if WRITE_MODE == WRITE_MODE_STRING
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
#endif

void scrollString(const char* s) {
  static uint8_t i = 0;

  stringWriter->writeStringAt(0, &s[i], true /* padRight */);
  Util::incrementMod(i, (uint8_t) strlen(s));
}
#endif
