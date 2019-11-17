#
# Makefile for gempython package
#

BUILD_HOME?= $(shell dirname `cd ../; pwd`)

ifndef BUILD_SUPPORT
BUILD_SUPPORT=build
endif

ifndef PROJECT_NAME
PROJECT_NAME=cmsgemos
endif

Project=$(PROJECT_NAME)
ShortProject := gem
Namespace    := gempython
Package      := gempython
LongPackage  := gempython
ShortPackage := gempython
PackageName  := python
PackageDir   := pkg/$(Namespace)
ScriptDir    := pkg/$(Namespace)/scripts

ProjectBase = $(BUILD_HOME)
CfgDir   = $(ProjectBase)/config

PythonModules = ["gempython.core", \
                 "gempython.tools", \
                 "gempython.utils" \
]
$(info PythonModules=${PythonModules})

GEMPYTHON_VER_MAJOR=1
GEMPYTHON_VER_MINOR=0
GEMPYTHON_VER_PATCH=0

include $(CfgDir)/mfDefsGEM.mk
include $(CfgDir)/mfPythonDefsGEM.mk

UserCFlags+= \
	-ftemplate-depth-128 \
	-O0 -rdynamic -finline-functions \
	-Wno-inline -DNDEBUG
UserCFlags+=$(PYTHONCFLAGS)

# The leading undescore is meant to be there
DynamicLibrary = _cmsgemos_gempython
# Changing some defaults from $XDAQ_ROOT/build/mfDefs
LibraryPrefix =
## Not being respected, probably due to automatic calculation later in the process...
PackageLibDir = $(PackageDir)

Sources = $(wildcard src/common/*.cc) $(wildcard src/common/*.cpp) $(wildcard src/common/**/*.cc) $(wildcard src/common/**/*.cpp)

IncludeDirs+=$(BUILD_HOME)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/gemhardware/devices/include
IncludeDirs+=$(BUILD_HOME)/gemhardware/utils/include
IncludeDirs+=$(uHALROOT)/include
IncludeDirs+=$(XHAL_ROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemhardware/devices/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/gemhardware/utils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(uHALROOT)/lib
DependentLibraryDirs+=$(XHAL_ROOT)/lib

LibraryDirs+=$(BUILD_HOME)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/gemhardware/devices/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/gemhardware/utils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(uHALROOT)/lib
LibraryDirs+=$(XHAL_ROOT)/lib

DependentLibraries =gemhwdevices gemhwutils
DependentLibraries+=boost_python

# UserDynamicLinkFlags+=

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/Makefile.rules
include $(CfgDir)/mfPythonRPM.mk

# need to ensure that the python only stuff is packaged into RPMs
#.PHONY: default
# default:
# 	$(info "Running default target")
# 	$(MakeDir) $(PackageDir)/core
# 	@cp -rf $(BUILD_HOME)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.so $(PackageDir)/core
# 	@cp -rf utils tools LICENSE MANIFEST.in requirements.txt $(PackageDir)
# 	$(MakeDir) $(PackageDir)/bin
# 	@cp -rf tests/*.py $(PackageDir)/bin
# 	@echo "__path__ = __import__('pkgutil').extend_path(__path__, __name__)" > pkg/$(Namespace)/__init__.py
# 	@cp -rf __init__.py $(PackageDir)

$(PackageSetupFile): pkg/setup.py pkg/setup.cfg
$(PackagePrepFile): $(PythonSources) Makefile | default
#	@cp -rf $(CfgDir)/scriptlets/installrpm.sh pkg/
#	@perl -pi -e "s|__PYTHON_SCRIPT_PATH__|$(INSTALL_PATH)/bin|g" pkg/installrpm.sh
#	@perl -pi -e "s|__PYTHON_NAMESPACE__|$(Namespace)|g" pkg/installrpm.sh
	$(MakeDir) $(PackageDir)/core
#	@touch $(PackageDir)/core/__init__.py
	@cp -rf $(BUILD_HOME)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.so $(PackageDir)/core
	@cp -rf gempython.md README.md
	@cp -rf utils tools $(PackageDir)
	$(MakeDir) $(ScriptDir)
	@cp -rf tests/*.py $(ScriptDir)
	-cp -rf README.md LICENSE CHANGELOG.md MANIFEST.in requirements.txt $(PackageDir)
	-cp -rf README.md LICENSE CHANGELOG.md MANIFEST.in requirements.txt pkg
	@cp -rf __init__.py $(PackageDir)/core
	@echo "__path__ = __import__('pkgutil').extend_path(__path__, __name__)" > pkg/$(Namespace)/__init__.py
#	@echo "include */*.so" > $(PackageDir)/MANIFEST.in
	touch $@

# .PHONY: clean preprpm
# _rpmprep: _all
# 	$(info "Running preprpm")
# 	$(MakeDir) $(PackageDir)/core
# #	@touch $(PackageDir)/core/__init__.py
# 	@cp -rf $(ProjectBase)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.so $(PackageDir)/core
# 	@cp -rf gempython.md README.md
# 	@cp -rf utils tools $(PackageDir)
# 	$(MakeDir) $(ScriptDir)
# 	@cp -rf tests/*.py $(ScriptDir)
# 	-cp -rf README.md LICENSE CHANGELOG.md MANIFEST.in requirements.txt $(PackageDir)
# 	-cp -rf README.md LICENSE CHANGELOG.md MANIFEST.in requirements.txt pkg
# 	@cp -rf __init__.py $(PackageDir)/core
# 	@echo "__path__ = __import__('pkgutil').extend_path(__path__, __name__)" > pkg/$(Namespace)/__init__.py
# #	@echo "include */*.so" > $(PackageDir)/MANIFEST.in

clean:
	$(info "Running clean target")
	-rm -rf $(PackageDir)/core
	-rm -rf $(PackageDir)/utils
	-rm -rf $(PackageDir)/tools
	-rm -rf $(ScriptDir)
	-rm -f  $(PackageDir)/README.md
	-rm -f  $(PackageDir)/LICENSE
	-rm -f  $(PackageDir)/MANIFEST.in
	-rm -f  $(PackageDir)/CHANGELOG.md
	-rm -f  $(PackageDir)/requirements.txt
	-rm -f  $(PackageDir)/_cmsgemos_gempython.so
	-rm -f  pkg/README.md
	-rm -f  pkg/LICENSE
	-rm -f  pkg/MANIFEST.in
	-rm -f  pkg/CHANGELOG.md
	-rm -f  pkg/requirements.txt

# rpm: _rpmbuild
# 	$(info "Running overridden rpm target")
# rpmall: ${BINDINGS_LIB}
# 	rm -rf /var/tmp/cactus*
# 	python setup.py bdist_rpm --release ${PACKAGE_RELEASE}.${XDAQ_OS}.python${PYTHON_VERSION} --binary-only --force-arch=`uname -m`

print-env:
	@echo BUILD_HOME     $(BUILD_HOME)
	@echo BUILD_VERSION  $(BUILD_VERSION)
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
	@echo MySQLLIBS      $(MySQLLIBS)
	@echo IncludeDirs    $(IncludeDirs)
	@echo LibraryDirs    $(LibraryDirs)
	@echo DependentLibraryDirs $(DependentLibraryDirs)
	@echo DependentLibraries $(DependentLibraries)
