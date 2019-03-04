#
# Makefile for gemhardware/utils subpackage
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
Package=gemhardware/utils
LongPackage=gemhardware/utils
ShortPackage=hw/utils
PackageName=hwutils

ProjectBase = $(BUILD_HOME)
CfgDir      = $(ProjectBase)/config

GEMHWUTILS_VER_MAJOR=1
GEMHWUTILS_VER_MINOR=0
GEMHWUTILS_VER_PATCH=1

include $(BUILD_HOME)/config/mfDefsGEM.mk
include $(BUILD_HOME)/config/mfPythonDefsGEM.mk

Sources =version.cc
Sources+=GEMCrateUtils.cc

DynamicLibrary=gemhwutils

IncludeDirs+=$(BUILD_HOME)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/gemutils/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

UserCFlags  +=$(PYTHONCFLAGS)
UserCCFlags +=$(PYTHONCFLAGS)

DependentLibraries+=gemutils

Libraries =

UserCFlags+=-O0 -g3 -fno-inline
UserCCFlags+=-O0 -g3 -fno-inline
CFlags+=-O0 -g3 -fno-inline
CCFlags+=-O0 -g3 -fno-inline

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
