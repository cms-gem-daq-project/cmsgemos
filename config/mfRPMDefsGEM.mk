## redefined from /opt/xdaq/config/mfRPM.rules

CMSGEMOS_PLATFORM := $(shell python -c "import platform; print(platform.platform())")
CMSGEMOS_ARCH     := $(shell uname -m)
CMSGEMOS_OS?=$(XDAQ_OS)

rpm: $(PackageListLoop)

installrpm: $(PackageListLoop)

cleanrpm: $(PackageListLoop)

changelog: $(PackageListLoop)

INSTALL_PATH = /opt/cmsgemos
XDAQ_PATH = /opt/xdaq
GROUP_NAME = gempro
USER_NAME = $(GROUP_NAME)
GID = 1050
UID = $(GID)
ProjectPath  = $(BUILD_HOME)
PackagePath  = $(ProjectPath)/$(LongPackage)
RPM_DIR?=$(PackagePath)/rpm
RPMBUILD_DIR:=$(RPM_DIR)/RPMBUILD
PWD          = $(shell pwd)
GITREV       = $(shell git rev-parse --short HEAD)
BUILD_DATE   = $(shell date -u +"%d%m%Y")

## when building with different compilers/python versions, encode this into the platfom?
## or for python, provide pythonX-packagename package?
## when does compilation with python includes come into play?
GEMOS_PLATFORM=${XDAQ_PLATFORM}

# copied from mfRPM_hcal.rules to use their directory manipulation
ifdef Package

