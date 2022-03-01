/*
 * A program which compiles various LedModule objects with different
 * LED configurations to determine the flash and static memory sizes from the
 * output of the compiler. See the generated README.md for details.
 */

#include <Arduino.h>

// DO NOT MODIFY THIS LINE. This will be overwritten by collect.sh on each
// iteration, incrementing from 0 to N. The Arduino IDE will compile the
// program, then the script will extract the flash and static memory usage
// numbers printed out by the Arduino compiler. The numbers will be printed on
// the STDOUT, which then can be saved to a file specific for a particular
// hardware platform, e.g. "nano.txt" or "esp8266.txt".
#define FEATURE 0

// List of features of AceSegment that we want to gather memory usage numbers.
#define FEATURE_BASELINE 0
#define FEATURE_DIRECT_MODULE 1
#define FEATURE_DIRECT_FAST4_MODULE 2
#define FEATURE_HYBRID_HARD_SPI 3
#define FEATURE_HYBRID_HARD_SPI_FAST 4
#define FEATURE_HYBRID_SIMPLE_SPI 5
#define FEATURE_HYBRID_SIMPLE_SPI_FAST 6
#define FEATURE_HC595_HARD_SPI 7
#define FEATURE_HC595_HARD_SPI_FAST 8
#define FEATURE_HC595_SIMPLE_SPI 9
#define FEATURE_HC595_SIMPLE_SPI_FAST 10
#define FEATURE_TM1637_TMI 11
#define FEATURE_TM1637_TMI_FAST 12
#define FEATURE_TM1638_TMI_1638 13
#define FEATURE_TM1638_TMI_1638_FAST 14
#define FEATURE_TM1638_ANODE_TMI_1638 15
#define FEATURE_TM1638_ANODE_TMI_1638_FAST 16
#define FEATURE_MAX7219_HARD_SPI 17
#define FEATURE_MAX7219_HARD_SPI_FAST 18
#define FEATURE_MAX7219_SIMPLE_SPI 19
#define FEATURE_MAX7219_SIMPLE_SPI_FAST 20
#define FEATURE_HT16K33_TWO_WIRE 21
#define FEATURE_HT16K33_SIMPLE_WIRE 22
#define FEATURE_HT16K33_SIMPLE_WIRE_FAST 23

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE > FEATURE_BASELINE
  #include <AceSegment.h>
  #include <AceSPI.h>
  #include <AceTMI.h>
  #include <AceWire.h>
  #if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
    #include <digitalWriteFast.h>
    #include <ace_spi/SimpleSpiFastInterface.h>
    #include <ace_spi/HardSpiFastInterface.h>
    #include <ace_tmi/SimpleTmi1637FastInterface.h>
    #include <ace_tmi/SimpleTmi1638FastInterface.h>
    #include <ace_segment/direct/DirectFast4Module.h>
  #endif
  using namespace ace_segment;
  using namespace ace_spi;
  using namespace ace_tmi;
  using namespace ace_wire;

  // Common to all FEATURES
  const uint8_t NUM_DIGITS = 4;
  const uint8_t NUM_SEGMENTS = 8;
  const uint8_t FRAMES_PER_SECOND = 60;
  const uint8_t NUM_SUBFIELDS = 1;

  // Direct
  const uint8_t DIGIT_PINS[NUM_DIGITS] = {4, 5, 6, 7};
  const uint8_t SEGMENT_PINS[NUM_SEGMENTS] = {8, 9, 10, 16, 14, 18, 19, 15};

  // 74HC595, MAX7219
  const uint8_t LATCH_PIN = 10; // ST_CP on 74HC595
  const uint8_t DATA_PIN = MOSI; // DS on 74HC595
  const uint8_t CLOCK_PIN = SCK; // SH_CP on 74HC595

  // TM1637, TM1638
  const uint8_t CLK_PIN = 16;
  const uint8_t DIO_PIN = 10;
  const uint8_t STB_PIN = 11;
  const uint8_t BIT_DELAY = 100;

  // HT16K33
  const uint8_t SDA_PIN = 2;
  const uint8_t SCL_PIN = 3;
  const uint8_t DELAY_MICROS = 4;
  const uint8_t HT16K33_I2C_ADDRESS = 0x70;

  #if FEATURE == FEATURE_DIRECT_MODULE
    // Common Anode, with transitions on Group pins
    DirectModule<NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
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
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND);

  #elif FEATURE == FEATURE_HYBRID_HARD_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);
    HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveHighPattern /*segmentOnPattern*/,
        kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_HYBRID_HARD_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(SPI);
    HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveHighPattern /*segmentOnPattern*/,
        kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_HYBRID_SIMPLE_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveHighPattern /*segmentOnPattern*/,
        kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_HYBRID_SIMPLE_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    HybridModule<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveHighPattern /*segmentOnPattern*/,
        kActiveHighPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        DIGIT_PINS
    );

  #elif FEATURE == FEATURE_HC595_HARD_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);
    Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        kByteOrderDigitHighSegmentLow
    );

  #elif FEATURE == FEATURE_HC595_HARD_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(SPI);
    Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        kByteOrderDigitHighSegmentLow
    );

  #elif FEATURE == FEATURE_HC595_SIMPLE_SPI
    // Common Cathode, with transistors on Group pins
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        kByteOrderDigitHighSegmentLow
    );

  #elif FEATURE == FEATURE_HC595_SIMPLE_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    // Common Cathode, with transistors on Group pins
    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    Hc595Module<SpiInterface, NUM_DIGITS, NUM_SUBFIELDS> scanningModule(
        spiInterface,
        kActiveLowPattern /*segmentOnPattern*/,
        kActiveLowPattern /*digitOnPattern*/,
        FRAMES_PER_SECOND,
        kByteOrderDigitHighSegmentLow
    );

  #elif FEATURE == FEATURE_TM1637_TMI
    using TmiInterface = SimpleTmi1637Interface;
    TmiInterface tmiInterface(DIO_PIN, CLK_PIN, BIT_DELAY);
    Tm1637Module<TmiInterface, NUM_DIGITS> tm1637Module(tmiInterface);

  #elif FEATURE == FEATURE_TM1637_TMI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using TmiInterface = SimpleTmi1637FastInterface<
        DIO_PIN, CLK_PIN, BIT_DELAY>;
    TmiInterface tmiInterface;
    Tm1637Module<TmiInterface, NUM_DIGITS> tm1637Module(tmiInterface);

  #elif FEATURE == FEATURE_TM1638_TMI_1638
    using TmiInterface = SimpleTmi1638Interface;
    TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY);
    Tm1638Module<TmiInterface, NUM_DIGITS> tm1638Module(tmiInterface);

  #elif FEATURE == FEATURE_TM1638_TMI_1638_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using TmiInterface = SimpleTmi1638FastInterface<
        DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY>;
    TmiInterface tmiInterface;
    Tm1638Module<TmiInterface, NUM_DIGITS> tm1638Module(tmiInterface);

  #elif FEATURE == FEATURE_TM1638_ANODE_TMI_1638
    using TmiInterface = SimpleTmi1638Interface;
    TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY);
    Tm1638AnodeModule<TmiInterface, NUM_DIGITS> tm1638Module(tmiInterface);

  #elif FEATURE == FEATURE_TM1638_ANODE_TMI_1638_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using TmiInterface = SimpleTmi1638FastInterface<
        DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY>;
    TmiInterface tmiInterface;
    Tm1638AnodeModule<TmiInterface, NUM_DIGITS> tm1638Module(tmiInterface);

  #elif FEATURE == FEATURE_MAX7219_HARD_SPI
    using SpiInterface = HardSpiInterface<SPIClass>;
    SpiInterface spiInterface(SPI, LATCH_PIN);
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_MAX7219_HARD_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using SpiInterface = HardSpiFastInterface<SPIClass, LATCH_PIN>;
    SpiInterface spiInterface(SPI);
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_MAX7219_SIMPLE_SPI
    using SpiInterface = SimpleSpiInterface;
    SpiInterface spiInterface(LATCH_PIN, DATA_PIN, CLOCK_PIN);
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_MAX7219_SIMPLE_SPI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using SpiInterface = SimpleSpiFastInterface<LATCH_PIN, DATA_PIN, CLOCK_PIN>;
    SpiInterface spiInterface;
    Max7219Module<SpiInterface, NUM_DIGITS> max7219Module(
        spiInterface, kDigitRemapArray8Max7219);

  #elif FEATURE == FEATURE_HT16K33_TWO_WIRE
    #include <Wire.h>
    using WireInterface = TwoWireInterface<TwoWire>;
    WireInterface wireInterface(Wire);
    Ht16k33Module<WireInterface, NUM_DIGITS> ht16k33Module(
        wireInterface, HT16K33_I2C_ADDRESS);

  #elif FEATURE == FEATURE_HT16K33_SIMPLE_WIRE
    using WireInterface = SimpleWireInterface;
    WireInterface wireInterface(SDA_PIN, SCL_PIN, DELAY_MICROS);
    Ht16k33Module<WireInterface, NUM_DIGITS> ht16k33Module(
        wireInterface, HT16K33_I2C_ADDRESS);

  #elif FEATURE == FEATURE_HT16K33_SIMPLE_WIRE_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    #include <digitalWriteFast.h>
    #include <ace_wire/SimpleWireFastInterface.h>
    using WireInterface = SimpleWireFastInterface<
        SDA_PIN, SCL_PIN, DELAY_MICROS>;
    WireInterface wireInterface;
    Ht16k33Module<WireInterface, NUM_DIGITS> ht16k33Module(
        wireInterface, HT16K33_I2C_ADDRESS);

  #else
    #error Unknown FEATURE

  #endif
