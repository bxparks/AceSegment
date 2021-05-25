# TM1637 LED Module Capacitor Removal

There are at least two types of TM1637 LED modules which seem to be readily
available on Amazon and eBay currently. 

* One type is labeled as being manufactured by diymore.cc and is availabe is 5
  different colors (white, red, yellow, green, and blue).  They manufactured on
  black printed circuit board, and come in 4-digit and 6-digit variations.
* Another type is a 4-digit a blue printed circuit board, but does not have any
  labels on it.

The LED modules from diymore.cc come with 2 power capacitors across the `VCC`
and `GND` lines, and 2 capacitors on the `DIO` and `CLK` pins to ground, as
shown in the schematic below:

![TM1637 LED Module Schematic](tm1637-led-module-schematic.png)

The capacitors for `CLK` and `DIO` lines are 10 nF, which is about 100X larger
than it should be. It causes the RC time constant to be about 100 microseconds,
which forces the `BIT_DELAY` parameter in the `SoftTmiInterface` and
`SoftTmiFastInterface` classes to be 100 microseoncds. According to the TM1637
datasheet, the controller chip should be able to handle a bit delay as low as 1
microsecond (i.e. 500 kHz).

If you are handy with a soldering iron, you can remove the two 10 nF capacitors.
The capacitors are located in different places depending on the size of the LED
segments and the number of LED segments. The best way to identify the correct
capacitors is to use a multimeter and find the capacitors where one end of the
capacitor is connected to `GND` and the other end of the capacitor is connected
to either the `DIO` pin or the `CLK` pin.

Here are the photos of the LED modules that I modified, where the red box marks
the place where the capacitors were removed:

**4-digit 0.36" module**
![TM1637 4-digit 0.36"](TM1637-4-36-marked.jpg)

**4-digit 0.56" module**
![TM1637 4-digit 0.56"](TM1637-4-56-marked.jpg)

**6-digit 0.36" module**
![TM1637 6-digit 0.36"](TM1637-6-36-marked.jpg)

**6-digit 0.56" module**
![TM1637 6-digit 0.56"](TM1637-6-56-marked.jpg)

With these capacitors removed, I have verified that `BIT_DELAY` as low as 1
microsecond will work.
