#
# Makefile for gemutils package
#

#Project=gem
Project=cmsgemos
ShortProject=gem
Package=gemutils
LongPackage=gemutils
ShortPackage=utils
PackageName=utils

GEMUTILS_VER_MAJOR=0
GEMUTILS_VER_MINOR=2
GEMUTILS_VER_PATCH=0

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem

Sources =version.cc
Sources+=Lock.cc gemXMLparser.cc GEMRegisterUtils.cc
Sources+=soap/GEMSOAPToolBox.cc
Sources+=db/GEMDatabaseUtils.cc

DynamicLibrary=gemutils

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include

# MySQL Config
MySQLCFLAGS = $(shell mysql_config --cflags)
MySQLLIBS   = $(shell mysql_config --libs)
MySQLGLIBS  = $(shell mysql_config --glibs)

UserCFlags  +=$(MySQLCFLAGS)
UserCCFlags +=$(MySQLCFLAGS)

LibraryDirs+=$(MySQLLIBS)
LibraryDirs+=$(MySQLGLIBS)

UserDynamicLinkFlags+=$(MySQLLIBS)

include $(XDAQ_ROOT)/config/Makefile.rules
#include $(BUILD_HOME)/$(Project)/config/mfRPM_gem.rules
#include $(XDAQ_ROOT)/config/mfRPM.rules
include $(BUILD_HOME)/$(Project)/config/mfRPM_gem.rules

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
