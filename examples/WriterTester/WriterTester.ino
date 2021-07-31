/*
 * This program is used to test the AceSegment feature matrix composed of:
 *
 *    * varous AceSegment writer classes
 *    * various supported microcontrollers (e.g. AVR, STM32, ESP8266, etc),
 *    * various supported LED modules (TM1637, MAX7219, 74HC595).
 *
 * This program is primarily intended for my own testing and debugging. The
 * other demo programs under the examples/ directory are better suited for
 * demonstration purposes. The program contains series of demo-reels. Each demo
 * reel loops continously, for example, incrementing a number from -99 to 1000.
 * Each increment is a "frame".
 *
 * The program depends on the AceButton library to handle 2 buttons, let's call
 * them "Mode" and "Change", to select the features that I want to test. Some of
 * my development boards use buttons wired to digital pins. Others use buttons
 * connected to an analog pin through a resistor ladder. This program supports
 * both configurations.
 *
 * The "Mode" button controls the progression of the demo-reel. Clicking on
 * "Mode" button button stops the auto-play of the demo reel at a specific
 * frame. Clicking on the "Mode" button again in the paused mode causes a
 * single-step through the demo-reel, frame by frame. For example, a number is
 * incremented by one each time the "Mode" button is pressed. To revert back
 * auto-play mode, perform a LongPress of the "Mode" button by holding it down
 * for more than 1 second. When the demo-reel is in the auto-play mode,
 * performing a LongPress changes to the next demo-reel. For example, it goes
 * from incrementing the temperature in degrees Celcius to incrementing the
 * temperature in degrees Fahrenheit.
 *
 * The "Change" button controls the rendering of the LED segment patterns to the
 * LED module. Clicking on the "Change" button pauses the auto rendering of the
 * display. Clicking on the "Change" button again performs a single-step through
 * each iteration of the LED module rendering logic, i.e. the
 * Tm1637Module::flush(), Tm1637Module::flushIncremental(),
 * Max7219Module::flush(), or Hc595Module::renderFieldNow() methods. While in
 * single-step mode, performing a LongPress on "Change" causes the program to
 * revert back to auto-rendering mode.
 */

#include <Arduino.h>
#include <SPI.h>
#include <AceButton.h>
#include <AceCommon.h> // incrementMod()
#include <AceSPI.h>
#include <AceTMI.h>
#include <AceWire.h>
#include <AceSegment.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  #include <digitalWriteFast.h>
  #include <ace_spi/SimpleSpiFastInterface.h>
  #include <ace_spi/HardSpiFastInterface.h>
  #include <ace_tmi/SimpleTmiFastInterface.h>
  #include <ace_wire/SimpleWireFastInterface.h>
  #include <ace_segment/direct/DirectFast4Module.h>
  using ace_tmi::SimpleTmiFastInterface;
  using ace_spi::HardSpiFastInterface;
  using ace_spi::SimpleSpiFastInterface;
  using ace_wire::SimpleWireFastInterface;
  using ace_segment::DirectFast4Module;
#endif

using ace_common::incrementMod;
using ace_button::AceButton;
using ace_button::ButtonConfig;
using ace_button::LadderButtonConfig;
using ace_spi::HardSpiInterface;
using ace_spi::SimpleSpiInterface;
using ace_tmi::SimpleTmiInterface;
using ace_wire::TwoWireInterface;
using ace_wire::SimpleWireInterface;
using ace_segment::DirectModule;
using ace_segment::HybridModule;
using ace_segment::Hc595Module;
using ace_segment::Tm1637Module;
using ace_segment::Max7219Module;
using ace_segment::Ht16k33Module;
using ace_segment::PatternWriter;
using ace_segment::CharWriter;
using ace_segment::NumberWriter;
using ace_segment::ClockWriter;
using ace_segment::TemperatureWriter;
using ace_segment::StringWriter;
using ace_segment::StringScroller;
using ace_segment::LevelWriter;
using ace_segment::kDigitRemapArray8Max7219;
using ace_segment::kDigitRemapArray8Hc595;
using ace_segment::kByteOrderSegmentHighDigitLow;
using ace_segment::kByteOrderDigitHighSegmentLow;
using ace_segment::kActiveLowPattern;
using ace_segment::kActiveHighPattern;

#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

// Use polling or interrupt.
#define USE_INTERRUPT 0

#if USE_INTERRUPT
  #include <TimerOne.h>