#endif

// TeensyDuino seems to pull in malloc() and free() when a class with virtual
// functions is used polymorphically. This causes the memory consumption of
// FEATURE_BASELINE (which normally has no classes defined, so does not include
// malloc() and free()) to be artificially small which throws off the memory
// consumption calculations for all subsequent features. Let's define a
// throw-away class and call its method for all FEATURES, including BASELINE.
#if defined(TEENSYDUINO)
  class FooClass {
    public:
      virtual void doit() {
        disableCompilerOptimization = 0;
      }
  };

  FooClass* foo;
#endif

void setup() {
#if defined(TEENSYDUINO)
  foo = new FooClass();
#endif

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

#elif FEATURE == FEATURE_HYBRID_SIMPLE_SPI
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_HYBRID_SIMPLE_SPI_FAST
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_HYBRID_HARD_SPI
  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_HYBRID_HARD_SPI_FAST
  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_HC595_SIMPLE_SPI
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_HC595_SIMPLE_SPI_FAST
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_HC595_HARD_SPI
  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_HC595_HARD_SPI_FAST
  SPI.begin();
  spiInterface.begin();
  scanningModule.begin();

#elif FEATURE == FEATURE_TM1637_TMI
  tmiInterface.begin();
  tm1637Module.begin();

#elif FEATURE == FEATURE_TM1637_TMI_FAST
  tmiInterface.begin();
  tm1637Module.begin();

#elif FEATURE == FEATURE_TM1638_TMI_1638
  tmiInterface.begin();
  tm1638Module.begin();

#elif FEATURE == FEATURE_TM1638_TMI_1638_FAST
  tmiInterface.begin();
  tm1638Module.begin();

#elif FEATURE == FEATURE_TM1638_ANODE_TMI_1638
  tmiInterface.begin();
  tm1638Module.begin();

#elif FEATURE == FEATURE_TM1638_ANODE_TMI_1638_FAST
  tmiInterface.begin();
  tm1638Module.begin();

#elif FEATURE == FEATURE_MAX7219_SIMPLE_SPI
  spiInterface.begin();
  max7219Module.begin();

#elif FEATURE == FEATURE_MAX7219_SIMPLE_SPI_FAST
  spiInterface.begin();
  max7219Module.begin();

#elif FEATURE == FEATURE_MAX7219_HARD_SPI
  SPI.begin();
  spiInterface.begin();
  max7219Module.begin();

#elif FEATURE == FEATURE_MAX7219_HARD_SPI_FAST
  SPI.begin();
  spiInterface.begin();
  max7219Module.begin();

#elif FEATURE == FEATURE_HT16K33_TWO_WIRE
  Wire.begin();
  wireInterface.begin();
  ht16k33Module.begin();

#elif FEATURE == FEATURE_HT16K33_SIMPLE_WIRE
  wireInterface.begin();
  ht16k33Module.begin();

#elif FEATURE == FEATURE_HT16K33_SIMPLE_WIRE_FAST
  wireInterface.begin();
  ht16k33Module.begin();

#else
  #error Unknown FEATURE

#endif
}

