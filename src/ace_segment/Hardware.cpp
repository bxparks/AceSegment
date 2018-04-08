#include "Hardware.h"

namespace ace_segment {

const Hardware::DigitalWriter Hardware::kDigitalWriters[] = {
  &Hardware::digitalWriteLow00,
  &Hardware::digitalWriteHigh00,
  &Hardware::digitalWriteLow01,
  &Hardware::digitalWriteHigh01,
  &Hardware::digitalWriteLow02,
  &Hardware::digitalWriteHigh02,
  &Hardware::digitalWriteLow03,
  &Hardware::digitalWriteHigh03,
  &Hardware::digitalWriteLow04,
  &Hardware::digitalWriteHigh04,
  &Hardware::digitalWriteLow05,
  &Hardware::digitalWriteHigh05,
  &Hardware::digitalWriteLow06,
  &Hardware::digitalWriteHigh06,
  &Hardware::digitalWriteLow07,
  &Hardware::digitalWriteHigh07,
  &Hardware::digitalWriteLow08,
  &Hardware::digitalWriteHigh08,
  &Hardware::digitalWriteLow09,
  &Hardware::digitalWriteHigh09,
  &Hardware::digitalWriteLow10,
  &Hardware::digitalWriteHigh10,
  &Hardware::digitalWriteLow11,
  &Hardware::digitalWriteHigh11,
  &Hardware::digitalWriteLow12,
  &Hardware::digitalWriteHigh12,
  &Hardware::digitalWriteLow13,
  &Hardware::digitalWriteHigh13,
  &Hardware::digitalWriteLow14,
  &Hardware::digitalWriteHigh14,
  &Hardware::digitalWriteLow15,
  &Hardware::digitalWriteHigh15,
  &Hardware::digitalWriteLow16,
  &Hardware::digitalWriteHigh16,
  &Hardware::digitalWriteLow17,
  &Hardware::digitalWriteHigh17,
  &Hardware::digitalWriteLow18,
  &Hardware::digitalWriteHigh18,
  &Hardware::digitalWriteLow19,
  &Hardware::digitalWriteHigh19,
};

const size_t Hardware::kNumWriters =
    sizeof(kDigitalWriters)/sizeof(kDigitalWriters[0]);

}
