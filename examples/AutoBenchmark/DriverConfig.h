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

#ifndef DRIVER_CONFIG_H
#define DRIVER_CONFIG_H

#include <stdint.h>

struct DriverConfig {
  enum ResistorWiring {
    ResistorsOnDigits,
    ResistorsOnSegments
  };
  
  enum PinWiring {
    DirectPins,
    SerialPins,
    SpiPins
  };

  enum Modulation {
    NoModulation,
    UseModulation
  };

  enum Fast {
    NoFastDriver,
    UseFastDriver
  };

  DriverConfig(
      ResistorWiring resistorWiring, PinWiring pinWiring,
      Modulation modulation, Fast fast, const char* label):
    mResistorWiring(resistorWiring),
    mPinWiring(pinWiring),
    mModulation(modulation),
    mFast(fast),
    mLabel(label)
  {}

  const ResistorWiring mResistorWiring;
  const PinWiring mPinWiring;
  const Modulation mModulation;
  const Fast mFast;
  const char* const mLabel;

  static const DriverConfig kDriverConfigs[];
  static const uint8_t kNumDriverConfigs;
};

#endif