void loop() {
#if defined(TEENSYDUINO)
  foo->doit();
#endif

#if FEATURE == FEATURE_BASELINE
  // do nothing

#elif FEATURE > FEATURE_BASELINE && FEATURE < FEATURE_TM1637_TMI
  scanningModule.setPatternAt(0, 0x3A);
  scanningModule.renderFieldWhenReady();

#elif FEATURE == FEATURE_TM1637_TMI
  tm1637Module.setPatternAt(0, 0xff);
  tm1637Module.flush();

#elif FEATURE == FEATURE_TM1637_TMI_FAST
  tm1637Module.setPatternAt(0, 0xff);
  tm1637Module.flush();

#elif FEATURE == FEATURE_TM1638_TMI_1638
  tm1638Module.setPatternAt(0, 0xff);
  tm1638Module.flush();

#elif FEATURE == FEATURE_TM1638_TMI_1638_FAST
  tm1638Module.setPatternAt(0, 0xff);
  tm1638Module.flush();

#elif FEATURE == FEATURE_TM1638_ANODE_TMI_1638
  tm1638Module.setPatternAt(0, 0xff);
  tm1638Module.flush();

#elif FEATURE == FEATURE_TM1638_ANODE_TMI_1638_FAST
  tm1638Module.setPatternAt(0, 0xff);
  tm1638Module.flush();

#elif FEATURE == FEATURE_MAX7219_SIMPLE_SPI
  max7219Module.setPatternAt(0, 0xff);
  max7219Module.flush();

#elif FEATURE == FEATURE_MAX7219_SIMPLE_SPI_FAST
  max7219Module.setPatternAt(0, 0xff);
  max7219Module.flush();

#elif FEATURE == FEATURE_MAX7219_HARD_SPI
  max7219Module.setPatternAt(0, 0xff);
  max7219Module.flush();

#elif FEATURE == FEATURE_MAX7219_HARD_SPI_FAST
  max7219Module.setPatternAt(0, 0xff);
  max7219Module.flush();

#elif FEATURE == FEATURE_HT16K33_TWO_WIRE
  ht16k33Module.setPatternAt(0, 0xff);
  ht16k33Module.flush();

#elif FEATURE == FEATURE_HT16K33_SIMPLE_WIRE
  ht16k33Module.setPatternAt(0, 0xff);
  ht16k33Module.flush();

#elif FEATURE == FEATURE_HT16K33_SIMPLE_WIRE_FAST
  ht16k33Module.setPatternAt(0, 0xff);
  ht16k33Module.flush();

#elif FEATURE == FEATURE_STUB_MODULE
  stubModule.setPatternAt(0, 0xff);

#else
  #error Unknown FEATURE

#endif
}
