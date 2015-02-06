TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

ifeq (windows-x64, $(findstring windows-x64, $(T_A)))
USR_CPPFLAGS += -D__X64
endif

LIBRARY_IOC_WIN32 = Dexela
LIB_SRCS_WIN32 += Dexela.cpp
LIB_LIBS += DexelaDetector
LIB_LIBS += DexelaException
LIB_LIBS += BusScanner
LIB_LIBS += DexImage

DBD += DexelaSupport.dbd

include $(ADCORE)/ADApp/commonLibraryMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

