## redefined from /opt/xdaq/config/mfRPM.rules
#  and
## amc13/config/mfPythonRPMRules.mk

rpm: $(PackageListLoop)
	$(info "PackageListLoop: $(PackageListLoop)")
	cat $(PackageListLoop) | tee mymakelog.log
	$(info "Running rpm target")

installrpm: $(PackageListLoop)
	$(info "PackageListLoop: $(PackageListLoop)")
	$(info "Running installrpm target")

cleanrpm: $(PackageListLoop)
	$(info "PackageListLoop: $(PackageListLoop)")
	$(info "Running cleanrpm target")

changelog: $(PackageListLoop)
	$(info "PackageListLoop: $(PackageListLoop)")
	$(info "Running changelog target")

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

ifndef PythonModules
	$(error Python module names missing "PythonModules")
endif

# copied from mfRPM_hcal.rules to use their directory manipulation
ifdef Package

ifndef PackageName
PackageName=$(shell awk -F'"' 'BEGIN{IGNORECASE=1} /package[ \t\r\f\v]*=/ {print $$2;}' $(PackagePath)/Makefile)
endif

# convert long package name to all upper case
LongPackageLoc:=$(shell echo "$(LongPackage)" | tr '[:lower:]' '[:upper:]')
#LongPackageLoc:=$(shell echo "$(LongPackage)" | awk '{print toupper($0)}') ## not working... why not?

# ifndef PACKAGE_VER_MAJOR
# PACKAGE_VER_MAJOR := $($(LongPackageLoc)_VER_MAJOR)
# else
# $(info mfPythonRPM_gem.rules PACKAGE_VER_MAJOR is already set to $(PACKAGE_VER_MAJOR))
# endif
# ifndef PACKAGE_VER_MINOR
# PACKAGE_VER_MINOR := $($(LongPackageLoc)_VER_MINOR)
# else
# $(info mfPythonRPM_gem.rules PACKAGE_VER_MINOR is already set to $(PACKAGE_VER_MINOR))
# endif
# ifndef PACKAGE_VER_MINOR
# PACKAGE_VER_PATCH := $($(LongPackageLoc)_VER_PATCH)
# else
# $(info mfPythonRPM_gem.rules PACKAGE_VER_PATCH is already set to $(PACKAGE_VER_PATCH))
# endif

ifndef PACKAGE_VER_MAJOR
PACKAGE_VER_MAJOR:=$($(LongPackageLoc)_VER_MAJOR)
endif

ifndef PACKAGE_VER_MINOR
PACKAGE_VER_MINOR:=$($(LongPackageLoc)_VER_MINOR)
endif

ifndef PACKAGE_VER_PATCH
PACKAGE_VER_PATCH:=$($(LongPackageLoc)_VER_PATCH)
endif

ifndef PACKAGE_RELEASE
include  $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfRPM.release
endif

ifndef BUILD_VERSION
BUILD_VERSION=1
endif

ifndef BUILD_COMPILER
#BUILD_COMPILER :=$(shell $(CC) --version | grep GCC |sed -e 's/-.*//g')$(shell $(CC) -dumpversion | sed -e 's/\./_/g')
BUILD_COMPILER :=$(CC)$(shell $(CC) -dumpversion | sed -e 's/\./_/g')
endif

ifndef BUILD_DISTRIBUTION
BUILD_DISTRIBUTION := $(shell $(XDAQ_ROOT)/$(BUILD_SUPPORT)/checkos.sh)
endif

VER_EXISTS=no
ifneq ($(PACKAGE_VER_MAJOR),)
ifneq ($(PACKAGE_VER_MINOR),)
ifneq ($(PACKAGE_VER_PATCH),)
VER_EXISTS=yes
endif
endif
endif

ifndef PACKAGE_FULL_VERSION
PACKAGE_FULL_VERSION = $(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR).$(PACKAGE_VER_PATCH)
endif

ifndef PACKAGE_FULL_RELEASE
PACKAGE_FULL_RELEASE = $(BUILD_VERSION).git$(GITREV).$(PACKAGE_RELEASE).$(BUILD_DISTRIBUTION).$(BUILD_COMPILER)
endif

REQUIRES_LIST=0
ifneq ($(PACKAGE_REQUIRED_PACKAGE_LIST),)
REQUIRES_LIST=1
endif


#
# Extract summary, description and authors
#
ifndef Description
Description = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /description[ \t\r\f\v]*=/ {print $$2;}' $(PackagePath)/Makefile)
endif

ifndef Summary
Summary = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /summary[ \t\r\f\v]*=/ {print $$2;}' $(PackagePath)/Makefile)
endif

#ifndef Authors
# Authors defined in mfDefs.linux as $(shell id -un)
Authors = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /authors[ \t\r\f\v]*=/ {print $$2;}' $(PackagePath)/Makefile)
#endif

