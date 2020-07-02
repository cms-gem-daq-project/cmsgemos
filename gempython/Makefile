#
# Makefile for gempython package
#

Project      := cmsgemos
ShortProject := gem
Namespace    := gempython
Package      := gempython
LongPackage  := gempython
ShortPackage := gempython
PackageName  := $(Project)_$(ShortPackage)
PackageDir   := pkg/$(Namespace)
ScriptDir    := pkg/$(Namespace)/scripts

PythonModules = ["gempython.core", \
                 "gempython.tools", \
                 "gempython.utils" \
]
$(info PythonModules=${PythonModules})

GEMPYTHON_VER_MAJOR:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[1];}' | awk '{split($$0,b,":"); print b[2];}')
GEMPYTHON_VER_MINOR:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[2];}' | awk '{split($$0,b,":"); print b[2];}')
GEMPYTHON_VER_PATCH:=$(shell $(BUILD_HOME)/$(Project)/config/build/tag2rel.sh | awk '{split($$0,a," "); print a[3];}' | awk '{split($$0,b,":"); print b[2];}')

include $(BUILD_HOME)/$(Project)/config/mfDefsGEM.mk
include $(BUILD_HOME)/$(Project)/config/mfPythonDefsGEM.mk

UserCFlags += \
	-ftemplate-depth-128 \
	-O0 -rdynamic -finline-functions \
	-Wno-inline -DNDEBUG
UserCCFlags=$(UserCFlags)

# The leading undescore is meant to be there
DynamicLibrary = _cmsgemos_gempython
# Changing some defaults from $XDAQ_ROOT/config/mfDefs
LibraryPrefix =
## Not being respected, probably due to automatic calculation later in the process...
PackageLibDir = $(PackageDir)

Sources = src/common/cmsgemos_gempython.cc

IncludeDirs+=$(BUILD_HOME)/$(Project)/$(Package)/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemutils/include
IncludeDirs+=$(BUILD_HOME)/$(Project)/gemhardware/include
IncludeDirs+=$(uHALROOT)/include

DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraryDirs+=$(uHALROOT)/lib

LibraryDirs+=$(BUILD_HOME)/$(Project)/gemutils/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(BUILD_HOME)/$(Project)/gemhardware/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
LibraryDirs+=$(uHALROOT)/lib

DependentLibraries+=boost_python cactus_uhal_uhal gemhardware_devices gemutils
### UGGH, I WANT TO BE FREE, minimal gemutils works with this, and in this order
# remove dependencies on xdata and things will be better
DependentLibraries+=xdata mimetic xoap
#DependentLibraries+=xoap xdata mimetic xdaq

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/config/mfPythonRPM.mk

# need to ensure that the python only stuff is packaged into RPMs
#.PHONY: default
# default:
# 	$(info "Running default target")
# 	$(MakeDir) $(PackageDir)/core
# 	@cp -rf $(BUILD_HOME)/$(Project)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.so $(PackageDir)/core
# 	@cp -rf utils tools LICENSE MANIFEST.in requirements.txt $(PackageDir)
# 	$(MakeDir) $(PackageDir)/bin
# 	@cp -rf tests/*.py $(PackageDir)/bin
# 	@echo "__path__ = __import__('pkgutil').extend_path(__path__, __name__)" > pkg/$(Namespace)/__init__.py
# 	@cp -rf __init__.py $(PackageDir)

.PHONY: clean preprpm
_rpmprep: _all
	$(info "Running preprpm")
	$(MakeDir) $(PackageDir)/core
	@cp -rf $(BUILD_HOME)/$(Project)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.so $(PackageDir)/core
	@cp -rf gempython.md README.md
	@cp -rf utils tools $(PackageDir)
	$(MakeDir) $(ScriptDir)
	@cp -rf tests/*.py $(ScriptDir)
	@cp -rf tools/xdaq/set_scanParams_latency.py $(ScriptDir)
	@cp -rf tools/xdaq/setDAQParamsPostConfigure.py $(ScriptDir)
	@cp -rf tools/xdaq/trackingDataCalibrationScan.py $(ScriptDir)
	-cp -rf README.md LICENSE CHANGELOG.md MANIFEST.in requirements.txt $(PackageDir)
	-cp -rf README.md LICENSE CHANGELOG.md MANIFEST.in requirements.txt pkg
	@echo "__path__ = __import__('pkgutil').extend_path(__path__, __name__)" > pkg/$(Namespace)/__init__.py
#	@echo "include */*.so" > $(PackageDir)/MANIFEST.in

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
	@echo IncludeDirs    $(IncludeDirs)
	@echo LibraryDirs    $(LibraryDirs)
	@echo DependentLibraryDirs $(DependentLibraryDirs)
	@echo DependentLibraries $(DependentLibraries)
