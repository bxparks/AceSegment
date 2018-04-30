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

#ifndef BENCHMARK_BUNDLE_H
#define BENCHMARK_BUNDLE_H

#include <stdint.h>
#include <Arduino.h> // SS, MOSI, SCK
#include <AceSegment.h>
#include "DriverConfig.h"
using namespace ace_segment;

struct BenchmarkBundle {
  static const uint8_t kFramePerSecond = 60;
  static const uint8_t kNumSubFields = 16;
  static const uint8_t kNumDigits = 4;
  static const uint8_t kDigitPins[kNumDigits];
  static const uint8_t kSegmentDirectPins[8];

  static const uint8_t kLatchPin = SS; // ST_CP on 74HC595
  static const uint8_t kDataPin = MOSI; // DS on 74HC595
  static const uint8_t kClockPin = SCK; // SH_CP on 74HC595

  static const uint16_t kBlinkDuration = 800;
  static const uint8_t kBlinkStyle = 1;
  static const uint16_t kPulseDuration = 1600;
  static const uint8_t kPulseStyle = 2;

  /** Constructor. */
  BenchmarkBundle(const DriverConfig* driverConfig);

  /** Destructor. */
  ~BenchmarkBundle() {
    delete mCharWriter;
    delete mRenderer;
    delete mPulseStyler;
    delete mBlinkStyler;
    delete mDriver;
    delete mHardware;
  }

  void configure() {
    mDriver->configure();
    mRenderer->configure();
  }

  Hardware* mHardware;
  DimmablePattern mDimmingPatterns[kNumDigits];
  Driver* mDriver;

  StyledPattern mStyledPatterns[kNumDigits];
  BlinkStyler* mBlinkStyler;
  PulseStyler* mPulseStyler;
  Renderer* mRenderer;

  CharWriter* mCharWriter;

  uint16_t mLastStatsCounter = 0;
  uint16_t mCurrentStatsCounter = 0;
};

#endif