#endif

// Type of LED Module
#define LED_DISPLAY_TYPE_SCANNING 0
#define LED_DISPLAY_TYPE_TM1637 1
#define LED_DISPLAY_TYPE_MAX7219 2
#define LED_DISPLAY_TYPE_HC595 3
#define LED_DISPLAY_TYPE_HT16K33 4
#define LED_DISPLAY_TYPE_DIRECT 5
#define LED_DISPLAY_TYPE_HYBRID 6
#define LED_DISPLAY_TYPE_FULL 7 /* Rename to CUSTOM_HC595? */

// Used by LED_DISPLAY_TYPE_DIRECT
#define DIRECT_INTERFACE_TYPE_NORMAL 0
#define DIRECT_INTERFACE_TYPE_FAST_4 1

// Methods of communication to the LED module (SPI, I2C, TM1637).
#define INTERFACE_TYPE_SIMPLE_SPI 0
#define INTERFACE_TYPE_SIMPLE_SPI_FAST 1
#define INTERFACE_TYPE_HARD_SPI 2
#define INTERFACE_TYPE_HARD_SPI_FAST 3
#define INTERFACE_TYPE_SIMPLE_TMI 4
#define INTERFACE_TYPE_SIMPLE_TMI_FAST 5
#define INTERFACE_TYPE_TWO_WIRE 6
#define INTERFACE_TYPE_SIMPLE_WIRE 7
#define INTERFACE_TYPE_SIMPLE_WIRE_FAST 8

// Some microcontrollers have 2 or more SPI buses. PRIMARY selects the default.
// SECONDARY selects the alternate. I don't have a board with more than 2, but
// we could add additional options here if needed.
#define SPI_INSTANCE_TYPE_PRIMARY 0
#define SPI_INSTANCE_TYPE_SECONDARY 1

// Button options: either digital buttons using ButtonConfig, 2 analog buttons
// using LadderButtonConfig, or 4 analog buttons using LadderButtonConfig:
//  * AVR: 10-bit analog pin
//  * ESP8266: 10-bit analog pin
//  * ESP32: 12-bit analog pin
#define BUTTON_TYPE_DIGITAL 0
#define BUTTON_TYPE_ANALOG 1

// Select the TM1637Module flush() method
#define TM_FLUSH_METHOD_NORMAL 0
#define TM_FLUSH_METHOD_INCREMENTAL 1

//------------------------------------------------------------------
// Hardware configuration.
//------------------------------------------------------------------

// Configuration for Arduino IDE
#if ! defined(EPOXY_DUINO) && ! defined(AUNITER)
  #define AUNITER_MICRO_TM1637
#endif

#if defined(EPOXY_DUINO)
  // For EpoxyDuino, the actual numbers don't matter, so let's set them to (2,3)
  // since I'm not sure if A2 and A3 are defined.
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_DIRECT
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;

  // Choose one of the following variants:
  //#define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_NORMAL
  #define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_FAST
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_ATTINY_HT16K33)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  #define MODE_BUTTON_PIN 0
  #define CHANGE_BUTTON_PIN 1
  #define ANALOG_BUTTON_PIN A0
  // Resistor ladder must remain above 90% VCC because they are connected to
  // the RESET button. We have 3 resistors (1k, 10k, 22k):
  //    * 933: 10k/(11k+1k) = 90.9%
  //    * 979: 22k/23k = 95.6%
  //    * 1023: 100%, open
  // Numbers then adjusted using LadderButtonCalibrator.
  #define ANALOG_BUTTON_LEVELS {933, 980, 1024}

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HT16K33
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_TWO_WIRE
  const uint8_t SDA_PIN = SDA;
  const uint8_t SCL_PIN = SCL;
  const uint8_t DELAY_MICROS = 1;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

// The Arduino Nano dev box was upgraded to support a HT16K33 LED module so that
// AceSegment/AutoBenchmark can generate correct benchmark for the
// TwoWireInterface<TwoWire> interface class. If an actual HT16K33 is not
// connected to the I2C, the hardware <Wire.h> library reads a NACK from the
// slave device and bails out early from the endTransmission(), producing timing
// information which are too short.
#elif defined(AUNITER_NANO_HT16K33)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = 2;
  const uint8_t CHANGE_BUTTON_PIN = 3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HT16K33
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_TWO_WIRE
  const uint8_t SDA_PIN = SDA;
  const uint8_t SCL_PIN = SCL;
  const uint8_t DELAY_MICROS = 1;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

