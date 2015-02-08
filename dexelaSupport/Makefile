TOP=../
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

# Note, the .h and .lib files were manually copied from the Dexela SDK directories

INC += BusScanner.h
INC += DexDefines.h
INC += DexDefs.h
INC += DexImage.h
INC += DexelaDetector.h
INC += DexelaDetectorCL.h
INC += DexelaDetectorGE.h
INC += DexelaException.h

ifeq (win32-x86, $(findstring win32-x86, $(T_A)))
LIB_INSTALLS_WIN32    += ../os/win32-x86/BusScanner.lib
LIB_INSTALLS_WIN32    += ../os/win32-x86/DexImage.lib
LIB_INSTALLS_WIN32    += ../os/win32-x86/DexelaDetector.lib
LIB_INSTALLS_WIN32    += ../os/win32-x86/DexelaException.lib

else ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
LIB_INSTALLS_WIN32    += ../os/windows-x64/BusScanner.lib
LIB_INSTALLS_WIN32    += ../os/windows-x64/DexImage.lib
LIB_INSTALLS_WIN32    += ../os/windows-x64/DexelaDetector.lib
LIB_INSTALLS_WIN32    += ../os/windows-x64/DexelaException.lib

endif

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

