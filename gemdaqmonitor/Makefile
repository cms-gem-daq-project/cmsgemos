#
# Makefile for gemdaqmonitor package
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
Package=gemdaqmonitor
LongPackage=gemdaqmonitor
ShortPackage=daqmon
PackageName=daqmonitor

GEMDAQMONITOR_VER_MAJOR=1
GEMDAQMONITOR_VER_MINOR=0
GEMDAQMONITOR_VER_PATCH=0

include $(BUILD_HOME)/config/mfDefsGEM.mk

Sources =version.cc
Sources+=DaqMonitor.cc ShelfMonitor.cc ShelfMonitorWeb.cc

DynamicLibrary=gemdaqmonitor

IncludeDirs+=$(BUILD_HOME)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/gembase/include
IncludeDirs+=$(BUILD_HOME)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/gemhardware/devices/include
IncludeDirs+=$(BUILD_HOME)/gemhardware/utils/include
IncludeDirs+=$(XHAL_ROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemhardware/devices/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemhardware/utils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(XHAL_ROOT)/lib

DependentLibraries =gembase
DependentLibraries+=gemhwdevices gemhwutils xhal-client
DependentLibraries+=boost_iostreams

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