// Pro Micro dev board digital buttons can be configured to pins (A2, A3)
// or (2, 3) via DIP switches. Since (2, 3) are used
// by I2C, be sure to use (A2, A3) when using I2C devices.
#elif defined(AUNITER_MICRO_CUSTOM_DIRECT)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_DIRECT
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;

  // Choose one of the following variants:
  //#define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_NORMAL
  #define DIRECT_INTERFACE_TYPE DIRECT_INTERFACE_TYPE_FAST
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

#elif defined(AUNITER_MICRO_CUSTOM_SINGLE)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HYBRID
  const uint8_t NUM_DIGITS = 4;
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_CUSTOM_DUAL)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_FULL
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;

#elif defined(AUNITER_MICRO_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_TMI
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_TMI_FAST
  const uint8_t DIO_PIN = 9;
  const uint8_t CLK_PIN = A0;
  const uint8_t DELAY_MICROS = 100;

  // Select one of the flush methods.
  //#define TM_FLUSH_METHOD TM_FLUSH_METHOD_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

#elif defined(AUNITER_MICRO_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = 10;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_MICRO_HT16K33)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = A2;
  const uint8_t CHANGE_BUTTON_PIN = A3;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HT16K33
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_TWO_WIRE
  const uint8_t SDA_PIN = SDA;
  const uint8_t SCL_PIN = SCL;
  const uint8_t DELAY_MICROS = 1;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

#elif defined(AUNITER_SAMD_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = 8;
  const uint8_t CHANGE_BUTTON_PIN = 9;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_TMI
  const uint8_t DIO_PIN = 11;
  const uint8_t CLK_PIN = 13;
  const uint8_t DELAY_MICROS = 100;

  // Select one of the flush methods.
  //#define TM_FLUSH_METHOD TM_FLUSH_METHOD_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

#elif defined(AUNITER_SAMD_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = 8;
  const uint8_t CHANGE_BUTTON_PIN = 9;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_SAMD_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = 8;
  const uint8_t CHANGE_BUTTON_PIN = 9;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI_FAST
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_SAMD_HT16K33)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = 8;
  const uint8_t CHANGE_BUTTON_PIN = 9;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HT16K33
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_TWO_WIRE
  const uint8_t SDA_PIN = SDA;
  const uint8_t SCL_PIN = SCL;
  const uint8_t DELAY_MICROS = 1;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

#elif defined(AUNITER_STM32_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = PA0;
  const uint8_t CHANGE_BUTTON_PIN = PA1;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_TMI
  const uint8_t DIO_PIN = PB4;
  const uint8_t CLK_PIN = PB3;
  const uint8_t DELAY_MICROS = 100;

  // Select one of the flush methods.
  //#define TM_FLUSH_METHOD TM_FLUSH_METHOD_NORMAL
  #define TM_FLUSH_METHOD TM_FLUSH_METHOD_INCREMENTAL

#elif defined(AUNITER_STM32_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = PA0;
  const uint8_t CHANGE_BUTTON_PIN = PA1;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // SPI1 pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
    SPIClass& spiInstance = SPI;
  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // SPI2 pins
    const uint8_t LATCH_PIN = PB12;
    const uint8_t DATA_PIN = PB15;
    const uint8_t CLOCK_PIN = PB13;
    SPIClass spiInstance(DATA_PIN, PB14 /*miso*/, CLOCK_PIN);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#elif defined(AUNITER_STM32_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = PA0;
  const uint8_t CHANGE_BUTTON_PIN = PA1;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_PRIMARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // SPI1 pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
    SPIClass& spiInstance = SPI;
  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // SPI2 pins
    const uint8_t LATCH_PIN = PB12;
    const uint8_t DATA_PIN = PB15;
    const uint8_t CLOCK_PIN = PB13;
    SPIClass spiInstance(DATA_PIN, PB14 /*miso*/, CLOCK_PIN);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#elif defined(AUNITER_STM32_HT16K33)
  #define BUTTON_TYPE BUTTON_TYPE_DIGITAL
  const uint8_t MODE_BUTTON_PIN = PA0;
  const uint8_t CHANGE_BUTTON_PIN = PA1;

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HT16K33
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_TWO_WIRE
  const uint8_t SDA_PIN = SDA;
  const uint8_t SCL_PIN = SCL;
  const uint8_t DELAY_MICROS = 1;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

