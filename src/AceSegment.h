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

/**
 * @mainpage AceSegment Library
 *
 * This is the Doxygen documentation for the
 * <a href="https://github.com/bxparks/AceSegment">AceSegment Library</a>.
 *
 * Click on the "Classes" menu above to see the list of classes.
 *
 * Click on the "Files" menu above to see the list of header files.
 */

#ifndef ACE_SEGMENT_ACE_SEGMENT_H
#define ACE_SEGMENT_ACE_SEGMENT_H

// Blacklist platforms using https://github.com/arduino/ArduinoCore-api due to
// incompatibilities.
#if defined(ARDUINO_API_VERSION)
#error Platforms using ArduinoCore-API not supported
#endif

// Version format: xxyyzz == "xx.yy.zz"
#define ACE_SEGMENT_VERSION 1300
#define ACE_SEGMENT_VERSION_STRING "0.13.0"

#include "ace_segment/hw/ClockInterface.h"
#include "ace_segment/hw/GpioInterface.h"
#include "ace_segment/hw/remap.h"
#include "ace_segment/scanning/LedMatrixDirect.h"
#include "ace_segment/scanning/LedMatrixSingleHc595.h"
#include "ace_segment/scanning/LedMatrixDualHc595.h"
#include "ace_segment/LedModule.h"
#include "ace_segment/scanning/ScanningModule.h"
#include "ace_segment/direct/DirectModule.h"
#include "ace_segment/hybrid/HybridModule.h"
#include "ace_segment/hc595/Hc595Module.h"
#include "ace_segment/tm1637/Tm1637Module.h"
#include "ace_segment/tm1638/Tm1638Module.h"
#include "ace_segment/tm1638/Tm1638AnodeModule.h"
#include "ace_segment/max7219/Max7219Module.h"
#include "ace_segment/ht16k33/Ht16k33Module.h"

#endif
