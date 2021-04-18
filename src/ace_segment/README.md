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
  LedModule.h
    ^
    |
    +-------+
            |
            |
        scanning/
          ScanningDisplay.h
          LedMatrix*.h
          LedMatrixDirectFast.h
             |
             |
             v
         hw/
           ClockInterface.h
           GpioInterface.h
           SwSpiInterface.h
           SwSpiFastInterface.h
           HwSpiInterface.h
```