ifndef PackageName
PackageName=$(shell awk -F'"' 'BEGIN{IGNORECASE=1} /package[ \t\r\f\v]*=/ {print $$2;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

# convert long package name to all upper case
LongPackageLoc = $(shell echo "$(ShortProject)$(PackageName)" | tr '[:lower:]' '[:upper:]')
#LongPackageLoc=$(shell echo "$(ShortProject)$(PackageName)" | awk '{print toupper($0)}') ## not working... why not?

ifndef PACKAGE_VER_MAJOR
PACKAGE_VER_MAJOR := $(shell awk 'BEGIN{IGNORECASE=1} /define $(ShortProject)$(PackageName)_VERSION_MAJOR/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_VER_MINOR
PACKAGE_VER_MINOR := $(shell awk 'BEGIN{IGNORECASE=1} /define $(ShortProject)$(PackageName)_VERSION_MINOR/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_VER_PATCH
PACKAGE_VER_PATCH := $(shell awk 'BEGIN{IGNORECASE=1} /define $(ShortProject)$(PackageName)_VERSION_PATCH/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_REQUIRED_PACKAGE_LIST
PACKAGE_REQUIRED_PACKAGE_LIST :=$(shell awk 'BEGIN{IGNORECASE=1} /define $(ShortProject)$(PackageName)_REQUIRED_PACKAGE_LIST/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_RELEASE
include  $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfRPM.release
endif

ifndef BUILD_COMPILER
#BUILD_COMPILER :=$(shell $(CC) --version | grep GCC |sed -e 's/-.*//g')$(shell $(CC) -dumpversion | sed -e 's/\./_/g')
BUILD_COMPILER :=$(CC)$(shell $(CC) -dumpfullversion -dumpversion | sed -e 's/\./_/g')
endif

ifndef BUILD_DISTRIBUTION
BUILD_DISTRIBUTION := $(shell $(XDAQ_ROOT)/$(BUILD_SUPPORT)/checkos.sh)
endif

ifndef PACKAGE_NOARCH_RELEASE
# would like to use the correct %?{dist}
PACKAGE_NOARCH_RELEASE = $(BUILD_VERSION).$(GITREV)git
endif

ifndef PACKAGE_FULL_RELEASE
# would like to use the correct %?{dist}
PACKAGE_FULL_RELEASE = $(PACKAGE_NOARCH_RELEASE).$(PACKAGE_RELEASE).$(BUILD_DISTRIBUTION).$(BUILD_COMPILER)
endif

REQUIRES_LIST=0
ifneq ($(PACKAGE_REQUIRED_PACKAGE_LIST),)
REQUIRES_LIST=1
endif

PACKAGE_ABI_VERSION?=$(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR)
#
# Extract summary, description and authors
#
ifndef Description
Description = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /description[ \t\r\f\v]*=/ {print $$2;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef Summary
Summary = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /summary[ \t\r\f\v]*=/ {print $$2;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

#ifndef Authors
# Authors defined in mfDefs.linux as $(shell id -un)
Authors = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /authors[ \t\r\f\v]*=/ {print $$2;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
#endif

ifndef Link
Link = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /link[ \t\r\f\v]*=/ {print $$2;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif


.PHONY: _rpmall

ifeq ($(VER_EXISTS),no)
_rpmall: fail
fail:
	$(error Unable to extract a valid version X.Y.Z from Makefile or version.h in package '$(Package)')
else
_rpmall: makerpm
endif


endif

# 	rpmbuild  -bb -bl --target $(XDAQ_PLATFORM) --define  "_topdir $(PackagePath)/rpm/RPMBUILD" \
	$(PackagePath)/rpm/$(ShortProject)$(PackageName).spec


.PHONY: makerpm
makerpm: _rpmbuild _rpmharvest

TargetSRPMName=$(RPM_DIR)/$(PackageName).src.rpm
TargetRPMName=$(RPM_DIR)/$(PackageName).$(CMSGEMOS_ARCH).rpm
PackageSourceTarball=$(RPM_DIR)/$(Project)-$(PackageName)-$(PACKAGE_FULL_VERSION)-$(PACKAGE_FULL_RELEASE).tbz2
#PackageSourceTarball=$(RPM_DIR)/$(Project)-$(PackageName)-$(PACKAGE_FULL_VERSION)-$(PACKAGE_NOARCH_RELEASE).tbz2
PackageSpecFile=$(RPM_DIR)/$(PackageName).spec

PackagingTargets=$(TargetSRPMName)
PackagingTargets+=$(TargetRPMName)

.PHONY: rpm rpmprep specificspecupdate
## @rpm performs all steps necessary to generate RPM packages
rpm: _rpmbuild _rpmharvest

## @rpm Perform any specific setup before packaging, is an implicit dependency of `rpm`
rpmprep: | $(PackageSourceTarball)
	$(MakeDir) $(RPMBUILD_DIR)/SOURCES
	cp -rfp $(PackageSourceTarball) $(RPMBUILD_DIR)/SOURCES

.PHONY: _rpmbuild _rpmharvest
_rpmbuild: $(PackageSourceTarball) $(PackagingTargets)

_rpmharvest: $(PackagingTargets)
	$(ProjectPath)/.ci/generate_repo.sh $(CMSGEMOS_OS) $(CMSGEMOS_ARCH) $(RPM_DIR) $(RPMBUILD_DIR) $(Project)

$(PackageSourceTarball):
	$(MakeDir) $(RPM_DIR)
	@tar -P -X $(XDAQ_ROOT)/config/src.exclude \
	    --exclude="*.tbz2" -jcf $@ \
	    $(PackagePath)

$(TargetSRPMName): $(PackageSpecFile) | specificspecupdate rpmprep
	rpmbuild --quiet -bs -bl \
	    --buildroot=$(RPMBUILD_DIR)/BUILDROOT \
	    --define "_requires $(REQUIRES_LIST)" \
	    --define "_release $(PACKAGE_NOARCH_RELEASE)" \
	    --define "_build_requires $(BUILD_REQUIRES_LIST)" \
	    --define  "_topdir $(RPMBUILD_DIR)" \
	    $(RPM_DIR)/$(LongPackage).spec \
	    $(RPM_OPTIONS) --target "$(CMSGEMOS_ARCH)";
	touch $@

$(TargetRPMName): $(PackageSpecFile) | specificspecupdate rpmprep
	rpmbuild --quiet -bb -bl \
	    --buildroot=$(RPMBUILD_DIR)/BUILDROOT \
	    --define "_requires $(REQUIRES_LIST)" \
	    --define "_release $(PACKAGE_FULL_RELEASE)" \
	    --define "_build_requires $(BUILD_REQUIRES_LIST)" \
	    --define  "_topdir $(RPMBUILD_DIR)" \
	    $(RPM_DIR)/$(LongPackage).spec \
	    $(RPM_OPTIONS) --target "$(CMSGEMOS_ARCH)";
	touch $@

$(PackageSpecFile): $(ProjectPath)/config/build/cmsgemos.spec.template $(PackageSourceTarball)
	$(info "Executing GEM specific spec_update")
	@mkdir -p $(PackagePath)/rpm
	if [ -e $(PackagePath)/spec.template ]; then \
		echo $(PackagePath) found spec.template; \
		cp $(PackagePath)/spec.template $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec; \
	elif [ -e $(ProjectPath)/config/build/cmsgemos.spec.template ]; then \
		echo $(ProjectPath)/config found cmsgemos.spec.template; \
		cp $(ProjectPath)/config/build/cmsgemos.spec.template $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec; \
	else \
		echo $(XDAQ_ROOT)/$(BUILD_SUPPORT) found spec.template; \
		cp $(XDAQ_ROOT)/$(BUILD_SUPPORT)/spec.template $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec; \
	fi

	sed -i 's#__gitrev__#$(GITREV)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__builddate__#$(BUILD_DATE)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__release__#$(PACKAGE_FULL_RELEASE)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__version__#$(PACKAGE_FULL_VERSION)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__prefix__#$(INSTALL_PATH)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__xdaqprefix__#$(XDAQ_ROOT)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__package__#$(Package)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__packagedir__#$(PackagePath)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__packagename__#$(PackageName)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__longpackage__#$(LongPackage)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__longpackagedir__#$(PackagePath)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__longpackagename__#$(ShortProject)$(PackageName)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__os__#$(XDAQ_OS)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__platform__#$(XDAQ_PLATFORM)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__project__#$(Project)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__author__#$(Authors)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__summary__#$(Summary)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__description__#$(Description)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__url__#$(Link)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__buildarch__#$(XDAQ_PLATFORM)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__includedirs__#$(IncludeDirs)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
#	sed -i 's#__datadir__#$(DATA_DIR)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__username__#$(USER_NAME)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__groupname__#$(GROUP_NAME)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__uid__#$(UID)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec
	sed -i 's#__gid__#$(GID)#' $(PackagePath)/rpm/$(ShortProject)$(PackageName).spec

.PHONY: _cleanrpmall
_cleanrpmall:
	-rm -rf $(PackagePath)/rpm


.PHONY: _installrpmall
_installrpmall:
	mkdir -p $(INSTALL_PREFIX)/rpm
	cp $(PackagePath)/rpm/*.rpm $(INSTALL_PREFIX)/rpm


.PHONY: _changelogall
_changelogall:
	cd $(PackagePath);\
	git log --full-diff --decorate=full < ChangeLog
