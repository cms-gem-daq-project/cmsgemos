#
# Makefile for gempython package
#

Project=cmsgemos
ShortProject=gem
Namespace=gempython
Package=gempython
LongPackage=gempython
ShortPackage=gempython
PackageName=gempython-cmsgemos
PackageDir=pkg/gempython

PythonModules = ["gempython.core", \
                 "gempython.tools", \
                 "gempython.utils" \
]
$(info PythonModules=${PythonModules})

GEMPYTHON_VER_MAJOR=0
GEMPYTHON_VER_MINOR=3
GEMPYTHON_VER_PATCH=1

include $(BUILD_HOME)/$(Project)/config/mfDefs.gem
include $(BUILD_HOME)/$(Project)/config/mfPythonDefs.gem

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
#include $(BUILD_HOME)/$(Project)/config/mfPythonRPM_gem.rules

# need to ensure that the python only stuff is packaged into RPMs
#.PHONY: default
# default:
# 	$(info "Running default target")
# 	$(MakeDir) $(PackageDir)/core
# 	@cp -rfp $(BUILD_HOME)/$(Project)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.so $(PackageDir)/core
# 	@cp -rfp utils tools LICENSE.txt MANIFEST.in requirements.txt $(PackageDir)
# 	$(MakeDir) $(PackageDir)/bin
# 	@cp -rfp tests/*.py $(PackageDir)/bin
# 	@echo "__path__ = __import__('pkgutil').extend_path(__path__, __name__)" > pkg/$(Namespace)/__init__.py
# 	@cp -rfp __init__.py $(PackageDir)

.PHONY: clean preprpm
#_rpmprep: preprpm
# prep_rpm: default
_rpmprep: _all
	$(info "Running preprpm")
	$(MakeDir) $(PackageDir)/core
	@cp -rfp $(BUILD_HOME)/$(Project)/$(Package)/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)/*.so $(PackageDir)/core
	@cp -rfp utils tools LICENSE.txt MANIFEST.in requirements.txt $(PackageDir)
	$(MakeDir) $(PackageDir)/bin
	@cp -rfp tests/*.py $(PackageDir)/bin
	@cp -rfp gempython.md $(PackageDir)/README.md
	@echo "__path__ = __import__('pkgutil').extend_path(__path__, __name__)" > pkg/$(Namespace)/__init__.py
	@echo "include */*.so" > $(PackageDir)/MANIFEST.in

clean:
	$(info "Running clean target")
	@rm -rf $(PackageDir)/core
	@rm -rf $(PackageDir)/utils
	@rm -rf $(PackageDir)/tools
	@rm -rf $(PackageDir)/bin
	@rm -rf $(PackageDir)/tests
	@rm -rf pkg/core
	@rm -rf pkg/utils
	@rm -rf pkg/tools
	@rm -rf pkg/bin
	@rm -rf pkg/tests
	@rm -f $(PackageDir)/LICENSE.txt
	@rm -f $(PackageDir)/MANIFEST.in
	@rm -f $(PackageDir)/requirements.txt
	@rm -f $(PackageDir)/_cmsgemos_gempython.so
	@rm -f $(PackageDir)/README.md

rpm: _rpmbuild
	$(info "Running overridden rpm target")
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
	@echo IncludeDirs    $(IncludeDirs)
	@echo LibraryDirs    $(LibraryDirs)
	@echo DependentLibraryDirs $(DependentLibraryDirs)
	@echo DependentLibraries $(DependentLibraries)
