#
# Makefile for gemHwMonitor package
#

#Project=gem
Project=gemdaq-testing
ShortProject=gem
Package=gemHwMonitor
LongPackage=gemHwMonitor
ShortPackage=hwMonitor
PackageName=hwMonitor

GEMHWMONITOR_VER_MAJOR=0
GEMHWMONITOR_VER_MINOR=1
GEMHWMONITOR_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem

Sources =version.cc
Sources+=gemHwMonitorWeb.cc gemHwMonitorBase.cc

DynamicLibrary=gemhwMonitor

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemhardware/include
IncludeDirs+=$(AMC13_STANDALONE_ROOT)/amc13/include
IncludeDirs+=$(uHALROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(AMC13_STANDALONE_ROOT)/amc13/lib
DependentLibraryDirs+=$(uHALROOT)/lib

LibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(AMC13_STANDALONE_ROOT)/amc13/lib
LibraryDirs+=$(uHALROOT)/lib

DependentLibraries+=cactus_uhal_uhal cactus_amc13_amc13
DependentLibraries+=gemutils gembase gemhardware

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPM_gem.rules
include $(XDAQ_ROOT)/config/mfRPM.rules

print-env:
	@echo BUILD_HOME    $(BUILD_HOME)
	@echo XDAQ_ROOT     $(XDAQ_ROOT)
	@echo XDAQ_OS       $(XDAQ_OS)
	@echo XDAQ_PLATFORM $(XDAQ_PLATFORM)
	@echo LIBDIR        $(LIBDIR)
	@echo ROOTCFLAGS    $(ROOTCFLAGS)
	@echo ROOTLIBS      $(ROOTLIBS)
	@echo ROOTGLIBS     $(ROOTGLIBS)