ifndef Link
Link = $(shell awk -F'"' 'BEGIN{IGNORECASE=1} /link[ \t\r\f\v]*=/ {print $$2;}' $(PackagePath)/Makefile)
endif


.PHONY: _rpmall

ifeq ($(VER_EXISTS),no)
_rpmall: fail
fail:
	$(error Error (rpmall) could not extract version information from Makefile in package '$(Package)')
else
_rpmall: makerpm
	$(info "Running _rpmall target")
endif


# if for package makefile only if endif
#else
#_rpmall:
#	$(info "*** "$(Package) I am in the else)
endif

# 	rpmbuild  -bb -bl --target $(XDAQ_PLATFORM) --define  "_topdir $(PackagePath)/rpm/RPMBUILD"  $(PackagePath)/rpm/$(LongPackage).spec


.PHONY: makedevrpm
makedevrpm: setup_update
	# use python pbr to set up and do the building of the python packages
	# Change directory into pkg and copy everything into rpm/pkg/
	cd pkg/$(Package) && \
	find . -iname 'setup.*' -prune -o -name "*" -exec install -D \{\} $(PackagePath)/rpm/\{\} \;
	# # Add a manifest file
	# echo "include */*.so" > $(PackagePath)/rpm/MANIFEST.in
	# Change into rpm/pkg to finally run the customized setup.py
	# for building a nightly, egg_info --tag-date --tag-build=DEV
	cd $(PackagePath)/rpm && python setup.py \
	egg_info --tag-date --tag-build=DEV \
	bdist_rpm \
	--release ${PACKAGE_RELEASE}.${XDAQ_PLATFORM}.python${PYTHON_VERSION} \
	--binary-only --force-arch=`uname -m`
	# Harvest the crop
	find rpm -name "*.rpm"    -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tar.gz" -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tgz"    -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tbz2"   -exec cp -a \{\} rpm/ \;

.PHONY: makerpm
makerpm: build_rpm
	$(info "Running makerpm target")

.PHONY: prep_rpm
prep_rpm: setup_update
	$(info "Running prep_rpm target")
	# use python pbr to set up and do the building of the python packages
	# Copy files into pkg
	cp README.md pkg; \
	find lib -iname '_cmsgemos_gempython.so' -exec cp \{\} pkg/gempython \;
	# cp -rfp tests tools utils pkg/gempython; \;
	find tests -name "*.py" -exec install -D \{\} $(PackagePath)/pkg/\{\} \;
	# find tools -name "*.py" -exec install -D \{\} $(PackagePath)/pkg/\{\} \;
	# find utils -name "*.py" -exec install -D \{\} $(PackagePath)/pkg/\{\} \;
	# Change directory into pkg and copy everything into rpm/pkg/
	# cd pkg/$(Package) && 
	cd pkg && \
	find . -iname 'setup.*' -prune -o -name "*" -exec install -D \{\} $(PackagePath)/rpm/\{\} \;
	# # Add a manifest file
	echo "include */*.so" > $(PackagePath)/rpm/MANIFEST.in

.PHONY: build_rpm
build_rpm: prep_rpm
	$(info "Running build_rpm target")
	# Change into rpm/pkg to finally run the customized setup.py
	# egg_info --tag-date --tag-build=DEV \#
	cd $(PackagePath)/rpm && python setup.py \
	egg_info --tag-date \
	bdist_rpm \
	--release ${PACKAGE_RELEASE}.${XDAQ_PLATFORM}.python${PYTHON_VERSION} \
	--binary-only --force-arch=`uname -m`
	# Harvest the crop
	find rpm -name "*.rpm"    -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tar.gz" -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tgz"    -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tbz2"   -exec cp -a \{\} rpm/ \;
	# for building a nightly, egg_info --tag-date --tag-build=DEV

