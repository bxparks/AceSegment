#include "FastHardware.h"

namespace ace_segment {

const FastHardware::FastHardwareFn FastHardware::kDigitalWriteFns[] = {
  &FastHardware::digitalWriteFastLow00,
  &FastHardware::digitalWriteFastHigh00,
  &FastHardware::digitalWriteFastLow01,
  &FastHardware::digitalWriteFastHigh01,
  &FastHardware::digitalWriteFastLow02,
  &FastHardware::digitalWriteFastHigh02,
  &FastHardware::digitalWriteFastLow03,
  &FastHardware::digitalWriteFastHigh03,
  &FastHardware::digitalWriteFastLow04,
  &FastHardware::digitalWriteFastHigh04,
  &FastHardware::digitalWriteFastLow05,
  &FastHardware::digitalWriteFastHigh05,
  &FastHardware::digitalWriteFastLow06,
  &FastHardware::digitalWriteFastHigh06,
  &FastHardware::digitalWriteFastLow07,
  &FastHardware::digitalWriteFastHigh07,
  &FastHardware::digitalWriteFastLow08,
  &FastHardware::digitalWriteFastHigh08,
  &FastHardware::digitalWriteFastLow09,
  &FastHardware::digitalWriteFastHigh09,
  &FastHardware::digitalWriteFastLow10,
  &FastHardware::digitalWriteFastHigh10,
  &FastHardware::digitalWriteFastLow11,
  &FastHardware::digitalWriteFastHigh11,
  &FastHardware::digitalWriteFastLow12,
  &FastHardware::digitalWriteFastHigh12,
  &FastHardware::digitalWriteFastLow13,
  &FastHardware::digitalWriteFastHigh13,
  &FastHardware::digitalWriteFastLow14,
  &FastHardware::digitalWriteFastHigh14,
  &FastHardware::digitalWriteFastLow15,
  &FastHardware::digitalWriteFastHigh15,
  &FastHardware::digitalWriteFastLow16,
  &FastHardware::digitalWriteFastHigh16,
  &FastHardware::digitalWriteFastLow17,
  &FastHardware::digitalWriteFastHigh17,
  &FastHardware::digitalWriteFastLow18,
  &FastHardware::digitalWriteFastHigh18,
  &FastHardware::digitalWriteFastLow19,
  &FastHardware::digitalWriteFastHigh19,
};

const size_t FastHardware::kNumFns =
    sizeof(kDigitalWriteFns)/sizeof(kDigitalWriteFns[0]);

}
