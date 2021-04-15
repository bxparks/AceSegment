/*
 * A program which compiles in different ScanningDisplay objects configured with
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
#define FEATURE_DIRECT 1
#define FEATURE_SINGLE_SW_SPI 2
#define FEATURE_SINGLE_HW_SPI 3
#define FEATURE_DUAL_SW_SPI 4
#define FEATURE_DUAL_HW_SPI 5
#define FEATURE_DIRECT_FAST 6
#define FEATURE_SINGLE_SW_SPI_FAST 7
#define FEATURE_DUAL_SW_SPI_FAST 8
#define FEATURE_TM1637_DISPLAY 9
#define FEATURE_TM1637_DISPLAY_FAST 10
#define FEATURE_STUB_DISPLAY 11
#define FEATURE_NUMBER_WRITER 12
#define FEATURE_CLOCK_WRITER 13
#define FEATURE_CHAR_WRITER 14
#define FEATURE_STRING_WRITER 15

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE > FEATURE_BASELINE
  #include <AceSegment.h>
  #include <ace_segment/fast/LedMatrixDirectFast.h>
  #include <ace_segment/fast/SwSpiAdapterFast.h>
  #include <ace_segment/tm1637/Tm1637DriverFast.h>
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

  class StubDisplay : public LedDisplay {
    public:
      StubDisplay(): LedDisplay(NUM_DIGITS) {}

      void writePatternAt(uint8_t /*pos*/, uint8_t pattern) override {
        disableCompilerOptimization = pattern;
      }

      void writePatternsAt(uint8_t /*pos*/, const uint8_t patterns[],
          uint8_t /*len*/) override {
        disableCompilerOptimization = patterns[0];
      }

      void writePatternsAt_P(uint8_t /*pos*/, const uint8_t patterns[],
          uint8_t /*len*/) override {
        disableCompilerOptimization = pgm_read_byte(patterns);
      }

      void writeDecimalPointAt(uint8_t /*pos*/, bool state = true)
          override {
        disableCompilerOptimization = state;
      }

      void setBrightness(uint8_t brightness) override {
        disableCompilerOptimization = brightness;
      }

      void clear() override {
        disableCompilerOptimization = 0;
      }
  };

  Hardware hardware;

  #if FEATURE == FEATURE_DIRECT
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
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_SINGLE_SW_SPI
    // Common Cathode, with transistors on Group pins
    SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixSingleShiftRegister<Hardware, SwSpiAdapter>;
    LedMatrix ledMatrix(
        hardware,
        spiAdapter,
        LedMatrix::kActiveHighPattern /*groupOnPattern*/,
        LedMatrix::kActiveHighPattern /*elementOnPattern*/,
        NUM_DIGITS,
        DIGIT_PINS);
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_SINGLE_SW_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

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
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_SINGLE_HW_SPI
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
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_DUAL_SW_SPI
    // Common Cathode, with transistors on Group pins
    SwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualShiftRegister<SwSpiAdapter>;
    LedMatrix ledMatrix(
        spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_DUAL_SW_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiAdapter = SwSpiAdapterFast<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiAdapter spiAdapter;
    using LedMatrix = LedMatrixDualShiftRegister<SpiAdapter>;
    LedMatrix ledMatrix(
        spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_DUAL_HW_SPI
    // Common Cathode, with transistors on Group pins
    HwSpiAdapter spiAdapter(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    using LedMatrix = LedMatrixDualShiftRegister<HwSpiAdapter>;
    LedMatrix ledMatrix(
        spiAdapter,
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_DIRECT_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Anode, with transitions on Group pins
    using LedMatrix = LedMatrixDirectFast<
      4, 5, 6, 7,
      8, 9, 10, 16, 14, 18, 19, 15
    >;
    LedMatrix ledMatrix(
        LedMatrix::kActiveLowPattern /*groupOnPattern*/,
        LedMatrix::kActiveLowPattern /*elementOnPattern*/);
    ScanningDisplay<Hardware, LedMatrix, NUM_DIGITS, NUM_SUBFIELDS>
        scanningDisplay(hardware, ledMatrix, FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_TM1637_DISPLAY
    using Driver = Tm1637Driver;
    Driver driver(CLK_PIN, DIO_PIN, BIT_DELAY);
    Tm1637Display<Driver, NUM_DIGITS> ledDisplay(driver);

  #elif FEATURE == FEATURE_TM1637_DISPLAY_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using Driver = Tm1637DriverFast<CLK_PIN, DIO_PIN, BIT_DELAY>;
    Driver driver;
    Tm1637Display<Driver, NUM_DIGITS> ledDisplay(driver);

  #elif FEATURE == FEATURE_STUB_DISPLAY
    StubDisplay ledDisplay;

  #elif FEATURE == FEATURE_NUMBER_WRITER
    StubDisplay ledDisplay;
    NumberWriter numberWriter(ledDisplay);

  #elif FEATURE == FEATURE_CLOCK_WRITER
    StubDisplay ledDisplay;
    ClockWriter clockWriter(ledDisplay);

  #elif FEATURE == FEATURE_CHAR_WRITER
    StubDisplay ledDisplay;
    CharWriter charWriter(ledDisplay);

  #elif FEATURE == FEATURE_STRING_WRITER
    StubDisplay ledDisplay;
    CharWriter charWriter(ledDisplay);
    StringWriter stringWriter(charWriter);

  #endif
#endif

void setup() {
#if FEATURE == FEATURE_BASELINE
  disableCompilerOptimization = 3;

// In the following, I used to grab the output of patterns[] and write to
// disableCompilerOptimization to prevent the compiler from optimizing away the
// entire program. But after templatizing ScanningDisplay, pattterns variable is
// no longer accessible. But it does not matter because I realized that
// ScanningDisplay performs a digitalWrite(), which has the same effect of
// disabling optimizations.

#elif FEATURE == FEATURE_DIRECT
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_DIRECT_FAST
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_SINGLE_SW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_SINGLE_SW_SPI_FAST
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_SINGLE_HW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_DUAL_SW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_DUAL_SW_SPI_FAST
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_DUAL_HW_SPI
  spiAdapter.begin();
  ledMatrix.begin();
  scanningDisplay.begin();

#elif FEATURE == FEATURE_TM1637_DISPLAY
  driver.begin();
  ledDisplay.begin();

#elif FEATURE == FEATURE_TM1637_DISPLAY_FAST
  driver.begin();
  ledDisplay.begin();

#else
  // No setup() needed for Writers.

#endif
}

void loop() {
#if FEATURE > FEATURE_BASELINE && FEATURE < FEATURE_TM1637_DISPLAY
  scanningDisplay.writePatternAt(0, 0x3A);
  scanningDisplay.renderFieldWhenReady();

#elif FEATURE == FEATURE_TM1637_DISPLAY
  ledDisplay.writePatternAt(0, 0xff);
  ledDisplay.flush();

#elif FEATURE == FEATURE_TM1637_DISPLAY_FAST
  ledDisplay.writePatternAt(0, 0xff);
  ledDisplay.flush();

#elif FEATURE == FEATURE_STUB_DISPLAY
  ledDisplay.writePatternAt(0, 0xff);

#elif FEATURE == FEATURE_NUMBER_WRITER
  numberWriter.writeUnsignedDecimalAt(0, 42);

#elif FEATURE == FEATURE_CLOCK_WRITER
  clockWriter.writeHourMinute(10, 45);

#elif FEATURE == FEATURE_CHAR_WRITER
  charWriter.writeCharAt(0, 'a');

#elif FEATURE == FEATURE_STRING_WRITER
  stringWriter.writeStringAt(0, "Hello");

#endif
}
