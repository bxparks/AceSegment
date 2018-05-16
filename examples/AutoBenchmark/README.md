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

## Teensy 3.2

```
------------+--------+------------+------+--------+-------------+
resistorsOn | wiring | modulation | fast | styles | min/avg/max |
------------|--------|------------|------|--------|-------------|
digits      | direct |            |      |        |   4/  4/ 10 |
digits      | direct |            |      | styles |   4/  8/ 16 |
digits      | serial |            |      |        |   4/  5/ 11 |
digits      | serial |            |      | styles |   4/  8/ 17 |
digits      | spi    |            |      |        |   4/  5/ 11 |
digits      | spi    |            |      | styles |   4/  8/ 17 |
segments    | direct |            |      |        |   3/  4/ 10 |
segments    | direct |            |      | styles |   3/  9/ 18 |
segments    | serial |            |      |        |   3/  4/ 10 |
segments    | serial |            |      | styles |   3/ 12/ 22 |
segments    | spi    |            |      |        |   3/  4/ 10 |
segments    | spi    |            |      | styles |   3/  7/ 15 |
segments    | direct | modulation |      |        |   1/  2/  8 |
segments    | direct | modulation |      | styles |   1/  2/ 18 |
segments    | serial | modulation |      |        |   1/  2/  8 |
segments    | serial | modulation |      | styles |   1/  2/ 22 |
segments    | spi    | modulation |      |        |   1/  2/  9 |
segments    | spi    | modulation |      | styles |   1/  2/ 16 |
------------+--------+------------+------+--------+-------------+

```

## ESP8266 NodeMCU v1.0

I'm not sure that the `max` numbers below should be trusted, since the WiFi code
in the single-core ESP8266 adds random latency into the normal code execution
path.

```
------------+--------+------------+------+--------+-------------+
resistorsOn | wiring | modulation | fast | styles | min/avg/max |
------------|--------|------------|------|--------|-------------|
digits      | direct |            |      |        |   6/  6/ 81 |
digits      | direct |            |      | styles |   6/ 12/ 43 |
digits      | serial |            |      |        |   6/  6/ 30 |
digits      | serial |            |      | styles |   6/ 12/ 39 |
digits      | spi    |            |      |        |   6/  6/ 29 |
digits      | spi    |            |      | styles |   6/ 12/ 39 |
segments    | direct |            |      |        |   4/  5/ 25 |
segments    | direct |            |      | styles |   4/ 15/ 36 |
segments    | serial |            |      |        |   4/  5/ 28 |
segments    | serial |            |      | styles |   4/ 26/ 57 |
segments    | spi    |            |      |        |   4/  5/ 16 |
segments    | spi    |            |      | styles |   4/ 14/ 93 |
segments    | direct | modulation |      |        |   1/  2/ 28 |
segments    | direct | modulation |      | styles |   1/  2/ 32 |
segments    | serial | modulation |      |        |   1/  2/  9 |
segments    | serial | modulation |      | styles |   1/  3/ 39 |
segments    | spi    | modulation |      |        |   1/  2/ 12 |
segments    | spi    | modulation |      | styles |   1/  2/ 63 |
------------+--------+------------+------+--------+-------------+
```

## ESP32 Node32s

```
------------+--------+------------+------+--------+-------------+
resistorsOn | wiring | modulation | fast | styles | min/avg/max |
------------|--------|------------|------|--------|-------------|
digits      | direct |            |      |        |   4/  5/ 41 |
digits      | direct |            |      | styles |   4/  6/ 22 |
digits      | serial |            |      |        |   4/  5/ 13 |
digits      | serial |            |      | styles |   4/  6/ 14 |
digits      | spi    |            |      |        |   4/  5/ 13 |
digits      | spi    |            |      | styles |   4/  6/ 14 |
segments    | direct |            |      |        |   2/  2/ 14 |
segments    | direct |            |      | styles |   2/  4/ 15 |
segments    | serial |            |      |        |   2/  2/ 14 |
segments    | serial |            |      | styles |   2/  5/ 21 |
segments    | spi    |            |      |        |   2/  2/ 10 |
segments    | spi    |            |      | styles |   2/  9/ 51 |
segments    | direct | modulation |      |        |   1/  1/ 18 |
segments    | direct | modulation |      | styles |   1/  2/ 14 |
segments    | serial | modulation |      |        |   1/  1/  9 |
segments    | serial | modulation |      | styles |   1/  2/ 15 |
segments    | spi    | modulation |      |        |   1/  1/  9 |
segments    | spi    | modulation |      | styles |   1/  2/ 19 |
------------+--------+------------+------+--------+-------------+
```