#elif defined(AUNITER_D1MINI_LARGE_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 2;
  #define ANALOG_BUTTON_PIN A0
  #define ANALOG_BUTTON_LEVELS { \
      0 /*short to ground*/, \
      327 /*32%, 4.7k*/, \
      512 /*50%, 10k*/, \
      844 /*82%, 47k*/, \
      1023 /*100%, open*/ \
    }

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_TMI
  const uint8_t DIO_PIN = D7;
  const uint8_t CLK_PIN = D5;
  const uint8_t DELAY_MICROS = 100;

#elif defined(AUNITER_D1MINI_LARGE_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 2;
  #define ANALOG_BUTTON_PIN A0
  #define ANALOG_BUTTON_LEVELS { \
      0 /*short to ground*/, \
      327 /*32%, 4.7k*/, \
      512 /*50%, 10k*/, \
      844 /*82%, 47k*/, \
      1023 /*100%, open*/ \
    }

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  //#define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_D1MINI_LARGE_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 2;
  #define ANALOG_BUTTON_PIN A0
  #define ANALOG_BUTTON_LEVELS { \
      0 /*short to ground*/, \
      327 /*32%, 4.7k*/, \
      512 /*50%, 10k*/, \
      844 /*82%, 47k*/, \
      1023 /*100%, open*/ \
    }

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  const uint8_t LATCH_PIN = SS;
  const uint8_t DATA_PIN = MOSI;
  const uint8_t CLOCK_PIN = SCK;
  SPIClass& spiInstance = SPI;

#elif defined(AUNITER_D1MINI_LARGE_HT16K33)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 2;
  #define ANALOG_BUTTON_PIN A0
  #define ANALOG_BUTTON_LEVELS { \
      0 /*short to ground*/, \
      327 /*32%, 4.7k*/, \
      512 /*50%, 10k*/, \
      844 /*82%, 47k*/, \
      1023 /*100%, open*/ \
    }

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HT16K33
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_TWO_WIRE
  const uint8_t SDA_PIN = SDA;
  const uint8_t SCL_PIN = SCL;
  const uint8_t DELAY_MICROS = 1;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

#elif defined(AUNITER_ESP32_TM1637)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 1;
  #define ANALOG_BUTTON_LEVELS {0, 2048, 4095}
  #define ANALOG_BUTTON_PIN A10

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_TM1637
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  #define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_TMI
  const uint8_t DIO_PIN = 13;
  const uint8_t CLK_PIN = 14;
  const uint8_t DELAY_MICROS = 100;

#elif defined(AUNITER_ESP32_MAX7219)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 1;
  #define ANALOG_BUTTON_LEVELS {0, 2048, 4095}
  #define ANALOG_BUTTON_PIN A10

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_MAX7219
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  // My dev board uses HSPI, which is the Secondary SPI.
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_SECONDARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // VSPI pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
    SPIClass& spiInstance = SPI;
  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // HSPI pins
    const uint8_t LATCH_PIN = 15;
    const uint8_t DATA_PIN = 13;
    const uint8_t CLOCK_PIN = 14;
    SPIClass spiInstance(HSPI);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#elif defined(AUNITER_ESP32_HC595)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 1;
  #define ANALOG_BUTTON_LEVELS {0, 2048, 4095}
  #define ANALOG_BUTTON_PIN A10

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HC595
  const uint8_t NUM_DIGITS = 8;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_SPI
  #define INTERFACE_TYPE INTERFACE_TYPE_HARD_SPI
  // My dev board uses HSPI.
  #define SPI_INSTANCE_TYPE SPI_INSTANCE_TYPE_SECONDARY

  #if SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_PRIMARY
    // VSPI pins (default)
    const uint8_t LATCH_PIN = SS;
    const uint8_t DATA_PIN = MOSI;
    const uint8_t CLOCK_PIN = SCK;
    SPIClass& spiInstance = SPI;

  #elif SPI_INSTANCE_TYPE == SPI_INSTANCE_TYPE_SECONDARY
    // HSPI pins
    const uint8_t LATCH_PIN = 15;
    const uint8_t DATA_PIN = 13;
    const uint8_t CLOCK_PIN = 14;
    SPIClass spiInstance(HSPI);
  #else
    #error Unknown SPI_INSTANCE_TYPE
  #endif

