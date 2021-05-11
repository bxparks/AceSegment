/*
 * A program which compiles various LedModule and Writer objects configured with
 * using different LED configurations to determine the flash and static memory
 * sizes from the output of the compiler. Set the FEATURE macro to various
 * integer to compile different algorithms.
 */

#include <Arduino.h>

// DO NOT MODIFY THIS LINE. This will be overwritten by collect.sh on each
// iteration, incrementing from 0 to 5. The Arduino IDE will compile the
// program, then the script will extract the flash and static memory usage
// numbers printed out by the Arduino compiler. The numbers will be printed on
// the STDOUT, which then can be saved to a file specific for a particular
// hardware platform, e.g. "nano.txt" or "esp8266.txt".
#define FEATURE 0

// List of features of AceSegment that we want to gather memory usage numbers.
#define FEATURE_BASELINE 0
#define FEATURE_DIRECT_MODULE 1
#define FEATURE_DIRECT_FAST4_MODULE 2
#define FEATURE_SINGLE_HC595_SOFT_SPI 3
#define FEATURE_SINGLE_HC595_SOFT_SPI_FAST 4
#define FEATURE_SINGLE_HC595_HARD_SPI 5
#define FEATURE_SINGLE_HC595_HARD_SPI_FAST 6
#define FEATURE_DUAL_HC595_SOFT_SPI 7
#define FEATURE_DUAL_HC595_SOFT_SPI_FAST 8
#define FEATURE_DUAL_HC595_HARD_SPI 9
#define FEATURE_DUAL_HC595_HARD_SPI_FAST 10
#define FEATURE_TM1637_WIRE 11
#define FEATURE_TM1637_WIRE_FAST 12
#define FEATURE_MAX7219_SOFT_SPI 13
#define FEATURE_MAX7219_SOFT_SPI_FAST 14
#define FEATURE_MAX7219_HARD_SPI 15
#define FEATURE_MAX7219_HARD_SPI_FAST 16
#define FEATURE_STUB_MODULE 17
#define FEATURE_NUMBER_WRITER 18
#define FEATURE_CLOCK_WRITER 19
#define FEATURE_TEMPERATURE_WRITER 20
#define FEATURE_CHAR_WRITER 21
#define FEATURE_STRING_WRITER 22

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE > FEATURE_BASELINE
  #include <AceSegment.h>
  #if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
    #include <digitalWriteFast.h>
    #include <ace_segment/hw/SoftSpiFastInterface.h>
    #include <ace_segment/hw/HardSpiFastInterface.h>
    #include <ace_segment/hw/SoftWireFastInterface.h>
    #include <ace_segment/direct/DirectFast4Module.h>
  #endif
  using namespace ace_segment;

  // Common to all FEATURES
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t FRAMES_PER_SECOND = 60;
  const uint8_t NUM_SUBFIELDS = 1;

  // Direct
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

  // 74HC595
  const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
  const uint8_t DATA_PIN = MOSI; // DS on 74HC595
  const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595

  // TM1637
  const uint8_t CLK_PIN = 16;
  const uint8_t DIO_PIN = 10;
  const uint16_t BIT_DELAY = 100;

  // A stub LedModule to allow various Writer classes to be created, but mostly
  // isolated from the underlying LedModule implementations.
  class StubModule : public LedModule {
    public:
      StubModule() : LedModule(NUM_DIGITS) {}

      void setBrightness(uint8_t brightness) override {
        disableCompilerOptimization = brightness;
      }

      void setPatternAt(uint8_t /*pos*/, uint8_t pattern) override {
        disableCompilerOptimization = pattern;
      }

      uint8_t getPatternAt(uint8_t /*pos*/) override {
        return disableCompilerOptimization;
      }
  };

  #if FEATURE == FEATURE_DIRECT_MODULE
    // Common Anode, with transitions on Group pins
    DirectModule<NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        SEGMENT_PINS,
        DIGIT_PINS);

  #elif FEATURE == FEATURE_DIRECT_FAST4_MODULE
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    DirectFast4Module<
        8, 9, 10, 16, 14, 18, 19, 15, // segment pins
        4, 5, 6, 7, // digit pins
        NUM_DIGITS,
        NUM_SUBFIELDS
    > scanningModule(
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_SINGLE_HC595_SOFT_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    SingleHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_SINGLE_HC595_SOFT_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    SingleHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_SINGLE_HC595_HARD_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    SingleHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_SINGLE_HC595_HARD_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    SingleHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveHighPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_DUAL_HC595_SOFT_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    DualHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND
    );

  #elif FEATURE == FEATURE_DUAL_HC595_SOFT_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    DualHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND
    );

  #elif FEATURE == FEATURE_DUAL_HC595_HARD_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    DualHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND
    );

  #elif FEATURE == FEATURE_DUAL_HC595_HARD_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    DualHc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        LedMatrixBase::kActiveLowPattern /*segmentOnPattern*/,
        LedMatrixBase::kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND
    );

  #elif FEATURE == FEATURE_TM1637_WIRE
    using WireInterface = SoftWireInterface;
    WireInterface wireInterface(CLK_PIN, DIO_PIN, BIT_DELAY);
    Tm1637Module<WireInterface, NUM_DIGITS> tm1637Module(wireInterface);

  #elif FEATURE == FEATURE_TM1637_WIRE_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using WireInterface = SoftWireFastInterface<CLK_PIN, DIO_PIN, BIT_DELAY>;
    WireInterface wireInterface;
    Tm1637Module<WireInterface, NUM_DIGITS> tm1637Module(wireInterface);

  #elif FEATURE == FEATURE_MAX7219_SOFT_SPI
    using SpiInterface = SoftSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_MAX7219_SOFT_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using SpiInterface = SoftSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_MAX7219_HARD_SPI
    using SpiInterface = HardSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_MAX7219_HARD_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using SpiInterface = HardSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_STUB_MODULE
    StubModule stubModule;
    LedDisplay ledDisplay(stubModule);

  #elif FEATURE == FEATURE_NUMBER_WRITER
    StubModule stubModule;
    LedDisplay ledDisplay(stubModule);
    NumberWriter numberWriter(ledDisplay);

  #elif FEATURE == FEATURE_CLOCK_WRITER
    StubModule stubModule;
    LedDisplay ledDisplay(stubModule);
    ClockWriter clockWriter(ledDisplay);

  #elif FEATURE == FEATURE_TEMPERATURE_WRITER
    StubModule stubModule;
    LedDisplay ledDisplay(stubModule);
    TemperatureWriter temperatureWriter(ledDisplay);

  #elif FEATURE == FEATURE_CHAR_WRITER
    StubModule stubModule;
    LedDisplay ledDisplay(stubModule);
    CharWriter charWriter(ledDisplay);

  #elif FEATURE == FEATURE_STRING_WRITER
    StubModule stubModule;
    LedDisplay ledDisplay(stubModule);
    CharWriter charWriter(ledDisplay);
    StringWriter stringWriter(charWriter);

  #endif
