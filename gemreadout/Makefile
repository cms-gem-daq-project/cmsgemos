#
# Makefile for gemreadout package
#

#Project=gem
Project=gemdaq-testing
ShortProject=gem
Package=gemreadout
LongPackage=gemreadout
ShortPackage=readout
PackageName=readout

GEMREADOUT_VER_MAJOR=0
GEMREADOUT_VER_MINOR=1
GEMREADOUT_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem

Sources =version.cc
#Sources+=GEMDataParker.cc
Sources+=GEMReadoutApplication.cc
#Sources+=GEMDataChecker.cc

DynamicLibrary=gemreadout

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gembase/include
#IncludeDirs+=$(BUILD_HOME)/$(Project)/gemhardware/include
#IncludeDirs+=$(uHALROOT)/include
IncludeDirs+=$(ROOTDIR)/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gembase/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
#DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
#DependentLibraryDirs+=$(uHALROOT)/lib

#LibraryDirs+=$(uHALROOT)/lib
LibraryDirs+=$(ROOTLIBS)
LibraryDirs+=$(ROOTGLIBS)

UserCFlags +=$(ROOTCFLAGS)
UserCCFlags+=$(ROOTCFLAGS)

#DependentLibraries+=cactus_uhal_uhal cactus_amc13_tools
DependentLibraries+=gemutils gembase
#DependentLibraries+=gemhardware

UserDynamicLinkFlags+=$(ROOTLIBS)

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
