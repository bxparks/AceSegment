/*
MIT License

Copyright (c) 2021 Brian T. Park

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

#ifndef ACE_SEGMENT_TEMPERATURE_WRITER_H
#define ACE_SEGMENT_TEMPERATURE_WRITER_H

#include <stdint.h>
#include "../LedModule.h"
#include "NumberWriter.h"

namespace ace_segment {

/**
 * The TemperatureWriter supports writing integer temperature values in Celcius
 * or Farenheit. Negative values are supported.
 */
class TemperatureWriter {
  public:
    /** The superscript degree symbol for temperature. */
    static const uint8_t kPatternDegree = 0b01100011;

    /** The "C" character for "Celcius". */
    static const uint8_t kPatternC = 0b00111001;

    /** The "F" character for "Farenheit". */
    static const uint8_t kPatternF = 0b01110001;

    /**
     * Constructor.
     *
     * @param ledModule instance of LedModule
     */
    explicit TemperatureWriter(LedModule& ledModule) :
        mNumberWriter(ledModule)
    {}

    /** Get the underlying LedModule. */
    LedModule& ledModule() { return mNumberWriter.ledModule(); }

    /** Get the underlying PatternWriter. */
    PatternWriter& patternWriter() { return mNumberWriter.patternWriter(); }

    /**
     * Write signed integer temperature without deg or unit within the boxSize.
     */
    uint8_t writeTempAt(uint8_t pos, int16_t temp, int8_t boxSize = 0) {
      return mNumberWriter.writeSignedDecimalAt(pos, temp, boxSize);
    }

    /**
     * Write integer temperature with degree symbol.
     *
     * The caller is responsible for verifying that the temp value fits inside
     * the `boxSize`. For example, if boxSize is 2, then the range of temp value
     * is [-9, 99]. If the boxSize is too small, the digits will bleed to the
     * right of the box.
     *
     * @return number of digits written, including any '-' or space characters
     */
    uint8_t writeTempDegAt(uint8_t pos, int16_t temp, int8_t boxSize = 0) {
      uint8_t written = mNumberWriter.writeSignedDecimalAt(pos, temp,
          boxSize >= 1 ? boxSize - 1 : 0);
      pos += written;
      patternWriter().writePatternAt(pos++, kPatternDegree);
      return written + 1;
    }

    /**
     * Write integer temperature with degree symbol and 'C' symbol.
     */
    uint8_t writeTempDegCAt(uint8_t pos, int16_t temp, int8_t boxSize = 0) {
      uint8_t written = mNumberWriter.writeSignedDecimalAt(pos, temp,
          boxSize >= 2 ? boxSize - 2 : 0);
      pos += written;
      patternWriter().writePatternAt(pos++, kPatternDegree);
      patternWriter().writePatternAt(pos++, kPatternC);
      return written + 2;
    }

    /**
     * Write integer temperature with degree symbol and 'F' symbol.
     */
    uint8_t writeTempDegFAt(uint8_t pos, int16_t temp, int8_t boxSize = 0) {
      uint8_t written = mNumberWriter.writeSignedDecimalAt(pos, temp,
          boxSize >= 2 ? boxSize - 2 : 0);
      pos += written;
      patternWriter().writePatternAt(pos++, kPatternDegree);
      patternWriter().writePatternAt(pos++, kPatternF);
      return written + 2;
    }

    /** Clear the entire display. */
    void clear() { mNumberWriter.clearToEnd(0); }

    /** Clear the display from `pos` to the end. */
    void clearToEnd(uint8_t pos) { mNumberWriter.clearToEnd(pos); }

  private:
    // disable copy-constructor and assignment operator
    TemperatureWriter(const TemperatureWriter&) = delete;
    TemperatureWriter& operator=(const TemperatureWriter&) = delete;

    NumberWriter mNumberWriter;
};

} // ace_segment

#endif
