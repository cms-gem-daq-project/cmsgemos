# Created with insights from
## amc13/config/mfPythonRPMRules.mk

INSTALL_PATH = /opt/cmsgemos
ProjectPath  ?= $(BUILD_HOME)/$(Project)
PackagePath  ?= $(BUILD_HOME)/$(Project)/$(Package)
RPMBUILD_DIR = $(PackagePath)/rpm
# 
CMSGEMOS_OS ?= $(XDAQ_OS)

## Version variables from Makefile and ShortPackage
ShortPackageLoc   := $(shell echo "$(ShortPackage)" | tr '[:lower:]' '[:upper:]')

# ifndef PACKAGE_VER_MAJOR
# PACKAGE_VER_MAJOR := $($(ShortPackageLoc)_VER_MAJOR)
# else
# $(info mfDefs.gem PACKAGE_VER_MAJOR is already set to $(PACKAGE_VER_MAJOR))
# endif
# ifndef PACKAGE_VER_MINOR
# PACKAGE_VER_MINOR := $($(ShortPackageLoc)_VER_MINOR)
# else
# $(info mfDefs.gem PACKAGE_VER_MINOR is already set to $(PACKAGE_VER_MINOR))
# endif
# ifndef PACKAGE_VER_MINOR
# PACKAGE_VER_PATCH := $($(ShortPackageLoc)_VER_PATCH)
# else
# $(info mfDefs.gem PACKAGE_VER_PATCH is already set to $(PACKAGE_VER_PATCH))
# endif

# ifndef BUILD_VERSION
# BUILD_VERSION=1
# endif

# ifndef PACKAGE_FULL_VERSION
# PACKAGE_FULL_VERSION = $(PACKAGE_VER_MAJOR).$(PACKAGE_VER_MINOR).$(PACKAGE_VER_PATCH)
# endif

ifndef PACKAGE_FULL_RELEASE
PACKAGE_FULL_RELEASE = $(BUILD_VERSION).git$(GITREV).$(CMSGEMOS_OS)
endif

ifndef PythonModules
	$(error Python module names missing "PythonModules")
endif

.PHONY: _all
.PHONY: rpm _rpmall _rpmprep _setup_update _rpmbuild _rpmdevbuild _rpmsetup
rpm: _rpmbuild
	@echo "Running rpm target"
#	cat $(RPMBUILD_DIR)/setup.py

#_rpmall: _all _rpmprep _setup_update _rpmsetup _rpmbuild
_rpm: _all _rpmbuild
	@echo "Running _rpmall target"
#	cat $(RPMBUILD_DIR)/setup.py
# Copy the package skeleton
# Ensure the existence of the module directory
# Copy the libraries into python module
_rpmsetup: _rpmprep _setup_update
# Change directory into pkg and copy everything into rpm build dir
	@echo "Running _rpmsetup target"
#	cat $(RPMBUILD_DIR)/setup.py
	cd pkg && \
	find . -iname 'setup.*' -prune -o -name "*" -exec install -D \{\} $(RPMBUILD_DIR)/\{\} \;
# Add a manifest file (may not be necessary
#	echo "include */*.so" > $(RPMBUILD_DIR)/MANIFEST.in

_rpmbuild: _rpmsetup
	@echo "Running _rpmbuild target"
#	cat $(RPMBUILD_DIR)/setup.py
	cd $(RPMBUILD_DIR) && python setup.py bdist_rpm \
	--release python$(PYTHON_VERSION) \
#	--release $(CMSGEMOS_OS).python$(PYTHON_VERSION)
	--binary-only --force-arch=`uname -m`
# Harvest the crop
	find rpm -name "*.rpm"    -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tar.gz" -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tgz"    -exec cp -a \{\} rpm/ \;
	find rpm -name "*.tbz2"   -exec cp -a \{\} rpm/ \;

