ADDexela Releases
======================

The latest untagged master branch can be obtained at
https://github.com/areaDetector/ADDexela.

Tagged source code releases can be obtained at 
https://github.com/areaDetector/ADDexela/releases.

Tagged prebuilt binaries can be obtained at
https://cars.uchicago.edu/software/pub/ADDexela.

The versions of EPICS base, asyn, and other synApps modules used for each release can be obtained from 
the EXAMPLE_RELEASE_PATHS.local, EXAMPLE_RELEASE_LIBS.local, and EXAMPLE_RELEASE_PRODS.local
files respectively, in the configure/ directory of the appropriate release of the 
[top-level areaDetector](https://github.com/areaDetector/areaDetector) repository.


Release Notes
=============
R2-4 (December XXX, 2020)
----
* Removed DEXSerialNumber record.
* Implemented ADSerialNumber and ADFirmwareVersion parameters.


R2-3 (December 4, 2018)
----
* Added DEXReadoutMode record.  This can be set to Idle to disable continuous detector scrubbing.
  This allows prompt response to software or hardware triggers, at the expense of potentially larger 
  dark current in the first few frames.


R2-2 (July 2, 2018)
----
* Changed configure/RELEASE files for compatibility with areaDetector R3-3.
* Added support for new PVs in ADCore R3-3 in opi files (NumQueuedArrays, etc.)
* Added ADBuffers.adl to main medm screen.
* Improved op/*/autoconvert/* files with better medm files and better converters.
* Added NDDriverVersion information.
* Removed calls to unlock()/lock() around doCallbacksGenericPointer, not needed.
* Added op/Makefile for autoconvert.


R2-1 (July 4, 2017)
----
* Fixed layout of medm screen for ADCore R3-0.

R2-0 (February 19, 2017)
----
* Added asynPrint ASYN_TRACEIO_DRIVER to print info for all calls to vendor library
* Added destructor
* Changed layout of medm screen for ADCore R2-6.


R2-0beta2 (June 23, 2015)
----
* Fixes from Zachary Brown at Cornell to get image modes and triggers working correctly.
* Changes for ADCore R2-2.
* Fixed build system to install boost header files (thanks to Andrew Johnson).


R2-0-beta1 (February 16, 2015)
----
* Initial release

