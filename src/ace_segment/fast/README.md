# Fast Variants using DigitalWriteFast

These classes are optimized for AVR processors using the `pinModeFast()` and
`digitalWriteFast()` methods provided by:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast, or
* https://github.com/NicksonYap/digitalWriteFast

The default `digitalWrite()` on a 16 MHz ATMega328 takes about 6.3 micros. But
`digitalWriteFast()` takes only 0.125 micros by writing to the port registers
directly. The catch is that `digitalWriteFast()` must be given compile-time
constants for both the `pin` and `value` to achieve this fast result.

To take advantage of the optimization to occur, I use 2 techinques:

* the pin numbers are defined at compile-time using C++ template parameters,
* a lookup table of pre-generated `digitalWriteFast()` function pointers is
  used to convert a group or element pin number to the appropriate
  `digitalWriteFast()` routine (See `LedMatrixDirectFast.h`).

## Usage

These header files are *not* included in the master `<AceSegment.h>` header
file, because these depend on one of the external libraries listed above. If
you want to use them, you need to include these headers explicitly:

```C++
#include <AceSegment.h> // do this first
#include <ace_segment/fast/LedMatrixDirectFast.h>
#include <ace_segment/fast/SwSpiAdapterFast.h>
```