#elif defined(AUNITER_ESP32_HT16K33)
  #define BUTTON_TYPE BUTTON_TYPE_ANALOG
  const uint8_t MODE_BUTTON_PIN = 0;
  const uint8_t CHANGE_BUTTON_PIN = 1;
  #define ANALOG_BUTTON_LEVELS {0, 2048, 4095}
  #define ANALOG_BUTTON_PIN A10

  #define LED_DISPLAY_TYPE LED_DISPLAY_TYPE_HT16K33
  const uint8_t NUM_DIGITS = 4;

  // Choose one of the following variants:
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE
  //#define INTERFACE_TYPE INTERFACE_TYPE_SIMPLE_WIRE_FAST
  #define INTERFACE_TYPE INTERFACE_TYPE_TWO_WIRE
  const uint8_t SDA_PIN = SDA;
  const uint8_t SCL_PIN = SCL;
  const uint8_t DELAY_MICROS = 1;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

#else
  #error Unknown AUNITER environment
#endif

//------------------------------------------------------------------
// Configurations for AceSegment
//------------------------------------------------------------------

// Total fields/second
//      = FRAMES_PER_SECOND * NUM_SUBFIELDS * NUM_DIGITS
//      = 60 * 1 * 4
//      = 240 fields/sec
//      => 4167 micros/field
const uint8_t FRAMES_PER_SECOND = 60;
const uint8_t NUM_SUBFIELDS = 1;

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_TM1637
  #if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_TMI
    using TmiInterface = SimpleTmiInterface;
    TmiInterface tmiInterface(DIO_PIN, CLK_PIN, DELAY_MICROS);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_TMI_FAST
    using TmiInterface = SimpleTmiFastInterface<DIO_PIN, CLK_PIN, DELAY_MICROS>;
    TmiInterface tmiInterface;
  #endif
  Tm1637Module<TmiInterface, NUM_DIGITS> ledModule(tmiInterface);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_MAX7219
  #if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(spiInstance, LATCH_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(spiInstance);
  #endif
  Max7219Module<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface, kDigitRemapArray8Max7219);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595
  // Common Anode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(spiInstance, LATCH_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(spiInstance);
  #endif

  Hc595Module<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderSegmentHighDigitLow,
      kDigitRemapArray8Hc595
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HT16K33
  #if INTERFACE_TYPE == INTERFACE_TYPE_TWO_WIRE
    // On AVR, simply including <Wire.h> consumes an extra 1700 bytes of flash
    // so include this only when required.
    #include <Wire.h>
    using WireInterface = TwoWireInterface<TwoWire>;
    WireInterface wireInterface(Wire);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_WIRE
    using WireInterface = SimpleWireInterface;
    WireInterface wireInterface(SDA_PIN, SCL_PIN, DELAY_MICROS);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_WIRE_FAST
    using WireInterface = SimpleWireFastInterface<
        SDA_PIN, SCL_PIN, DELAY_MICROS>;
    WireInterface wireInterface;
  #endif
  Ht16k33Module<WireInterface, NUM_DIGITS> ledModule(
      wireInterface, HT16K33_I2C_ADDRESS);

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  // Common Anode, with transitors on Group pins
  #if DIRECT_INTERFACE_TYPE == DIRECT_INTERFACE_TYPE_NORMAL
    DirectModule<NUM_DIGITS> ledModule(
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        SEGMENT_PINS,
        DIGIT_PINS);
  #else
    DirectFast4Module<
        8, 9, 10, 16, 14, 18, 19, 15, // segment pins
        4, 5, 6, 7, // digit pins
        NUM_DIGITS
    > ledModule(
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND);
  #endif

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID
  // Common Cathode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(SPI);
  #endif
  HybridModule<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface,
      kActiveHighPattern /*segmentOnPattern*/,
      kActiveHighPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      DIGIT_PINS
  );

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL
  // Common Anode, with transistors on Group pins
  #if INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_SIMPLE_SPI_FAST
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);
  #elif INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface;
  #endif

  Hc595Module<SpiInterface, NUM_DIGITS> ledModule(
      spiInterface,
      kActiveLowPattern /*segmentOnPattern*/,
      kActiveLowPattern /*digitOnPattern*/,
      FRAMES_PER_SECOND,
      kByteOrderDigitHighSegmentLow,
      nullptr /*remapArray*/
  );

#else
  #error Unknown LED_DISPLAY_TYPE
#endif

