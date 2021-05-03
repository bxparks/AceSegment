# Source Code Organization

All classes are in the `ace_segment` namespace, but the `.h` and `.cpp` files
are organized in subdirectories like this:

```
                    writer/
                      *Writer.h
                        |
                        v
                    ace_segment/
                      LedDisplay.h
                        |
                        v
                    ace_segment/
                      LedModule.h
                        ^
                        |
+-----------------------+---------------------+
|                       |                     |
|                       |                     |
scanning/               tm1637/               max7219/
  ScanningDisplay.h       Tm1637Module.h        Max7219Module.h
  LedMatrix*.h              |                   /
  LedMatrixDirectFast4.h    |                  /
                  \         |                 /
                   \        |                /
                    v       v               v
                        hw/
                        ClockInterface.h
                        GpioInterface.h
                        SoftSpiInterface.h
                        SoftSpiFastInterface.h
                        HardSpiInterface.h
                        HardSpiFastInterface.h
```