#endif

void setup() {
#if FEATURE == FEATURE_BASELINE
  disableCompilerOptimization = 3;

// In the following, I used to grab the output of patterns[] and write to
// disableCompilerOptimization to prevent the compiler from optimizing away the
// entire program. But after templatizing ScanningModule, pattterns variable is
// no longer accessible. But it does not matter because I realized that
// ScanningModule performs a digitalWrite(), which has the same effect of
// disabling optimizations.

#elif FEATURE == FEATURE_DIRECT_MODULE
  scanningModule.begin();

#elif FEATURE == FEATURE_DIRECT_FAST4_MODULE
  scanningModule.begin();

#elif FEATURE == FEATURE_SINGLE_HC595_SOFT_SPI
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_SINGLE_HC595_SOFT_SPI_FAST
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_SINGLE_HC595_HARD_SPI
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_SINGLE_HC595_HARD_SPI_FAST
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_DUAL_HC595_SOFT_SPI
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_DUAL_HC595_SOFT_SPI_FAST
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_DUAL_HC595_HARD_SPI
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_DUAL_HC595_HARD_SPI_FAST
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_TM1637_WIRE
  wireInterface.begin();
  tm1637Module.begin();

#elif FEATURE == FEATURE_TM1637_WIRE_FAST
  wireInterface.begin();
  tm1637Module.begin();

#elif FEATURE == FEATURE_MAX7219_SOFT_SPI
  spiInterface.begin();
  max7219Module.begin();

#elif FEATURE == FEATURE_MAX7219_SOFT_SPI_FAST
  spiInterface.begin();
  max7219Module.begin();

#elif FEATURE == FEATURE_MAX7219_HARD_SPI
  spiInterface.begin();
  max7219Module.begin();

#elif FEATURE == FEATURE_MAX7219_HARD_SPI_FAST
  spiInterface.begin();
  max7219Module.begin();

#else
  // No setup() needed for Writers.

#endif
}

void loop() {
#if FEATURE > FEATURE_BASELINE && FEATURE < FEATURE_TM1637_WIRE
  scanningModule.setPatternAt(0, 0x3A);
  scanningModule.renderFieldWhenReady();

#elif FEATURE == FEATURE_TM1637_WIRE
  tm1637Module.setPatternAt(0, 0xff);
  tm1637Module.flush();

#elif FEATURE == FEATURE_TM1637_WIRE_FAST
  tm1637Module.setPatternAt(0, 0xff);
  tm1637Module.flush();

#elif FEATURE == FEATURE_MAX7219_SOFT_SPI
  max7219Module.setPatternAt(0, 0xff);
  max7219Module.flush();

#elif FEATURE == FEATURE_MAX7219_SOFT_SPI_FAST
  max7219Module.setPatternAt(0, 0xff);
  max7219Module.flush();

#elif FEATURE == FEATURE_MAX7219_HARD_SPI
  max7219Module.setPatternAt(0, 0xff);
  max7219Module.flush();

#elif FEATURE == FEATURE_STUB_MODULE
  stubModule.setPatternAt(0, 0xff);

#elif FEATURE == FEATURE_NUMBER_WRITER
  numberWriter.writeUnsignedDecimalAt(0, 42);

#elif FEATURE == FEATURE_CLOCK_WRITER
  clockWriter.writeHourMinute(10, 45);

#elif FEATURE == FEATURE_TEMPERATURE_WRITER
  temperatureWriter.writeTempDegCAt(0, 22 /*temp*/, 4 /*boxSize*/);

#elif FEATURE == FEATURE_CHAR_WRITER
  charWriter.writeCharAt(0, 'a');

#elif FEATURE == FEATURE_STRING_WRITER
  stringWriter.writeStringAt(0, "Hello");

#endif
}