PatternWriter patternWriter(ledModule);
NumberWriter numberWriter(ledModule);
ClockWriter clockWriter(ledModule);
TemperatureWriter temperatureWriter(ledModule);
CharWriter charWriter(ledModule);
StringWriter stringWriter(charWriter);
StringScroller stringScroller(charWriter);
LevelWriter levelWriter(ledModule);

// Setup the various resources.
void setupAceSegment() {

#if INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI \
    || INTERFACE_TYPE == INTERFACE_TYPE_HARD_SPI_FAST
  spiInstance.begin();
#elif INTERFACE_TYPE == INTERFACE_TYPE_TWO_WIRE
  Wire.begin();
#endif

#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_TM1637
  tmiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(2); // 1-7

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_MAX7219
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(2); // 0-15

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HT16K33
  wireInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-15

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#elif LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL
  spiInterface.begin();
  ledModule.begin();
  ledModule.setBrightness(1); // 0-1

#else
  #error Unknown LED_DISPLAY_TYPE

#endif
}

#if USE_INTERRUPT == 1
void renderNow() {
  ledModule.renderFieldNow();
}

void setupInterupt() {
  Timer1.initialize(ledModule.getMicrosPerField());
  Timer1.attachInterrupt(renderNow);
}
#endif

//------------------------------------------------------------------
// Configurations for various demo-reels.
//------------------------------------------------------------------

// State of loop, whether paused or not.
const uint8_t DEMO_LOOP_MODE_AUTO = 0;
const uint8_t DEMO_LOOP_MODE_PAUSED = 1;
uint8_t demoLoopMode = DEMO_LOOP_MODE_AUTO;

// A DEMO_MODE represents a demo-reel that demonstrates a specific AceSegment
// feature.
const uint8_t DEMO_MODE_HEX_NUMBERS = 0;
const uint8_t DEMO_MODE_CLOCK = 1;
const uint8_t DEMO_MODE_UNSIGNED_DEC_NUMBERS = 2;
const uint8_t DEMO_MODE_SIGNED_DEC_NUMBERS = 3;
const uint8_t DEMO_MODE_TEMPERATURE_C = 4;
const uint8_t DEMO_MODE_TEMPERATURE_F = 5;
const uint8_t DEMO_MODE_CHAR = 6;
const uint8_t DEMO_MODE_STRINGS = 7;
const uint8_t DEMO_MODE_SCROLL = 8;
const uint8_t DEMO_MODE_SPIN = 9;
const uint8_t DEMO_MODE_SPIN_2 = 10;
const uint8_t DEMO_MODE_LEVEL = 11;
const uint8_t DEMO_MODE_COUNT = 12;
uint8_t demoMode = DEMO_MODE_HEX_NUMBERS;
uint8_t prevDemoMode = demoMode - 1;

static const uint16_t DEMO_INTERNAL_DELAY[DEMO_MODE_COUNT] = {
  10, // DEMO_MODE_HEX_NUMBERS
  20, // DEMO_MODE_CLOCK
  10, // DEMO_MODE_UNSIGNED_DEC_NUMBERS
  10, // DEMO_MODE_SIGNED_DEC_NUMBERS
  100, // DEMO_MODE_TEMPERATURE_C
  100, // DEMO_MODE_TEMPERATURE_F
  200, // DEMO_MODE_CHAR
  500, // DEMO_MODE_STRINGS
  300, // DEMO_MODE_SCROLL
  100, // DEMO_MODE_SPIN
  100, // DEMO_MODE_SPIN2
  200, // DEMO_MODE_LEVEL
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

void writeTemperatureC() {
  static int16_t w = -9;

  temperatureWriter.writeTempDegCAt(0, w, 4);
  w++;
  if (w > 99) w = -9;
}

//-----------------------------------------------------------------------------

void writeTemperatureF() {
  static int16_t w = -9;

  temperatureWriter.writeTempDegFAt(0, w, 4);
  w++;
  if (w > 99) w = -9;
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

  uint8_t numChars = charWriter.getNumChars();
  if (numChars == 0) { // 0 means 256
    b++;
  } else {
    incrementMod(b, numChars);
  }
}

//-----------------------------------------------------------------------------

void writeStrings() {
  // These are defined in the function because the F() works only inside
  // functions.
  static const __FlashStringHelper* STRINGS[] = {
    F("0123"),
    F("0.123"),
    F("0.1 "),
    F("0.1.2.3."),
    F("a.b.c.d"),
    F(".1.2.3"),
    F("brian"),
  };
  static const uint8_t NUM_STRINGS = sizeof(STRINGS) / sizeof(STRINGS[0]);

  static uint8_t i = 0;

  uint8_t written = stringWriter.writeStringAt(0, STRINGS[i]);
  stringWriter.clearToEnd(written);

  incrementMod(i, NUM_STRINGS);
}

//-----------------------------------------------------------------------------

static const char SCROLL_STRING[] PROGMEM =
"the quick brown fox jumps over the lazy dog, "
"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG. "
"[0123456789]";

void scrollString() {
  static bool isInit = false;
  static bool scrollLeft = true;

  if (! isInit) {
    if (scrollLeft) {
      stringScroller.initScrollLeft(
          (const __FlashStringHelper*) SCROLL_STRING);
    } else {
      stringScroller.initScrollRight(
          (const __FlashStringHelper*) SCROLL_STRING);
    }
    isInit = true;
  }

  bool isDone;
  if (scrollLeft) {
    isDone = stringScroller.scrollLeft();
  } else {
    isDone = stringScroller.scrollRight();
  }

  if (isDone) {
    scrollLeft = !scrollLeft;
    isInit = false;
  }
}

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
  patternWriter.writePatternsAt_P(0, patterns, 4);

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
  patternWriter.writePatternsAt_P(0, patterns, 4 /*len*/);

  incrementMod(i, NUM_SPIN_PATTERNS_2);
}

