#
# Makefile for gembase package
#
BUILD_HOME:=$(shell pwd)/..

ifndef BUILD_SUPPORT
BUILD_SUPPORT=build
endif

ifndef PROJECT_NAME
PROJECT_NAME=cmsgemos
endif

Project=$(PROJECT_NAME)
ShortProject=gem
Package=gembase
LongPackage=gembase
ShortPackage=base
PackageName=base

GEMBASE_VER_MAJOR=0
GEMBASE_VER_MINOR=0
GEMBASE_VER_PATCH=0

include $(BUILD_HOME)/config/mfDefsGEM.mk

Sources =version.cc
Sources+=GEMApplication.cc GEMFSMApplication.cc GEMFSM.cc
Sources+=GEMWebApplication.cc GEMMonitor.cc
Sources+=utils/GEMInfoSpaceToolBox.cc

DynamicLibrary=gembase

IncludeDirs+=$(BUILD_HOME)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/gemutils/include

DependentLibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)

DependentLibraries+=gemutils

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
