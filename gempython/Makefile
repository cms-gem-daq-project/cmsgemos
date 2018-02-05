#
# Makefile for gempython package
#

Project=cmsgemos
ShortProject=gem
Package=gempython
LongPackage=gempython
ShortPackage=python
PackageName=python
PackageDir=pkg/gempython

PythonModules = ["gempython"]
$(info PythonModules=${PythonModules})

GEMPYTHON_VER_MAJOR=0
GEMPYTHON_VER_MINOR=3
GEMPYTHON_VER_PATCH=1

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem
include $(BUILD_HOME)/$(Project)/config/mfPythonDefs.gem


include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfPythonRPM_gem.rules

# The leading undescore is meant to be there
Library = cmsgemos_python
LibraryTarget = $(PackageDir)/_gempython.so

# BINDINGS_SOURCES = $(wildcard src/common/*.cpp)
# BINDINGS_OBJECT_FILES = $(patsubst src/common/%.cpp,obj/%.o,${BINDINGS_SOURCES})
# BINDINGS_LIB = ${PackageDir}/_core.so

# need to ensure that the python only stuff is packaged into RPMs
.PHONY: preprpm
preprpm:
	cp -rfp utils tools tests LICENSE.txt MANIFEST.in requirements.txt $(PackageDir)
	cp -rfp gempython.md $(PackageDir)/README.md

clean:
	rm -rf $(PackageDir)/utils
	rm -rf $(PackageDir)/tools
	rm -rf $(PackageDir)/tests
	rm  $(PackageDir)/LICENSE.txt
	rm  $(PackageDir)/MANIFEST.in
	rm  $(PackageDir)/requirements.txt
	rm  $(PackageDir)/README.md

rpm: preprpm _rpmall
# rpmall: ${BINDINGS_LIB}
# 	rm -rf /var/tmp/cactus*
# 	python setup.py bdist_rpm --release ${PACKAGE_RELEASE}.${XDAQ_OS}.python${PYTHON_VERSION} --binary-only --force-arch=`uname -m`

print-env:
	@echo BUILD_HOME     $(BUILD_HOME)
	@echo XDAQ_ROOT      $(XDAQ_ROOT)
	@echo XDAQ_OS        $(XDAQ_OS)
	@echo XDAQ_PLATFORM  $(XDAQ_PLATFORM)
	@echo LIBDIR         $(LIBDIR)
	@echo ROOTCFLAGS     $(ROOTCFLAGS)
	@echo ROOTLIBS       $(ROOTLIBS)
	@echo ROOTGLIBS      $(ROOTGLIBS)
	@echo GIT_VERSION    $(GIT_VERSION)
	@echo PYTHON_VERSION $(PYTHON_VERSION)
	@echo GEMDEVELOPER   $(GEMDEVELOPER)
	@echo CC             $(CC)
	@echo CPP            $(CPP)
	@echo CXX            $(CXX)
	@echo LD             $(LD)
	@echo AR             $(AR)
	@echo NM             $(NM)
	@echo RANLIB         $(RANLIB)
	@echo GCCVERSION     $(GCCVERSION)
	@echo CLANGVERSION   $(CLANGVERSION)
