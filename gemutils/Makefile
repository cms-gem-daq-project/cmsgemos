#
# Makefile for gemutils package
#

Project=cmsgemos
ShortProject=gem
Package=gemutils
LongPackage=gemutils
ShortPackage=utils
PackageName=utils

GEMUTILS_VER_MAJOR:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[1];}' | awk '{split($$0,b,":"); print b[2];}')
GEMUTILS_VER_MINOR:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[2];}' | awk '{split($$0,b,":"); print b[2];}')
GEMUTILS_VER_PATCH:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[3];}' | awk '{split($$0,b,":"); print b[2];}')

include $(BUILD_HOME)/$(Project)/config/mfDefsGEM.mk
include $(BUILD_HOME)/$(Project)/config/mfPythonDefsGEM.mk

Sources =version.cc
Sources+=Lock.cc GEMRegisterUtils.cc
Sources+=soap/GEMSOAPToolBox.cc
Sources+=db/GEMDatabaseUtils.cc
# Sources+=gemXMLparser.cc

DynamicLibrary=gemutils

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include

UserCFlags  +=$(MySQLCFLAGS)
UserCCFlags +=$(MySQLCFLAGS)

UserCFlags  +=$(PYTHONCFLAGS)
UserCCFlags +=$(PYTHONCFLAGS)

LibraryDirs+=$(MySQLLIBS)
LibraryDirs+=$(MySQLGLIBS)

LibraryDirs+=$(PYTHONLIBS)
LibraryDirs+=$(PYTHONGLIBS)

UserDynamicLinkFlags+=$(MySQLLIBS)

UserDynamicLinkFlags+=$(PYTHONLIBS)

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfRPMDefsGEM.mk


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
