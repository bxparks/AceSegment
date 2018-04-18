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

/*
 * A sketch that generates the min/avg/max (in microsecondes) benchmarks of
 * Renderer::renderField() for all versions of the Driver which are supported
 * by AceSegment library. The output is an ASCII formatted table that can be
 * pasted directly into the README.md file as a code block. This saves a lot of
 * error-prone manual collection of these numbers.
 *
 * DriverConfig contains an enumeration of all DriverBuilder configurations
 * which are currently supported. For each DriverConfig, the DriverBuilder is
 * used to construct the Renderer stack and all of its dependencies. It calls
 * Renderer::displayField() a number of times (NUM_FIELD_SAMPLES is 1800), then
 * retrieves the TimingStats from the Renderer, and prints out the min/avg/max
 * numbers.
 */

#include <stdio.h>
#include <AceSegment.h>
#include "FastDirectDriver.h"
#include "FastSerialDriver.h"
#include "FastSpiDriver.h"
#include "DriverConfig.h"
#include "BenchmarkBundle.h"

using namespace ace_segment;

void writeChars();
void nextBenchmark();

//------------------------------------------------------------------
// Setup for AutoBenchmark
//------------------------------------------------------------------
const uint8_t LOOP_MODE_BEGIN = 0;
const uint8_t LOOP_MODE_RENDER = 1;
const uint8_t LOOP_MODE_NEXT_DRIVER = 2;
const uint8_t LOOP_MODE_FOOTER = 3;
const uint8_t LOOP_MODE_DONE = 4;
static uint8_t loopMode = LOOP_MODE_BEGIN;

const DriverConfig* driverConfig = nullptr;
BenchmarkBundle* benchmarkBundle = nullptr;

void setup() {
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
  Serial.begin(115200); // ESP8266 default of 74880 not supported on Linux
  while (!Serial); // Wait until Serial is ready - Leonardo/Micro
  Serial.println(F("setup(): begin"));

  driverConfig = &DriverConfig::kDriverConfigs[0];
  setupBenchmark(driverConfig);

  Serial.println(F("setup(): end"));
}

void setupBenchmark(const DriverConfig* driverConfig) {
  if (benchmarkBundle != nullptr) delete benchmarkBundle;

  benchmarkBundle = new BenchmarkBundle(driverConfig);
  benchmarkBundle->configure();
  benchmarkBundle->mStringWriter->writeStringAt(0, "1234");
}

//------------------------------------------------------------------
// Loop for AutoBenchmark
//------------------------------------------------------------------

void loop() {
  if (loopMode == LOOP_MODE_BEGIN) {
    Serial.println(
        F("------------+-----------+------------+------+-------------+"));
    Serial.println(
        F("resistorsOn | pinWiring | modulation | fast | min/avg/max |"));
    Serial.println(
        F("------------|-----------|------------|------|-------------|"));
    loopMode = LOOP_MODE_RENDER;
  } else if (loopMode == LOOP_MODE_RENDER) {
    render();
  } else if (loopMode == LOOP_MODE_NEXT_DRIVER) {
    nextBenchmark();
  } else if (loopMode == LOOP_MODE_FOOTER) {
    Serial.println(
        F("------------+-----------+------------+------+-------------+"));
    loopMode = LOOP_MODE_DONE;
  }
}

void render() {
  // Number of renderFields() to sample, should be between (1, 2) *
  // RenderBuilder::kStatsResetIntervalDefault so that we have one reset of
  // TimingStats to avoid spurious times in the initial iterations. For 4
  // digits, with 16 subfields/field, there are 64 fields per frame. So 1800
  // fields means 28 frames, which means about 1/2 second at 60 frames per
  // second.
  const uint16_t NUM_FIELD_SAMPLES = 1800;

  bool isRendered = benchmarkBundle->mRenderer->renderFieldWhenReady();

  if (isRendered) {
    uint16_t elapsedCount = benchmarkBundle->mCurrentStatsCounter - 
        benchmarkBundle->mLastStatsCounter;
    if (elapsedCount >= NUM_FIELD_SAMPLES) {
      TimingStats stats = benchmarkBundle->mRenderer->getTimingStats();
      printTimingStats(driverConfig, stats);
      loopMode = LOOP_MODE_NEXT_DRIVER;
    } else {
      benchmarkBundle->mCurrentStatsCounter++;
    }
  }
}

void nextBenchmark() {
  driverConfig++;
  if (driverConfig >=
      &DriverConfig::kDriverConfigs[0] + DriverConfig::kNumDriverConfigs) {
    loopMode = LOOP_MODE_FOOTER;
  } else {
    setupBenchmark(driverConfig);
    loopMode = LOOP_MODE_RENDER;
  }
}

void printTimingStats(const DriverConfig* driverConfig,
    const TimingStats& stats) {
  Serial.print(driverConfig->mLabel);

  char buf[15]; // 12 should be enough, but give 3 more just in case
  sprintf(buf, " %3d/%3d/%3d |", stats.getMin(), stats.getAvg(), 
      stats.getMax());
  Serial.println(buf);
}