//-----------------------------------------------------------------------------

void writeLevels() {
  static uint8_t level;

  // Do a random walk, with reflection on the left and right boundaries.
  uint8_t maxLevel = levelWriter.getMaxLevel();
  uint8_t delta = random(3);
  level += delta - 1;
  if (level == 0xff) {
    level = 1;
  } else if (level > maxLevel) {
    level = maxLevel - 1;
  }

  levelWriter.writeLevel(level);
}

//-----------------------------------------------------------------------------

/** Display the demo pattern selected by demoMode. */
void updateDemo() {
  if (demoMode == DEMO_MODE_HEX_NUMBERS) {
    writeHexNumbers();
  } else if (demoMode == DEMO_MODE_CLOCK) {
    writeClock();
  } else if (demoMode == DEMO_MODE_UNSIGNED_DEC_NUMBERS) {
    writeUnsignedDecNumbers();
  } else if (demoMode == DEMO_MODE_SIGNED_DEC_NUMBERS) {
    writeSignedDecNumbers();
  } else if (demoMode == DEMO_MODE_TEMPERATURE_C) {
    writeTemperatureC();
  } else if (demoMode == DEMO_MODE_TEMPERATURE_F) {
    writeTemperatureF();
  } else if (demoMode == DEMO_MODE_CHAR) {
    writeChars();
  } else if (demoMode == DEMO_MODE_STRINGS) {
    writeStrings();
  } else if (demoMode == DEMO_MODE_SCROLL) {
    scrollString();
  } else if (demoMode == DEMO_MODE_SPIN) {
    spinDisplay();
  } else if (demoMode == DEMO_MODE_SPIN_2) {
    spinDisplay2();
  } else if (demoMode == DEMO_MODE_LEVEL) {
    writeLevels();
  }
}

/** Go to the next demo. */
void nextDemo() {
// The HT16K33 module can display both a decimal point and a colon segment after
// digit 1 (second from left). The Ht16k33Module.enableColon() selects one or
// the other.
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HT16K33
  if (prevDemoMode == DEMO_MODE_CLOCK) {
    ledModule.enableColon(false);
  }
#endif
  prevDemoMode = demoMode;

  incrementMod(demoMode, DEMO_MODE_COUNT);
#if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HT16K33
  if (demoMode == DEMO_MODE_CLOCK) {
    ledModule.enableColon(true);
  }
#endif

  patternWriter.clear();

  updateDemo();
}

/** Loop within a single demo. */
void demoLoop() {
  static uint16_t lastUpdateMillis = millis();

  uint16_t demoInternalDelay = DEMO_INTERNAL_DELAY[demoMode];
  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - lastUpdateMillis) >= demoInternalDelay) {
    lastUpdateMillis = nowMillis;
    if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
      updateDemo();
    }
  }
}

