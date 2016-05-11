#
# Makefile for gembase package
#

#Project=gem
Project=gemdaq-testing
ShortProject=gem
Package=gembase
LongPackage=gembase
ShortPackage=base
PackageName=base

GEMBASE_VER_MAJOR=0
GEMBASE_VER_MINOR=1
GEMBASE_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem

Sources =version.cc
Sources+=GEMApplication.cc GEMFSMApplication.cc GEMFSM.cc
Sources+=GEMWebApplication.cc GEMMonitor.cc
Sources+=utils/GEMInfoSpaceToolBox.cc

DynamicLibrary=gembase

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
#IncludeDirs+=$(uHALROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
#DependentLibraryDirs+=$(uHALROOT)/lib

#LibraryDirs+=$(uHALROOT)/lib

DependentLibraries+=gemutils

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
	@echo GIT_VERSION   $(GIT_VERSION)
	@echo GEMDEVELOPER  $(GEMDEVELOPER)
