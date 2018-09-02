# Class Hierarchy and Wiring Configurations

Internal notes to help me remember the various class hierarchies and
configuration combinations. I think I can create a templatized wrapper class
that incorporates all this information and expose a much easier API to the user
to help create the right set of objects for the given LED wiring situtation.

## Classes

* LedMatrix
  * LedMatrixSplit
    * LedMatrixSplitDirect
    * LedMatrixSplitSerial
    * LedMatrixSplitSpi
  * LedMatrixMerged
    * LedMatrixMergedSerial
    * LedMatrixMergedSpi

Drivers
  * SplitDirectDigitDriverModule
    * LedMatrixSplitDirect
    * SplitDigitDriver
  * SplitDirectSegmentDriverModule
    * LedMatrixSplitDirect
    * SplitSegmentDriver
  * SplitSerialDigitDriverModule
    * LedMatrixSplitSerial
    * SplitDigitDriver
  * SplitSpiDigitDriverModule
    * LedMatrixSplitSpi
    * SplitDigitDriver
  * MergedSerialDigitDriverModule
    * LedMatrixMergedSerial
    * MergedDigitDriver
  * MergedSpiDigitDriverModule
    * LedMatrixMergedSpi
    * MergedDigitDriver

## Wiring

* Split
    * SplitDirect
    * SplitSerial
    * SplitSpi
* Merged
    * MergedSerial
    * MergedSpi
* Driver
    * SplitDigitDriver
    * SplitSegmentDriver
    * MergedDigitDriver
    * MergedSegmentDriver (X)
