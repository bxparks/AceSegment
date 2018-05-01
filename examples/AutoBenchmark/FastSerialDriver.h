// This file was generated by the following script:
//   ../../tools/fast_driver.py --digit_pins 4 5 6 7 --segment_serial_pins 10 11 13 --class_name FastSerialDriver --output_files
//
// DO NOT EDIT

#ifdef __AVR__

#include <stdint.h>
#include <digitalWriteFast.h>
#include <ace_segment/ModulatingDigitDriver.h>
#include <ace_segment/Util.h>

#ifndef ACE_SEGMENT_FastSerialDriver_H
#define ACE_SEGMENT_FastSerialDriver_H

class FastSerialDriver: public ace_segment::ModulatingDigitDriver {
  public:
    // Constructor
    FastSerialDriver(ace_segment::DimmablePattern* dimmablePatterns,
            uint8_t numDigits, uint8_t numSubFields):
        ace_segment::ModulatingDigitDriver(
            nullptr /* ledMatrix */, dimmablePatterns, numDigits, numSubFields)
    {}

    // Destructor
    virtual ~FastSerialDriver() override {}

    virtual void configure() override;
    virtual void finish() override;
    virtual void displayCurrentField() override;
    virtual void prepareToSleep() override;

  private:
    typedef void (*DigitalWriter)(void);

    static const uint8_t kLatchPin = 10;
    static const uint8_t kDataPin = 11;
    static const uint8_t kClockPin = 13;

    // define pin values depending on common cathode or anode wiring
    static const uint8_t kDigitOn = LOW;
    static const uint8_t kDigitOff = HIGH;
    static const uint8_t kSegmentOn = HIGH;
    static const uint8_t kSegmentOff = LOW;

    static const uint8_t kDigitPins[];
    static const DigitalWriter kDigitWriters[];

    static void disableDigit(uint8_t digit) {
      uint8_t index = digit * 2 + kDigitOff;
      DigitalWriter writer = kDigitWriters[index];
      writer();
    }

    static void enableDigit(uint8_t digit) {
      uint8_t index = digit * 2 + kDigitOn;
      DigitalWriter writer = kDigitWriters[index];
      writer();
    }

    static void drawSegments(uint8_t pattern) {
      digitalWriteFast(kLatchPin, LOW);
      uint8_t actualPattern = (kSegmentOn == HIGH) ? pattern : ~pattern;
      shiftOutFast(actualPattern);
      digitalWriteFast(kLatchPin, HIGH);
    }

    static void shiftOutFast(uint8_t pattern);

    // DigitalWriter functions for writing digit pins.
    static void digitalWriteFastDigit00Low() { digitalWriteFast(4, LOW); }
    static void digitalWriteFastDigit00High() { digitalWriteFast(4, HIGH); }
    static void digitalWriteFastDigit01Low() { digitalWriteFast(5, LOW); }
    static void digitalWriteFastDigit01High() { digitalWriteFast(5, HIGH); }
    static void digitalWriteFastDigit02Low() { digitalWriteFast(6, LOW); }
    static void digitalWriteFastDigit02High() { digitalWriteFast(6, HIGH); }
    static void digitalWriteFastDigit03Low() { digitalWriteFast(7, LOW); }
    static void digitalWriteFastDigit03High() { digitalWriteFast(7, HIGH); }
};

#endif

#endif
