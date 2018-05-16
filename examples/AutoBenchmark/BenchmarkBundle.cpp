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

#include <Arduino.h>
#include "BenchmarkBundle.h"
#ifdef __AVR__
  #include "FastDirectDriver.h"
  #include "FastSerialDriver.h"
  #include "FastSpiDriver.h"
#endif

#if defined(ESP8266)
  // NodeMCU doesn't have 12 available pins to directly drive 4 digits with 8
  // segments. Serial or SPI drivers are probably the only opion (though
  // haven't tested in real life). For the  purposes of the AutoBenchmark,
  // just use some arbitrary pins which don't cause trouble.
  const uint8_t BenchmarkBundle::kDigitPins[kNumDigits] = {D3, D4, D5, D6};
  const uint8_t BenchmarkBundle::kSegmentDirectPins[8] =
      {D7, D7, D7, D7, D7, D7, D7, D7};
#elif defined(ESP32)
  // Use some random GPIO pins for benchmark purposes.
  const uint8_t BenchmarkBundle::kDigitPins[kNumDigits] = {T3, T4, T5, T6};
  const uint8_t BenchmarkBundle::kSegmentDirectPins[8] =
      {T7, T7, T7, T7, T7, T7, T7, T7};
#else
  // These pins should work for AVR and Teensy.
  const uint8_t BenchmarkBundle::kDigitPins[kNumDigits] = {4, 5, 6, 7};
  const uint8_t BenchmarkBundle::kSegmentDirectPins[8] =
      {8, 9, 10, 11, 12, 14, 15, 13};
#endif

// Normally it's not a good idea to do so much in a constructor but this is
// just a diagnostic program.
BenchmarkBundle::BenchmarkBundle(const DriverConfig* driverConfig) {
  mHardware = new Hardware();

  // Create the Driver.
  if (driverConfig->mFast) {
#ifdef __AVR__
    if (driverConfig->mPinWiring == DriverConfig::DirectPins) {
      mDriver = new FastDirectDriver(
          mDimmingPatterns, kNumDigits, kNumSubFields);
    } else if (driverConfig->mPinWiring == DriverConfig::SerialPins) {
      mDriver = new FastSerialDriver(
          mDimmingPatterns, kNumDigits, kNumSubFields);
    } else {
      mDriver = new FastSpiDriver(
          mDimmingPatterns, kNumDigits, kNumSubFields);
    }
#endif
  } else {
    DriverBuilder builder = DriverBuilder(mHardware)
          .setDimmablePatterns(mDimmingPatterns);
    if (driverConfig->mResistorWiring == DriverConfig::ResistorsOnSegments) {
      builder.setNumDigits(kNumDigits)
          .setCommonCathode()
          .setResistorsOnSegments()
          .setDigitPins(kDigitPins);
      if (driverConfig->mPinWiring == DriverConfig::DirectPins) {
        builder.setSegmentDirectPins(kSegmentDirectPins);
      } else if (driverConfig->mPinWiring == DriverConfig::SerialPins) {
        builder.setSegmentSerialPins(kLatchPin, kDataPin, kClockPin);
      } else {
        builder.setSegmentSpiPins(kLatchPin, kDataPin, kClockPin);
      }
      if (driverConfig->mModulation == DriverConfig::UseModulation) {
        builder.useModulatingDriver(kNumSubFields);
      }
      mDriver = builder.build();
    } else {
      mDriver = builder.setNumDigits(kNumDigits)
        .setCommonCathode()
        .setResistorsOnDigits()
        .setDigitPins(kDigitPins)
        .setSegmentDirectPins(kSegmentDirectPins)
        .setDimmablePatterns(mDimmingPatterns)
        .build();
    }
  }

  // Create the Renderer.
  memset(mStyledPatterns, 0, kNumDigits * sizeof(StyledPattern));
  mBlinkStyler = new BlinkStyler(kFramePerSecond, kBlinkDuration);
  mPulseStyler = new PulseStyler(kFramePerSecond, kPulseDuration);
  RendererBuilder rendererBuilder =
      RendererBuilder(mHardware, mDriver, mStyledPatterns, kNumDigits)
      .setFramesPerSecond(kFramePerSecond)
      .setStatsResetInterval(0);
  if (driverConfig->mStyle == DriverConfig::UseStyles) {
      rendererBuilder
          .setStyler(kBlinkStyle, mBlinkStyler)
          .setStyler(kPulseStyle, mPulseStyler);
  }
  mRenderer = rendererBuilder.build();
  mCharWriter = new CharWriter(mRenderer);
}
