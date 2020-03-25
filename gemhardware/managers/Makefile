#
# Makefile for gemhardware/managers subpackage
#

BUILD_HOME?= $(shell dirname `cd ../; pwd`)

ifndef BUILD_SUPPORT
BUILD_SUPPORT=build
endif

ifndef PROJECT_NAME
PROJECT_NAME=cmsgemos
endif

ifndef GEM_VARIANT
GEM_VARIANT = ge11
endif

Project=$(PROJECT_NAME)
ShortProject=gem
Package=gemhardware/managers
LongPackage=gemhardware/managers
ShortPackage=hw/managers
PackageName=hwmanagers

ProjectBase = $(BUILD_HOME)
CfgDir      = $(ProjectBase)/config

GEMHWMANAGERS_VER_MAJOR=1
GEMHWMANAGERS_VER_MINOR=0
GEMHWMANAGERS_VER_PATCH=1

include $(CfgDir)/mfDefsGEM.mk

UserCFlags+=-DGEM_VARIANT="$(GEM_VARIANT)"

Sources =version.cc
# Sources+=utils/GEMCrateUtils.cc
Sources+=amc13/AMC13Manager.cc amc13/AMC13ManagerWeb.cc amc13/AMC13Readout.cc
Sources+=amc/AMCManager.cc amc/AMCManagerWeb.cc 
Sources+=optohybrid/OptoHybridManager.cc optohybrid/OptoHybridManagerWeb.cc 
#Sources+=GEMController.cc GEMControllerPanelWeb.cc

DynamicLibrary=gemhwmanagers

IncludeDirs+=$(BUILD_HOME)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/gemhardware/devices/include
IncludeDirs+=$(BUILD_HOME)/gemhardware/utils/include
IncludeDirs+=$(BUILD_HOME)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/gembase/include
IncludeDirs+=$(BUILD_HOME)/gemreadout/include
IncludeDirs+=$(uHALROOT)/include
IncludeDirs+=$(XHAL_ROOT)/include
IncludeDirs+=$(CTP7_ROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemhardware/devices/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemhardware/utils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemreadout/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(uHALROOT)/lib
DependentLibraryDirs+=$(XHAL_ROOT)/lib

LibraryDirs+=$(BUILD_HOME)/gemhardware/devices/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/gemhardware/utils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

DependentLibraries =gemreadout gemhwutils gemhwdevices
DependentLibraries+=cactus_amc13_amc13 cactus_amc13_tools

DependentLibraries+=cactus_amc13_tools
DependentLibraries+=gemutils gembase gemreadout gemhwutils gemhwdevices

Libraries =

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
