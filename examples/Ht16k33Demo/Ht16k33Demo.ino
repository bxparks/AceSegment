/*
 * A demo of a 4-digit, 7-segment HT16K33 LED module from Adafruit.com or one of
 * its clones, with the digits [0,3] scrolling to the left every second, and
 * changing the brightness each iteration.
 *
 * Supported microcontroller environments:
 *
 *  * AUNITER_NANO_HT16K33: Arduino Nano
 *  * AUNITER_MICRO_HT16K33: SparkFun Pro Micro
 *  * AUNITER_SAMD_HT16K33: SAMD21 M0 Mini
 *  * AUNITER_STM32_HT16K33: STM32 F1 Blue Pill
 *  * AUNITER_D1MINILARGE_HT16K33: WeMos D1 Mini ESP8266
 *  * AUNITER_ESP32_HT16K33: ESP32 Dev Kit v1
 */

#include <Arduino.h>
#include <AceCommon.h> // incrementMod()
#include <AceWire.h>
#include <AceSegment.h> // Ht16k33Module

using ace_common::incrementMod;
using ace_common::incrementModOffset;
using ace_common::TimingStats;
using ace_segment::LedModule;
using ace_segment::Ht16k33Module;

// Select the I2C implementation:
//
// Built-in Arduino <Wire.h>
#define WIRE_INTERFACE_TYPE_TWO_WIRE 0

// AceWire/SimpleWireInterface
#define WIRE_INTERFACE_TYPE_SIMPLE_WIRE 1

// AceWire/SimpleWireFastInterface (AVR only)
#define WIRE_INTERFACE_TYPE_SIMPLE_WIRE_FAST 2

// https://github.com/Testato/SoftwareWire (AVR only)
#define WIRE_INTERFACE_TYPE_TESTATO_WIRE 3

// https://github.com/stevemarple/SoftWire
#define WIRE_INTERFACE_TYPE_MARPLE_WIRE 4

// https://github.com/RaemondBW/SWire
#define WIRE_INTERFACE_TYPE_RAEMOND_WIRE 5

// https://github.com/felias-fogg/SlowSoftWire
#define WIRE_INTERFACE_TYPE_FELIAS_FOGG_WIRE 6

// https://github.com/thexeno/HardWire-Arduino-Library (AVR only)
#define WIRE_INTERFACE_TYPE_THEXENO_WIRE 7

// https://github.com/Seeed-Studio/Arduino_Software_I2C
#define WIRE_INTERFACE_TYPE_SEEED_WIRE 8

// https://github.com/todbot/SoftI2CMaster, conflicts with
// https://github.com/felias-fogg/SoftI2CMaster so only one of them can be
// installed at the same time. (AVR only)
#define WIRE_INTERFACE_TYPE_TODBOT_WIRE 9

const uint8_t HT16K33_I2C_ADDRESS = 0x70;

//----------------------------------------------------------------------------
// Hardware configuration.
//----------------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_HT16K33
#endif

#if defined(EPOXY_DUINO)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_SIMPLE_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_ATTINY_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_SIMPLE_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_NANO_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_TWO_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_MICRO_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_TODBOT_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_SAMD_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_TWO_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_STM32_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_TWO_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_D1MINI_LARGE_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_TWO_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#elif defined(AUNITER_ESP32_HT16K33)
  #define WIRE_INTERFACE_TYPE WIRE_INTERFACE_TYPE_TWO_WIRE

  const uint8_t SCL_PIN = SCL;
  const uint8_t SDA_PIN = SDA;
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DELAY_MICROS = 1;

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// AceSegment Configuration
//------------------------------------------------------------------

