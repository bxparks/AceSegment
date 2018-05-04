# AutoBenchmark

The `AutoBenchmark` sketch iterates through every supported variation of
`Driver`, calls `renderField()` about a 1000 times, then reports the min/avg/max
CPU time (in microseconds) collected by the `TimingStats` object in the
`Renderer`. It uses a frame rate of 60Hz, with 16 subfields per field for the
`useModulatingDriver()` option.

## Arduino Nano clone
* 16MHz ATmega328P

```
------------+--------+------------+------+--------+-------------+
resistorsOn | wiring | modulation | fast | styles | min/avg/max |
------------|--------|------------|------|--------|-------------|
digits      | direct |            |      |        |  32/ 37/ 64 |
digits      | direct |            |      | styles |  32/ 67/112 |
digits      | serial |            |      |        |  32/ 37/ 60 |
digits      | serial |            |      | styles |  32/ 67/112 |
digits      | spi    |            |      |        |  32/ 37/ 64 |
digits      | spi    |            |      | styles |  32/ 67/112 |
segments    | direct |            |      |        |  24/ 33/ 56 |
segments    | direct |            |      | styles |  24/ 78/140 |
segments    | serial |            |      |        |  24/ 33/ 56 |
segments    | serial |            |      | styles |  24/135/224 |
segments    | spi    |            |      |        |  24/ 33/ 56 |
segments    | spi    |            |      | styles |  24/ 51/ 96 |
segments    | direct | modulation |      |        |  12/ 14/ 60 |
segments    | direct | modulation |      | styles |  12/ 18/152 |
segments    | serial | modulation |      |        |  12/ 14/ 60 |
segments    | serial | modulation |      | styles |  12/ 22/236 |
segments    | spi    | modulation |      |        |  12/ 14/ 60 |
segments    | spi    | modulation |      | styles |  12/ 16/116 |
segments    | direct | modulation | fast |        |  12/ 14/ 52 |
segments    | direct | modulation | fast | styles |  12/ 16/ 96 |
segments    | serial | modulation | fast |        |  12/ 14/ 48 |
segments    | serial | modulation | fast | styles |  12/ 15/ 92 |
segments    | spi    | modulation | fast |        |  12/ 14/ 48 |
segments    | spi    | modulation | fast | styles |  12/ 15/ 84 |
------------+--------+------------+------+--------+-------------+
```
