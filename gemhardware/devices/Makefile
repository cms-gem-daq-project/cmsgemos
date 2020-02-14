#
# Makefile for gemhardware/devices subpackage
#

BUILD_HOME?= $(shell dirname `cd ../; pwd`)

ifndef BUILD_SUPPORT
BUILD_SUPPORT=build
endif

ifndef PROJECT_NAME
PROJECT_NAME=cmsgemos
endif

Project=$(PROJECT_NAME)
ShortProject=gem
Package=gemhardware/devices
LongPackage=gemhardware/devices
ShortPackage=hw/devices
PackageName=hwdevices

ProjectBase = $(BUILD_HOME)
CfgDir      = $(ProjectBase)/config

GEMHWDEVICES_VER_MAJOR=1
GEMHWDEVICES_VER_MINOR=0
GEMHWDEVICES_VER_PATCH=1

include $(CfgDir)/mfDefsGEM.mk
include $(CfgDir)/mfPythonDefsGEM.mk

Sources =version.cc
# Sources =utils/GEMCrateUtils.cc
Sources+=GEMHwDevice.cc
Sources+=amc/HwGenericAMC.cc
# Sources+=vfat/HwVFAT2.cc
Sources+=glib/HwGLIB.cc
#Sources+=ctp7/HwCTP7.cc
Sources+=optohybrid/HwOptoHybrid.cc

DynamicLibrary=gemhwdevices

IncludeDirs =$(XDAQ_ROOT)/include
IncludeDirs+=$(BUILD_HOME)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/gemhardware/utils/include
IncludeDirs+=$(BUILD_HOME)/gemutils/include
IncludeDirs+=$(uHALROOT)/include
IncludeDirs+=$(XHAL_ROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/gemhardware/utils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(uHALROOT)/lib
DependentLibraryDirs+=$(XHAL_ROOT)/lib

LibraryDirs+=$(BUILD_HOME)/gemhardware/utils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(uHALROOT)/lib
LibraryDirs+=$(XHAL_ROOT)/lib

DependentLibraries =cactus_uhal_uhal xhal-client
DependentLibraries+=gemutils gemhwutils

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/Makefile.rules
include $(BUILD_HOME)/config/mfRPMDefsGEM.mk

print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_ROOT     $(XDAQ_ROOT)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo LIBDIR        $(LIBDIR)
	@echo ROOTCFLAGS    $(ROOTCFLAGS)
	@echo ROOTLIBS      $(ROOTLIBS)
	@echo ROOTGLIBS     $(ROOTGLIBS)
	@echo GIT_VERSION   $(GIT_VERSION)
	@echo GEMDEVELOPER  $(GEMDEVELOPER)
	@echo CC            $(CC)
	@echo CPP           $(CPP)
	@echo CXX           $(CXX)
	@echo LD            $(LD)
	@echo AR            $(AR)
	@echo NM            $(NM)
	@echo RANLIB        $(RANLIB)
	@echo GCCVERSION    $(GCCVERSION)
	@echo CLANGVERSION  $(CLANGVERSION)