#if WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_TWO_WIRE
  #warning Using built-in <Wire.h>
  #include <Wire.h>
  using WireInterface = ace_wire::TwoWireInterface<TwoWire>;
  WireInterface wireInterface(Wire);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SIMPLE_WIRE
  #warning Using AceWire/SimpleWireInterface.h
  using WireInterface = ace_wire::SimpleWireInterface;
  WireInterface wireInterface(SDA_PIN, SCL_PIN, DELAY_MICROS);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #warning Using AceWire/SimpleWireFastInterface.h
  #include <digitalWriteFast.h>
  #include <ace_wire/SimpleWireFastInterface.h>
  using WireInterface = ace_wire::SimpleWireFastInterface<
      SDA_PIN, SCL_PIN, DELAY_MICROS>;
  WireInterface wireInterface;
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_TESTATO_WIRE
  #warning Using https://github.com/Testato/SoftwareWire
  #include <SoftwareWire.h>
  SoftwareWire softwareWire(SDA_PIN, SCL_PIN);
  using WireInterface = ace_wire::TestatoWireInterface<SoftwareWire>;
  WireInterface wireInterface(softwareWire);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_MARPLE_WIRE
  #warning Using https://github.com/stevemarple/SoftWire
  #include <SoftWire.h>
  const uint8_t SOFT_WIRE_BUFFER_SIZE = 32;
  uint8_t rxBuffer[SOFT_WIRE_BUFFER_SIZE];
  uint8_t txBuffer[SOFT_WIRE_BUFFER_SIZE];
  SoftWire softWire(SDA_PIN, SCL_PIN);
  using WireInterface = ace_wire::MarpleWireInterface<SoftWire>;
  WireInterface wireInterface(softWire);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_RAEMOND_WIRE
  #warning Using https://github.com/RaemondBW/SWire
  #include <SWire.h>
  using WireInterface = ace_wire::RaemondWireInterface<SoftWire>;
  WireInterface wireInterface(SWire);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_FELIAS_FOGG_WIRE
  #warning Using https://github.com/felias-fogg/SlowSoftWire
  #include <SlowSoftWire.h>
  SlowSoftWire slowSoftWire(SDA_PIN, SCL_PIN);
  using WireInterface = ace_wire::FeliasFoggWireInterface<SlowSoftWire>;
  WireInterface wireInterface(slowSoftWire);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_THEXENO_WIRE
  #warning https://github.com/thexeno/HardWire-Arduino-Library
  #include <HardWire.h> // A different 'TwoWire' class
  using WireInterface = ace_wire::ThexenoWireInterface<TwoWire>;
  WireInterface wireInterface(Wire);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SEEED_WIRE
  #warning Using https://github.com/Seeed-Studio/Arduino_Software_I2C
  #include <SoftwareI2C.h>
  SoftwareI2C seeedWire;
  using WireInterface = ace_wire::SeeedWireInterface<SoftwareI2C>;
  WireInterface wireInterface(seeedWire);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_TODBOT_WIRE
  #warning Using https://github.com/todbot/SoftI2CMaster
  #include <SoftI2CMaster.h>
  SoftI2CMaster wire(SCL_PIN, SDA_PIN);
  using WireInterface = ace_wire::TodbotWireInterface<SoftI2CMaster>;
  WireInterface wireInterface(wire);
#else
  #error Unknown WIRE_INTERFACE_TYPE
#endif

Ht16k33Module<WireInterface, NUM_DIGITS> ledModule(
    wireInterface, HT16K33_I2C_ADDRESS);

void setupAceSegment() {
#if WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_TWO_WIRE
  Wire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SIMPLE_WIRE
  // do nothing
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SIMPLE_WIRE_FAST
  // do nothing
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_TESTATO_WIRE
  softwareWire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_MARPLE_WIRE
  softWire.setRxBuffer(rxBuffer, SOFT_WIRE_BUFFER_SIZE);
  softWire.setTxBuffer(txBuffer, SOFT_WIRE_BUFFER_SIZE);
  softWire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_RAEMOND_WIRE
  SWire.begin(SDA_PIN, SCL_PIN);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_FELIAS_FOGG_WIRE
  slowSoftWire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_THEXENO_WIRE
  Wire.begin();
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_SEEED_WIRE
  seeedWire.begin(SDA_PIN, SCL_PIN);
#elif WIRE_INTERFACE_TYPE == WIRE_INTERFACE_TYPE_TODBOT_WIRE
  wire.begin();
#else
  #error Unknown WIRE_INTERFACE_TYPE
#endif

  wireInterface.begin();
  ledModule.begin();
}

//----------------------------------------------------------------------------

// The HT16K33 controller supports up to 6 digits.
const uint8_t PATTERNS[6] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
};

TimingStats stats;
uint8_t digitIndex = 0;
uint8_t brightness = 0;

// Every second, scroll the display and change the brightness.
void updateDisplay() {
  static uint16_t prevChangeMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevChangeMillis) >= 1000) {
    prevChangeMillis = nowMillis;

    // Update the display
    uint8_t j = digitIndex;
    for (uint8_t i = 0; i < NUM_DIGITS; ++i) {
      // Write a decimal point every other digit, for demo purposes.
      uint8_t pattern = PATTERNS[j] | ((j & 0x1) ? 0x80 : 0x00);
      ledModule.setPatternAt(i, pattern);
      incrementMod(j, (uint8_t) NUM_DIGITS);
    }
    incrementMod(digitIndex, (uint8_t) NUM_DIGITS);

    // Update the brightness. The HT16K33 has 16 levels of brightness (0-15).
    ledModule.setBrightness(brightness);
    incrementMod(brightness, (uint8_t) 16);
  }
}

// Every 100 ms, unconditionally flush() to the LED module which updates all
// digits.
void flushModule() {
  static uint16_t prevFlushMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevFlushMillis) >= 100) {
    prevFlushMillis = nowMillis;

    // Flush incrementally, and measure the time.
    uint16_t startMicros = micros();
    ledModule.flush();
    uint16_t elapsedMicros = (uint16_t) micros() - startMicros;
    stats.update(elapsedMicros);
  }
}

// Every 5 seconds, print stats about how long flush() or flushIncremental()
// took.
void printStats() {
#if ENABLE_SERIAL_DEBUG >= 1
  static uint16_t prevStatsMillis;

  // Every 5 seconds, print out the statistics.
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

//-----------------------------------------------------------------------------

void setup() {
  delay(1000);

#if ENABLE_SERIAL_DEBUG >= 1
  Serial.begin(115200);
  while (!Serial);
#endif

  setupAceSegment();
}

void loop() {
  updateDisplay();

  flushModule();

  printStats();
}