_setup_update:
	@echo "Running _setup_update target"
	$(MakeDir) $(PackagePath)/rpm/RPMBUILD

	if [ -e $(PackagePath)/pkg/setup.py ]; then \
		echo Found $(PackagePath)/pkg/setup.py; \
		cp $(PackagePath)/pkg/setup.py $(RPMBUILD_DIR)/setup.py; \
	elif [ -e $(PackagePath)/setup.py ]; then \
		echo $(PackagePath) found setup.py; \
		cp $(PackagePath)/setup.py $(RPMBUILD_DIR)/setup.py; \
	elif [ -e $(PackagePath)/setup/setup.py ]; then \
		echo Found $(PackagePath)/setup/setup.py; \
		cp $(PackagePath)/setup/setup.py $(RPMBUILD_DIR)/setup.py; \
	elif [ -e $(PackagePath)/setup/build/setup.py ]; then \
		echo Found $(PackagePath)/setup/build/setup.py; \
		cp $(PackagePath)/setup/build/setup.py $(RPMBUILD_DIR)/setup.py; \
	elif [ -e $(ProjectPath)/setup/config/setupTemplate.py ]; then \
		echo Found $(ProjectPath)/setup/config/setupTemplate.py; \
		cp $(ProjectPath)/setup/config/setupTemplate.py $(RPMBUILD_DIR)/setup.py; \
	elif [ -e $(BUILD_HOME)/config/build/setupTemplate.py ]; then \
		echo Found $(BUILD_HOME)/config/build/setupTemplate.pyz; \
		cp $(BUILD_HOME)/config/build/setupTemplate.py $(RPMBUILD_DIR)/setup.py; \
	else \
		echo Unable to find any setupTemplate.py; \
		exit 1; \
	fi

	sed -i 's#__author__#$(Packager)#'                $(RPMBUILD_DIR)/setup.py
	sed -i 's#__project__#$(Project)#'                $(RPMBUILD_DIR)/setup.py
	sed -i 's#__summary__#None#'                      $(RPMBUILD_DIR)/setup.py
	sed -i 's#__gitrev__#$(GITREV)#'                  $(RPMBUILD_DIR)/setup.py
	sed -i 's#__builddate__#$(BUILD_DATE)#'           $(RPMBUILD_DIR)/setup.py
	sed -i 's#__package__#$(Package)#'                $(RPMBUILD_DIR)/setup.py
	sed -i 's#__packagedir__#$(PackagePath)#'         $(RPMBUILD_DIR)/setup.py
	sed -i 's#__packagename__#$(PackageName)#'        $(RPMBUILD_DIR)/setup.py
	sed -i 's#__longpackage__#$(LongPackage)#'        $(RPMBUILD_DIR)/setup.py
	sed -i 's#__pythonmodules__#$(PythonModules)#'    $(RPMBUILD_DIR)/setup.py
	sed -i 's#__version__#$(PACKAGE_FULL_VERSION)#'   $(RPMBUILD_DIR)/setup.py
	sed -i 's#__release__#$(CMSGEMOS_OS)#'            $(RPMBUILD_DIR)/setup.py
	sed -i 's#__prefix__#$(GEMPYTHON_ROOT)#'          $(RPMBUILD_DIR)/setup.py
	sed -i 's#__os__#$(CMSGEMOS_OS)#'                 $(RPMBUILD_DIR)/setup.py
	sed -i 's#__platform__#$(CMSGEMOS_PLATFORM)#'     $(RPMBUILD_DIR)/setup.py
	sed -i 's#__description__#None#'                  $(RPMBUILD_DIR)/setup.py
#	cat $(RPMBUILD_DIR)/setup.py

	if [ -e $(PackagePath)/pkg/setup.cfg ]; then \
		echo Found $(PackagePath)/pkg/setup.cfg; \
		cp $(PackagePath)/pkg/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
	elif [ -e $(PackagePath)/setup.cfg ]; then \
		echo Found $(PackagePath)/setup.cfg; \
		cp $(PackagePath)/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
	elif [ -e $(PackagePath)/setup/setup.cfg ]; then \
		echo Found $(PackagePath)/setup/setup.cfg; \
		cp $(PackagePath)/setup/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
	elif [ -e $(PackagePath)/setup/build/setup.cfg ]; then \
		echo Found $(PackagePath)/setup/build/setup.cfg; \
		cp $(PackagePath)/setup/build/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
	elif [ -e $(ProjectPath)/setup/config/setupTemplate.cfg ]; then \
		echo Found $(ProjectPath)/setup/config/setupTemplate.cfg; \
		cp $(ProjectPath)/setup/config/setupTemplate.cfg $(RPMBUILD_DIR)/setup.cfg; \
	elif [ -e $(BUILD_HOME)/config/build/setupTemplate.cfg ]; then \
		echo Found $(BUILD_HOME)/config/setupTemplate.cfg; \
		cp $(BUILD_HOME)/config/build/setupTemplate.cfg $(RPMBUILD_DIR)/setup.cfg; \
	else \
		echo Unable to find any setupTemplate.cfg; \
		exit 1; \
	fi

	sed -i 's#__author__#$(Packager)#'                $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__project__#$(Project)#'                $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__summary__#None#'                      $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__gitrev__#$(GITREV)#'                  $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__builddate__#$(BUILD_DATE)#'           $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__package__#$(Package)#'                $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__packagedir__#$(PackagePath)#'         $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__packagename__#$(PackageName)#'        $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__longpackage__#$(LongPackage)#'        $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__pythonmodules__#$(PythonModules)#'    $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__version__#$(PACKAGE_FULL_VERSION)#'   $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__release__#$(CMSGEMOS_OS)#'            $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__prefix__#$(GEMPYTHON_ROOT)#'          $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__os__#$(CMSGEMOS_OS)#'                 $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__platform__#$(CMSGEMOS_PLATFORM)#'     $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__description__#None#'                  $(RPMBUILD_DIR)/setup.cfg


.PHONY: cleanrpm _cleanrpm
cleanrpm: _cleanrpm
	@echo "Running cleanrpm target"

_cleanrpm:
	@echo "Running _cleanrpm target"
	-rm -rf $(PackagePath)/rpm

.PHONY: _cleanrpmall
_cleanrpmall:
	-rm -rf $(PackagePath)/rpm


.PHONY: _installrpmall
_installrpmall:
	@mkdir -p $(INSTALL_PREFIX)/rpm
	-cp $(PackagePath)/rpm/*.rpm $(INSTALL_PREFIX)/rpm


.PHONY: _changelogall
_changelogall:
	@cd $(PackagePath); \
	git log --full-diff --decorate=full < ChangeLog
