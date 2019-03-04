## redefined from /opt/xdaq/config/mfRPM.rules

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
PWD          = $(shell pwd)
GITREV       = $(shell git rev-parse --short HEAD)
BUILD_DATE   = $(shell date -u +"%d%m%Y")

# copied from mfRPM_hcal.rules to use their directory manipulation
ifdef Package

ifndef PackageName
PackageName=$(shell awk -F'"' 'BEGIN{IGNORECASE=1} /package[ \t\r\f\v]*=/ {print $$2;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

# convert long package name to all upper case
LongPackageLoc = $(shell echo "$(LongPackage)" | tr '[:lower:]' '[:upper:]')
#LongPackageLoc=$(shell echo "$(LongPackage)" | awk '{print toupper($0)}') ## not working... why not?

ifndef PACKAGE_VER_MAJOR
PACKAGE_VER_MAJOR := $(shell awk 'BEGIN{IGNORECASE=1} /define $(LongPackage)_VERSION_MAJOR/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_VER_MINOR
PACKAGE_VER_MINOR := $(shell awk 'BEGIN{IGNORECASE=1} /define $(LongPackage)_VERSION_MINOR/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_VER_PATCH
PACKAGE_VER_PATCH := $(shell awk 'BEGIN{IGNORECASE=1} /define $(LongPackage)_VERSION_PATCH/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_REQUIRED_PACKAGE_LIST
PACKAGE_REQUIRED_PACKAGE_LIST :=$(shell awk 'BEGIN{IGNORECASE=1} /define $(LongPackage)_REQUIRED_PACKAGE_LIST/ {print $$3;}' \
	$(PackagePath)/include/$(ShortProject)/$(ShortPackage)/version.h)
endif

ifndef PACKAGE_RELEASE
include  $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfRPM.release
endif

ifndef BUILD_COMPILER
#BUILD_COMPILER :=$(shell $(CC) --version | grep GCC |sed -e 's/-.*//g')$(shell $(CC) -dumpversion | sed -e 's/\./_/g')
BUILD_COMPILER :=$(CC)$(shell $(CC) -dumpversion | sed -e 's/\./_/g')
endif

ifndef BUILD_DISTRIBUTION
BUILD_DISTRIBUTION := $(shell $(XDAQ_ROOT)/$(BUILD_SUPPORT)/checkos.sh)
endif

ifndef PACKAGE_FULL_RELEASE
# would like to use the correct %?{dist}
PACKAGE_FULL_RELEASE = $(BUILD_VERSION).$(GITREV)git.$(PACKAGE_RELEASE).$(BUILD_DISTRIBUTION).$(BUILD_COMPILER)
endif

REQUIRES_LIST=0
ifneq ($(PACKAGE_REQUIRED_PACKAGE_LIST),)
REQUIRES_LIST=1
endif

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
_rpmall: spec_update makerpm
endif


# if for package makefile only if endif
#else
#_rpmall:
#	@echo "*** "$(Package) I am in the else
endif

# 	rpmbuild  -bb -bl --target $(XDAQ_PLATFORM) --define  "_topdir $(PackagePath)/rpm/RPMBUILD" \
	$(PackagePath)/rpm/$(LongPackage).spec


.PHONY: makerpm
makerpm:
	$(MakeDir) $(PackagePath)/rpm/RPMBUILD/{RPMS/$(XDAQ_PLATFORM),SPECS,BUILD,SOURCES,SRPMS}
	@echo BUILD_VERSION $(BUILD_VERSION)
	@echo PACKAGE_FULL_VERSION $(PACKAGE_FULL_VERSION)
	@echo PACKAGE_RELEASE $(PACKAGE_RELEASE)
	@echo PACKAGE_FULL_RELEASE $(PACKAGE_FULL_RELEASE)
	@echo BUILD_DISTRIBUTION $(BUILD_DISTRIBUTION)
	@echo BUILD_COMPILER $(BUILD_COMPILER)
	@echo PackagePath $(PackagePath)
	@tar -P -X $(XDAQ_ROOT)/$(BUILD_SUPPORT)/src.exclude --exclude="*.tbz2" -jcf \
		$(PackagePath)/rpm/RPMBUILD/SOURCES/$(Project)-$(PackageName)-$(PACKAGE_FULL_VERSION)-$(PACKAGE_FULL_RELEASE).tbz2 \
		$(PackagePath)
	@rpmbuild  --quiet -ba -bl --define "_requires $(REQUIRES_LIST)" \
		--define  "_topdir $(PackagePath)/rpm/RPMBUILD" \
		$(PackagePath)/rpm/$(LongPackage).spec
	@find  $(PackagePath)/rpm/RPMBUILD -name "*.rpm" -exec mv {} $(PackagePath)/rpm \;

.PHONY: spec_update
spec_update:
	$(info "Executing GEM specific spec_update")
	@mkdir -p $(PackagePath)/rpm
	if [ -e $(PackagePath)/spec.template ]; then \
		echo $(PackagePath) found spec.template; \
		cp $(PackagePath)/spec.template $(PackagePath)/rpm/$(LongPackage).spec; \
	elif [ -e $(ProjectPath)/config/build/cmsgemos.spec.template ]; then \
		echo $(ProjectPath)/config found cmsgemos.spec.template; \
		cp $(ProjectPath)/config/build/cmsgemos.spec.template $(PackagePath)/rpm/$(LongPackage).spec; \
	else \
		echo $(XDAQ_ROOT)/$(BUILD_SUPPORT) found spec.template; \
		cp $(XDAQ_ROOT)/$(BUILD_SUPPORT)/spec.template $(PackagePath)/rpm/$(LongPackage).spec; \
	fi

	sed -i 's#__gitrev__#$(GITREV)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__builddate__#$(BUILD_DATE)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__release__#$(PACKAGE_FULL_RELEASE)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__version__#$(PACKAGE_FULL_VERSION)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__prefix__#$(INSTALL_PATH)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__xdaqprefix__#$(XDAQ_ROOT)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__package__#$(Package)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__packagedir__#$(PackagePath)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__packagename__#$(PackageName)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__longpackage__#$(LongPackage)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__longpackagedir__#$(PackagePath)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__longpackagename__#$(LongPackage)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__os__#$(XDAQ_OS)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__platform__#$(XDAQ_PLATFORM)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__project__#$(Project)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__author__#$(Authors)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__summary__#$(Summary)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__description__#$(Description)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__url__#$(Link)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__buildarch__#$(XDAQ_PLATFORM)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__includedirs__#$(IncludeDirs)#' $(PackagePath)/rpm/$(LongPackage).spec
#	sed -i 's#__datadir__#$(DATA_DIR)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__username__#$(USER_NAME)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__groupname__#$(GROUP_NAME)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__uid__#$(UID)#' $(PackagePath)/rpm/$(LongPackage).spec
	sed -i 's#__gid__#$(GID)#' $(PackagePath)/rpm/$(LongPackage).spec

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
