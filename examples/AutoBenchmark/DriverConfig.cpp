/*
MIT License

Copyright (c) 2018 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Flash.h"
#include "DriverConfig.h"

static const char kLabelSplitDirectDigitDriverOption[] PROGMEM =
    "SplitDirectDigitDriver";
static const char kLabelSplitDirectSegmentDriverOption[] PROGMEM =
    "SplitDirectSegmentDriver";
static const char kLabelSplitSerialDigitDriverOption[] PROGMEM =
    "SplitSerialDigitDriver";
static const char kLabelSplitSpiDigitDriverOption[] PROGMEM =
    "SplitSpiDigitDriver";
static const char kLabelMergedSerialDigitDriverOption[] PROGMEM =
    "MergedSerialDigitDriver";
static const char kLabelMergedSpiDigitDriverOption[] PROGMEM =
    "MergedSpiDigitDriver";
static const char kLabelFastDirectDriverOption[] PROGMEM =
    "FastDirectDriver";
static const char kLabelFastSerialDriverOption[] PROGMEM =
    "FastSerialDriver";
static const char kLabelFastSpiDriverOption[] PROGMEM =
    "FastSpiDriver";

// These must appear in the same order as DriverConfig::DriverOption enum.
const char* DriverConfig::kDriverOptionLabels[] = {
  kLabelSplitDirectDigitDriverOption,
  kLabelSplitDirectSegmentDriverOption,
  kLabelSplitSerialDigitDriverOption,
  kLabelSplitSpiDigitDriverOption,
  kLabelMergedSerialDigitDriverOption,
  kLabelMergedSpiDigitDriverOption,
  kLabelFastDirectDriverOption,
  kLabelFastSerialDriverOption,
  kLabelFastSpiDriverOption,
};

const DriverConfig DriverConfig::kDriverConfigs[] {
  {SplitDirectDigitDriverOption, NoModulation, NoStyles},
  {SplitDirectDigitDriverOption, NoModulation, UseStyles},

  {SplitDirectDigitDriverOption, NoModulation, NoStyles},
  {SplitDirectDigitDriverOption, NoModulation, UseStyles},
  {SplitDirectDigitDriverOption, UseModulation, NoStyles},
  {SplitDirectDigitDriverOption, UseModulation, UseStyles},

  {SplitDirectSegmentDriverOption, NoModulation, NoStyles},
  {SplitDirectSegmentDriverOption, NoModulation, UseStyles},

  {SplitSerialDigitDriverOption, NoModulation, NoStyles},
  {SplitSerialDigitDriverOption, NoModulation, UseStyles},
  {SplitSerialDigitDriverOption, UseModulation, NoStyles},
  {SplitSerialDigitDriverOption, UseModulation, UseStyles},

  {SplitSpiDigitDriverOption, NoModulation, NoStyles},
  {SplitSpiDigitDriverOption, NoModulation, UseStyles},
  {SplitSpiDigitDriverOption, UseModulation, NoStyles},
  {SplitSpiDigitDriverOption, UseModulation, UseStyles},

  {MergedSerialDigitDriverOption, NoModulation, NoStyles},
  {MergedSerialDigitDriverOption, NoModulation, UseStyles},
  {MergedSerialDigitDriverOption, UseModulation, NoStyles},
  {MergedSerialDigitDriverOption, UseModulation, UseStyles},

  {MergedSpiDigitDriverOption, NoModulation, NoStyles},
  {MergedSpiDigitDriverOption, NoModulation, UseStyles},
  {MergedSpiDigitDriverOption, UseModulation, NoStyles},
  {MergedSpiDigitDriverOption, UseModulation, UseStyles},

#ifdef __AVR__

  {FastDirectDriverOption, NoModulation, NoStyles},
  {FastDirectDriverOption, NoModulation, UseStyles},
  {FastDirectDriverOption, UseModulation, NoStyles},
  {FastDirectDriverOption, UseModulation, UseStyles},

  {FastSerialDriverOption, NoModulation, NoStyles},
  {FastSerialDriverOption, NoModulation, UseStyles},
  {FastSerialDriverOption, UseModulation, NoStyles},
  {FastSerialDriverOption, UseModulation, UseStyles},

  {FastSpiDriverOption, NoModulation, NoStyles},
  {FastSpiDriverOption, NoModulation, UseStyles},
  {FastSpiDriverOption, UseModulation, NoStyles},
  {FastSpiDriverOption, UseModulation, UseStyles},

#endif
};

const uint8_t DriverConfig::kNumDriverConfigs =
    sizeof(DriverConfig::kDriverConfigs)
        / sizeof(DriverConfig::kDriverConfigs[0]);

uint8_t DriverConfig::getMaxWidthDriverLabel() {
  const uint8_t numLabels = sizeof(kDriverOptionLabels)
      / sizeof(kDriverOptionLabels[0]);
  uint8_t maxWidth = 0;
  for (uint8_t i = 0; i < numLabels; i++) {
    uint8_t width = strlen_P(kDriverOptionLabels[i]);
    if (width > maxWidth) {
      maxWidth = width;
    }
  }
  return maxWidth;
}