.PHONY: setup_update
setup_update:
	$(info "Running setup_update target")
	$(info "Executing GEM specific setup_update")
	@mkdir -p $(PackagePath)/rpm
	if [ -e $(PackagePath)/pkg/setup.py ]; then \
		echo Found $(PackagePath)/pkg/setup.py; \
		cp $(PackagePath)/pkg/setup.py $(PackagePath)/rpm/setup.py; \
	elif [ -e $(PackagePath)/setup.py ]; then \
		echo $(PackagePath) found setup.py; \
		cp $(PackagePath)/setup.py $(PackagePath)/rpm/setup.py; \
	elif [ -e $(ProjectPath)/config/build/cmsgemos.setupTemplate.py ]; then \
		echo $(ProjectPath)/config found cmsgemos.setupTemplate.py; \
		cp $(ProjectPath)/config/build/cmsgemos.setupTemplate.py $(PackagePath)/rpm/setup.py; \
	else \
		echo "Error (setup_update) could not find a valid setup.py template for package '$(Package)'"; \
	fi

	sed -i 's#__gitrev__#$(GITREV)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__builddate__#$(BUILD_DATE)#' $(PackagePath)/rpm/setup.py
	# sed -i 's#__release__#$(BUILD_VERSION).$(PACKAGE_RELEASE).$(BUILD_DISTRIBUTION).$(BUILD_COMPILER)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__release__#${PACKAGE_FULL_RELEASE}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__version__#$(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR).$(PACKAGE_VER_PATCH)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__python_packages__#${PythonModules}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__pythonmodules__#${PythonModules}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__package__#${Package}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__packagedir__#$(PackagePath)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__packagename__#${PackageName}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__longpackage__#$(LongPackage)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__longpackagedir__#$(PackagePath)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__longpackagename__#$(LongPackage)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__prefix__#$(INSTALL_PATH)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__os__#${XDAQ_OS}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__platform__#None#' $(PackagePath)/rpm/setup.py
	sed -i 's#__project__#${Project}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__author__#${Packager}#' $(PackagePath)/rpm/setup.py
	sed -i 's#__buildarch__#$(XDAQ_PLATFORM)#' $(PackagePath)/rpm/setup.py
	# sed -i 's#__summary__#$(Summary)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__summary__#None#' $(PackagePath)/rpm/setup.py
	# sed -i 's#__description__#$(Description)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__description__#None#' $(PackagePath)/rpm/setup.py
	# sed -i 's#__long_description__#$(LongDescription)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__long_description__#None#' $(PackagePath)/rpm/setup.py
	# sed -i 's#__url__#$(Link)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__url__#None#' $(PackagePath)/rpm/setup.py
	# sed -i 's#__datadir__#$(DATA_DIR)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__username__#$(USER_NAME)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__groupname__#$(GROUP_NAME)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__uid__#$(UID)#' $(PackagePath)/rpm/setup.py
	sed -i 's#__gid__#$(GID)#' $(PackagePath)/rpm/setup.py

	## for pbr setup.cfg, maybe replace the previous... in any event, the setup.cfg takes precedence
	if [ -e $(PackagePath)/pkg/setup.cfg ]; then \
		echo Found $(PackagePath)/pkg/setup.cfg; \
		cp $(PackagePath)/pkg/setup.cfg $(PackagePath)/rpm/setup.cfg; \
	elif [ -e $(PackagePath)/setup.cfg ]; then \
		echo $(PackagePath) found setup.cfg; \
		cp $(PackagePath)/setup.cfg $(PackagePath)/rpm/setup.cfg; \
	elif [ -e $(ProjectPath)/config/build/cmsgemos.setupTemplate.cfg ]; then \
		echo $(ProjectPath)/config found cmsgemos.setupTemplate.cfg; \
		cp $(ProjectPath)/config/build/cmsgemos.setupTemplate.cfg $(PackagePath)/rpm/setup.cfg; \
	else \
		echo "Error (setup_update) could not find a valid setup.cfg template for package '$(Package)'"; \
	fi

	sed -i 's#__gitrev__#$(GITREV)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__builddate__#$(BUILD_DATE)#' $(PackagePath)/rpm/setup.cfg
	# sed -i 's#__release__#$(BUILD_VERSION).$(PACKAGE_RELEASE).$(BUILD_DISTRIBUTION).$(BUILD_COMPILER)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__release__#${PACKAGE_FULL_RELEASE}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__version__#$(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR).$(PACKAGE_VER_PATCH)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__python_packages__#${PythonModules}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__pythonmodules__#${PythonModules}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__package__#${Package}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__packagedir__#$(PackagePath)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__packagename__#${PackageName}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__longpackage__#$(LongPackage)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__longpackagedir__#$(PackagePath)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__longpackagename__#$(LongPackage)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__prefix__#$(INSTALL_PATH)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__os__#${XDAQ_OS}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__platform__#None#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__project__#${Project}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__author__#${Packager}#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__buildarch__#$(XDAQ_PLATFORM)#' $(PackagePath)/rpm/setup.cfg
	# sed -i 's#__summary__#$(Summary)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__summary__#None#' $(PackagePath)/rpm/setup.cfg
	# sed -i 's#__description__#$(Description)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__description__#None#' $(PackagePath)/rpm/setup.cfg
	# sed -i 's#__long_description__#$(LongDescription)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__long_description__#None#' $(PackagePath)/rpm/setup.cfg
	# sed -i 's#__url__#$(Link)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__url__#None#' $(PackagePath)/rpm/setup.cfg
	# sed -i 's#__datadir__#$(DATA_DIR)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__username__#$(USER_NAME)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__groupname__#$(GROUP_NAME)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__uid__#$(UID)#' $(PackagePath)/rpm/setup.cfg
	sed -i 's#__gid__#$(GID)#' $(PackagePath)/rpm/setup.cfg

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
