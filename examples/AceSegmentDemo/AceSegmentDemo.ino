#include <AceSegment.h>
using namespace ace_segment;

#define USE_INTERRUPT 0

#define DRIVER_MODE_DIGIT 1
#define DRIVER_MODE_MODULATING 2
#define DRIVER_MODE_SEGMENT 3
#define DRIVER_MODE_SERIAL 4

#define DRIVER_MODE DRIVER_MODE_SERIAL

const uint8_t NUM_SUBFIELDS = 12;
const uint8_t FRAMES_PER_SECOND = 60;

#if DRIVER_MODE == DRIVER_MODE_SEGMENT
// 2 digits, resistors on digits
const uint8_t NUM_DIGITS = 2;
uint8_t digitPins[NUM_DIGITS] = {12, 14};
uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
#elif DRIVER_MODE == DRIVER_MODE_DIGIT || DRIVER_MODE == DRIVER_MODE_MODULATING
// 4 digits, resistors on segments
const uint8_t NUM_DIGITS = 4;
uint8_t digitPins[NUM_DIGITS] = {12, 14, 15, 16};
uint8_t segmentPins[8] = {4, 5, 6, 7, 8, 9, 10, 11};
#elif DRIVER_MODE == DRIVER_MODE_SERIAL
// 2 digits, resistors on segments, serial-to-parallel on segments
const uint8_t NUM_DIGITS = 4;
uint8_t digitPins[NUM_DIGITS] = {4, 5, 6, 7};
uint8_t latchPin = 10; // ST_CP on 74HC595
uint8_t clockPin = 13; // SH_CP on 74HC595
uint8_t dataPin = 11; // DS on 74HC595
#endif

// Set up the chain of resources and their dependencies.
Hardware hardware;
DimmingDigit dimmingDigits[NUM_DIGITS];
StyledDigit styledDigits[NUM_DIGITS];

#if DRIVER_MODE == DRIVER_MODE_DIGIT
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();
#elif DRIVER_MODE == DRIVER_MODE_MODULATING
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .setNumSubFields(NUM_SUBFIELDS)
    .useModulatingDriver()
    .build();
#elif DRIVER_MODE == DRIVER_MODE_SEGMENT
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnDigits()
    .setDigitPins(digitPins)
    .setSegmentDirectPins(segmentPins)
    .setDimmingDigits(dimmingDigits)
    .build();
#elif DRIVER_MODE == DRIVER_MODE_SERIAL
Driver* driver = DriverBuilder()
    .setHardware(&hardware)
    .setNumDigits(NUM_DIGITS)
    .setCommonCathode()
    .setResistorsOnSegments()
    .setDigitPins(digitPins)
    .setSegmentSerialPins(latchPin, dataPin, clockPin)
    .setDimmingDigits(dimmingDigits)
    .build();
#endif

Renderer renderer(&hardware, driver, styledDigits, NUM_DIGITS);
CharWriter charWriter(&renderer);
StringWriter stringWriter(&charWriter);

const char* STRINGS[] = {
  "0123",
  "1.123",
  "2.1 ",
  "3.2.3.4.",
  "4bc.d",
  ".1.2..3",
  "brian"
};

const int NUM_STRINGS = sizeof(STRINGS) / sizeof(STRINGS[0]);

#if USE_INTERRUPT == 1
// interrupt handler for timer 2
ISR(TIMER2_COMPA_vect) {
  renderer.renderField();
}
#endif

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro

  renderer.setFramesPerSecond(FRAMES_PER_SECOND);
  renderer.configure();

#if USE_INTERRUPT == 1
  // set up Timer 2
  uint8_t timerCompareValue =
      (long) F_CPU / 1024 / renderer.getFieldsPerSecond() - 1;
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

  Serial.println(F("setup(): end"));
}

void loop() {
  static unsigned long lastUpdateTime = millis();
  static unsigned long stopWatchStart = lastUpdateTime;
  static uint16_t lastRenderCount = 0;
  static uint32_t loopCount = 0;

  // Print something every 400 ms.
  unsigned long now = millis();
  if (now - lastUpdateTime > 1000) {
    lastUpdateTime = now;

    writeChars();
    //writeStrings();
    //scrollString("   Angela is the best.");
  }

  // Print out statistics every 10 seconds.
  unsigned long elapsedTime = now - stopWatchStart;
  if (elapsedTime >= 2000) {
    uint16_t renderCounter = renderer.getRenderFieldCounter();
    uint16_t elapsedCount = renderCounter - lastRenderCount;
    uint16_t renderDurationAverage = renderer.getRenderFieldDurationAverage();
    uint16_t renderDurationMin = renderer.getRenderFieldDurationMin();
    uint16_t renderDurationMax = renderer.getRenderFieldDurationMax();

    Serial.print("loops: ");
    Serial.print(loopCount);
    Serial.print("; renders: ");
    Serial.print(elapsedCount);
    Serial.print("; t: ");
    Serial.print(elapsedTime / 1000);
    Serial.print("s; fields/s: ");
    Serial.print((uint32_t) elapsedCount * 1000 / elapsedTime);
    Serial.print("Hz; min: ");
    Serial.print(renderDurationMin);
    Serial.print("us; avg: ");
    Serial.print(renderDurationAverage);
    Serial.print("us; max: ");
    Serial.print(renderDurationMax);
    Serial.println("us");
    
    stopWatchStart = now;
    lastRenderCount = renderCounter;
    loopCount = 0;
  } else {
    loopCount++;
  }

#if USE_INTERRUPT == 0
  renderer.renderFieldWhenReady();
#endif
}

void writeChars() {
  static uint8_t c = 0;
  if (c >= 128) c = 0;

  char buffer[3];
  sprintf(buffer, "%02X", c);
  charWriter.writeCharAt(0, buffer[0], StyledDigit::kStylePulseFast);
  charWriter.writeCharAt(1, buffer[1], StyledDigit::kStylePulseSlow);
  charWriter.writeCharAt(2, '-', StyledDigit::kStyleBlinkFast);
  charWriter.writeCharAt(3, c, StyledDigit::kStyleBlinkSlow);

  c++;
}

void writeStrings() {
  static uint8_t i = 0;
  if (i >= NUM_STRINGS) i = 0;
  stringWriter.writeStringAt(0, STRINGS[i]);
  i++;
}

void scrollString(const char* s) {
  static uint8_t i = 0;

  if (i >= strlen(s)) i = 0;
  stringWriter.writeStringAt(0, &s[i], true /* padRight */);
  i++;
}
