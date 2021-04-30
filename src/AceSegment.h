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
 */

#ifndef ACE_SEGMENT_ACE_SEGMENT_H
#define ACE_SEGMENT_ACE_SEGMENT_H

#include "ace_segment/hw/ClockInterface.h"
#include "ace_segment/hw/GpioInterface.h"
#include "ace_segment/hw/SwSpiInterface.h"
#include "ace_segment/hw/HwSpiInterface.h"
#include "ace_segment/hw/SwWireInterface.h"
#include "ace_segment/scanning/LedMatrixDirect.h"
#include "ace_segment/scanning/LedMatrixSingleShiftRegister.h"
#include "ace_segment/scanning/LedMatrixDualShiftRegister.h"
#include "ace_segment/LedModule.h"
#include "ace_segment/scanning/ScanningModule.h"
#include "ace_segment/hc595/DualHc595Module.h"
#include "ace_segment/tm1637/Tm1637Module.h"
#include "ace_segment/max7219/Max7219Module.h"
#include "ace_segment/LedDisplay.h"
#include "ace_segment/writer/NumberWriter.h"
#include "ace_segment/writer/ClockWriter.h"
#include "ace_segment/writer/TemperatureWriter.h"
#include "ace_segment/writer/CharWriter.h"
#include "ace_segment/writer/StringWriter.h"
#include "ace_segment/writer/StringScroller.h"

// Version format: xxyyzz == "xx.yy.zz"
#define ACE_SEGMENT_VERSION 400
#define ACE_SEGMENT_VERSION_STRING "0.4"

#endif
