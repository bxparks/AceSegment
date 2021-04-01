// Fragments of code related to SplitSegmentDriver.h.

// From Drivers.h

/**
 * A driver that uses a GPIO pin to drive each digit and segment pins of
 * the LED display. The resistors are assumed to be on the digits and can be
 * driven at the same time, so the segments are multiplexed.
 */
class SplitDirectSegmentDriver:
    private LedMatrixSplitDirect,
    public SplitSegmentDriver {

  public:
    SplitDirectSegmentDriver(Hardware* hardware,
            DimmablePattern* dimmablePatterns, bool commonCathode,
            bool transistorsOnDigits, bool transistorsOnSegments,
            uint8_t numDigits, uint8_t numSegments,
            const uint8_t* digitPins, const uint8_t* segmentPins):
        LedMatrixSplitDirect(hardware, !commonCathode, transistorsOnSegments,
            transistorsOnDigits, numSegments, numDigits, segmentPins,
            digitPins),
        SplitSegmentDriver(this, dimmablePatterns, numDigits)
    {}
};


// From DriverTest/DriverTest.ino

// ----------------------------------------------------------------------
// Tests for SplitSegmentDriver w/ LedMatrixDirect.
// ----------------------------------------------------------------------

class SplitDirectSegmentDriverTest: public BaseHardwareTest {
  protected:
    void setup() override {
      BaseHardwareTest::setup();
      mDriver = new SplitDirectSegmentDriver(mHardware,
          dimmablePatterns,
          true /* commonCathode */,
          false /* transistorsOnDigits */,
          false /* transistorsOnSegments */,
          NUM_DIGITS,
          NUM_SEGMENTS,
          digitPins,
          segmentPins);
      mDriver->configure();
      mHardware->clear();
    }

    void teardown() override {
      delete mDriver;
      BaseHardwareTest::teardown();
    }

    Driver* mDriver;
};

testF(SplitDirectSegmentDriverTest, configure) {
  mDriver->configure();
  assertEvents(24,
      Event::kTypePinMode, 4, OUTPUT,
      Event::kTypeDigitalWrite, 4, LOW,
      Event::kTypePinMode, 5, OUTPUT,
      Event::kTypeDigitalWrite, 5, LOW,
      Event::kTypePinMode, 6, OUTPUT,
      Event::kTypeDigitalWrite, 6, LOW,
      Event::kTypePinMode, 7, OUTPUT,
      Event::kTypeDigitalWrite, 7, LOW,
      Event::kTypePinMode, 8, OUTPUT,
      Event::kTypeDigitalWrite, 8, LOW,
      Event::kTypePinMode, 9, OUTPUT,
      Event::kTypeDigitalWrite, 9, LOW,
      Event::kTypePinMode, 10, OUTPUT,
      Event::kTypeDigitalWrite, 10, LOW,
      Event::kTypePinMode, 11, OUTPUT,
      Event::kTypeDigitalWrite, 11, LOW,
      Event::kTypePinMode, 0, OUTPUT,
      Event::kTypeDigitalWrite, 0, HIGH,
      Event::kTypePinMode, 1, OUTPUT,
      Event::kTypeDigitalWrite, 1, HIGH,
      Event::kTypePinMode, 2, OUTPUT,
      Event::kTypeDigitalWrite, 2, HIGH,
      Event::kTypePinMode, 3, OUTPUT,
      Event::kTypeDigitalWrite, 3, HIGH);
  assertEqual((uint16_t)(8), mDriver->getFieldsPerFrame());

  mHardware->clear();
  mDriver->finish();
  assertEvents(12,
      Event::kTypePinMode, 4, INPUT,
      Event::kTypePinMode, 5, INPUT,
      Event::kTypePinMode, 6, INPUT,
      Event::kTypePinMode, 7, INPUT,
      Event::kTypePinMode, 8, INPUT,
      Event::kTypePinMode, 9, INPUT,
      Event::kTypePinMode, 10, INPUT,
      Event::kTypePinMode, 11, INPUT,
      Event::kTypePinMode, 0, INPUT,
      Event::kTypePinMode, 1, INPUT,
      Event::kTypePinMode, 2, INPUT,
      Event::kTypePinMode, 3, INPUT);
}

testF(SplitDirectSegmentDriverTest, displayCurrentField_one_dark) {
  // The following segment patterns were crafted so that digitPattern(7) bits
  // have the same digitPattern as digitPattern(0), which should cause the digit
  // bits to be reused between the two iterations.
  mDriver->setPattern(0, 0x91, 255);  // 1001 0001
  mDriver->setPattern(1, 0x22, 128);  // 0010 0010
  mDriver->setPattern(2, 0xD5, 64);   // 1101 0101
  mDriver->setPattern(3, 0x91, 0);    // 0000 0000 <- 1001 0001 (brightness = 0)

  // field 0 (segment 0)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON);

  // field 1 (segment 1)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON);

  // field 2 (segment 2)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 5, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 6, SEGMENT_ON);

  // field 3 (segment 3)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 6, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 7, SEGMENT_ON);

  // field 4 (segment 4)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 7, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 8, SEGMENT_ON);

  // field 5 (segment 5)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 8, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 9, SEGMENT_ON);

  // field 6 (segment 6)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 9, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 10, SEGMENT_ON);

  // field 7 (segment 7)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 10, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 11, SEGMENT_ON);

  // field 0 (segment 0), reuse the digit pattern from the prev iteration
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(2,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON);
}

testF(SplitDirectSegmentDriverTest, prepareToSleep) {
  mDriver->setPattern(0, 0x91, 255);  // 1001 0001
  mDriver->setPattern(1, 0x22, 128);  // 0010 0010
  mDriver->setPattern(2, 0xD5, 64);   // 1101 0101
  mDriver->setPattern(3, 0x91, 0);    // 0000 0000 <- 1001 0001 (brightness = 0)

  // display field 0
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 11, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_ON,
      Event::kTypeDigitalWrite, 1, DIGIT_OFF,
      Event::kTypeDigitalWrite, 2, DIGIT_ON,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 4, SEGMENT_ON);

  mHardware->clear();
  mDriver->prepareToSleep();
  assertEvents(1,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF);

  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(0);

  mHardware->clear();
  mDriver->wakeFromSleep();
  assertEvents(0);

  // field 1 (segment 1)
  mHardware->clear();
  mDriver->displayCurrentField();
  assertEvents(6,
      Event::kTypeDigitalWrite, 4, SEGMENT_OFF,
      Event::kTypeDigitalWrite, 0, DIGIT_OFF,
      Event::kTypeDigitalWrite, 1, DIGIT_ON,
      Event::kTypeDigitalWrite, 2, DIGIT_OFF,
      Event::kTypeDigitalWrite, 3, DIGIT_OFF,
      Event::kTypeDigitalWrite, 5, SEGMENT_ON);
}


