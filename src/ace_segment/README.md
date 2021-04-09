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
    ^
    |
    +-------+
            |
            |
      scanning/
        ScanningDisplay.h
        LedMatrix*.h
        /          \
       /            \
      v              v
    hw/              fast/
      Hardware.h       SwSpiAdapterFast.h
      SwSpiAdapter.h   LedMatrixDirectFast.h
      HwSpiAdapter.h
```
