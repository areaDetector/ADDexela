ADDexela Releases
======================

The latest untagged master branch can be obtained at
https://github.com/areaDetector/ADDexela.

Tagged source code releases can be obtained at 
https://github.com/areaDetector/ADDexela/releases.

Tagged prebuilt binaries can be obtained at
http://cars.uchicago.edu/software/pub/ADDexela.

The versions of EPICS base, asyn, and other synApps modules used for each release can be obtained from 
the EXAMPLE_RELEASE_PATHS.local, EXAMPLE_RELEASE_LIBS.local, and EXAMPLE_RELEASE_PRODS.local
files respectively, in the configure/ directory of the appropriate release of the 
[top-level areaDetector](https://github.com/areaDetector/areaDetector) repository.


Release Notes
=============

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