void renderField() {
  #if LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_DIRECT \
      || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HYBRID \
      || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_FULL \
      || LED_DISPLAY_TYPE == LED_DISPLAY_TYPE_HC595
    ledModule.renderFieldWhenReady();
  #else
    #if TM_FLUSH_METHOD == TM_FLUSH_METHOD_NORMAL
      ledModule.flush();
    #elif TM_FLUSH_METHOD == TM_FLUSH_METHOD_INCREMENTAL
      ledModule.flushIncremental();
    #endif
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

#if BUTTON_TYPE == BUTTON_TYPE_DIGITAL

  ButtonConfig buttonConfig;
  AceButton modeButton(&buttonConfig, MODE_BUTTON_PIN);
  AceButton changeButton(&buttonConfig, CHANGE_BUTTON_PIN);

#elif BUTTON_TYPE == BUTTON_TYPE_ANALOG

  AceButton modeButton((uint8_t) MODE_BUTTON_PIN);
  AceButton changeButton((uint8_t) CHANGE_BUTTON_PIN);
  AceButton* const BUTTONS[] = {&modeButton, &changeButton};
  const uint16_t LEVELS[] = ANALOG_BUTTON_LEVELS;

  LadderButtonConfig buttonConfig(
      ANALOG_BUTTON_PIN,
      sizeof(LEVELS) / sizeof(LEVELS[0]),
      LEVELS,
      sizeof(BUTTONS) / sizeof(BUTTONS[0]),
      BUTTONS
  );

#else

  #error Unknown BUTTON_TYPE

#endif

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  (void) buttonState;

  uint8_t pin = button->getPin();
  if (pin == MODE_BUTTON_PIN) {
    switch (eventType) {
      case AceButton::kEventReleased:
        // Single click pauses the demo-reel.
        if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
          demoLoopMode = DEMO_LOOP_MODE_PAUSED;
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo loop paused"));
          }
        // If already paused, single-step to the next demo frame.
        } else if (demoLoopMode == DEMO_LOOP_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo stepped"));
          }
          updateDemo();
        }
        break;

      case AceButton::kEventLongPressed:
        // Long press goes back to auto loop mode if paused.
        if (demoLoopMode == DEMO_LOOP_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): demo loop enabled"));
          }
          demoLoopMode = DEMO_LOOP_MODE_AUTO;
        // If currently in auto mode, goes to the next demo reel.
        } else if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
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
        // Single click pauses the rendering/flushing.
        if (renderMode == RENDER_MODE_AUTO) {
        #if USE_INTERRUPT
          Timer1.stop();
        #endif
          renderMode = RENDER_MODE_PAUSED;
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): paused"));
          }
        // If already paused, single click single-steps through the rendering.
        } else if (renderMode == RENDER_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): stepping"));
          }
          singleStep();
        }
        break;

      case AceButton::kEventLongPressed:
        // LongPress resumes auto rendering/flushing.
        if (renderMode == RENDER_MODE_PAUSED) {
          if (ENABLE_SERIAL_DEBUG >= 1) {
            Serial.println(F("handleEvent(): switching to auto rendering"));
          }
        #if USE_INTERRUPT
          Timer1.start();
        #endif
          renderMode = RENDER_MODE_AUTO;
        }
        break;
    }
  }
}

void setupAceButton() {
#if BUTTON_TYPE == BUTTON_TYPE_DIGITAL
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CHANGE_BUTTON_PIN, INPUT_PULLUP);
#endif

  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
}

// Check AceButtons, limiting sampling rate to about 200/seconds to avoid
// problems on ESP8266 using analogRead().
void checkButtons() {
  static uint16_t prevMillis;

  uint16_t nowMillis = millis();
  if ((uint16_t) (nowMillis - prevMillis) >= 5) {
    prevMillis = nowMillis;

  #if BUTTON_TYPE == BUTTON_TYPE_DIGITAL
    modeButton.check();
    changeButton.check();
  #else
    buttonConfig.checkButtons();
  #endif
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

  setupAceButton();
  setupAceSegment();
#if USE_INTERRUPT
  setupInterupt();
#endif

  if (ENABLE_SERIAL_DEBUG >= 1) {
    Serial.println(F("setup(): end"));
  }

  updateDemo();
}

void loop() {
#if ! USE_INTERRUPT
  if (renderMode == RENDER_MODE_AUTO) {
    renderField();
  }
#endif

  if (demoLoopMode == DEMO_LOOP_MODE_AUTO) {
    demoLoop();
  }

  checkButtons();
}
