#
# Makefile for gemutils package
#

#Project=gem
Project=gemdaq-testing
ShortProject=gem
Package=gemutils
LongPackage=gemutils
ShortPackage=utils
PackageName=utils

GEMUTILS_VER_MAJOR=0
GEMUTILS_VER_MINOR=1
GEMUTILS_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem

Sources =version.cc
Sources+=Lock.cc gemXMLparser.cc soap/GEMSOAPToolBox.cc GEMRegisterUtils.cc

DynamicLibrary=gemutils

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include

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
