# Created with insights from
## amc13/config/mfPythonRPMRules.mk

include $(BUILD_HOME)/config/mfCommonDefs.mk

INSTALL_PATH = /opt/cmsgemos
ProjectPath  = $(BUILD_HOME)
PackagePath  = $(ProjectPath)/$(Package)
RPMBUILD_DIR = $(PackagePath)/rpm
CMSGEMOS_OS ?= $(XDAQ_OS)

ifndef PACKAGE_FULL_RELEASE
PACKAGE_FULL_RELEASE = $(BUILD_VERSION).git$(GITREV).$(CMSGEMOS_OS)
endif

ifndef PythonModules
	$(error Python module names missing "PythonModules")
endif

.PHONY: _all
.PHONY: pip rpm _rpmall _rpmprep _setup_update _rpmbuild _rpmdevbuild _rpmsetup _bdistbuild _sdistbuild _harvest
pip: _sdistbuild _harvest
	@echo "Running pip target"

rpm: _rpmbuild _harvest
	@echo "Running rpm target"
	find $(RPMBUILD_DIR)/dist -iname "*.rpm" -print0 -exec mv -t $(RPMBUILD_DIR) {} \+

_rpm: _all _rpmbuild
	@echo "Running _rpmall target"

_rpmsetup: _rpmprep _setup_update
	@echo "Running _rpmsetup target"
	cd pkg && \
	find . -iname 'setup.*' -prune -o -iname "*" -exec install -D {} $(RPMBUILD_DIR)/{} \;
# Add a manifest file (may not be necessary
#	echo "include */*.so" > $(RPMBUILD_DIR)/MANIFEST.in

_rpmbuild: _sdistbuild
	@echo "Running _rpmbuild target for release: $(PACKAGE_NOARCH_RELEASE).$(CMSGEMOS).python$(PYTHON_VERSION)"
	cd $(RPMBUILD_DIR) && python setup.py \
	bdist_rpm \
	--release $(PACKAGE_NOARCH_RELEASE).$(CMSGEMOS_OS).python$(PYTHON_VERSION) \
	--force-arch=`uname -m`

_bdistbuild: _rpmsetup
	@echo "Running _tarbuild target"
	cd $(RPMBUILD_DIR) && python setup.py \
	egg_info --tag-build=$(PREREL_VERSION) \
	bdist --formats=bztar,gztar,zip

_sdistbuild: _rpmsetup
	@echo "Running _tarbuild target"
	cd $(RPMBUILD_DIR) && python setup.py \
	egg_info --tag-build=$(PREREL_VERSION) \
	sdist --formats=bztar,gztar,zip

_harvest:
# Harvest the crop
	find $(RPMBUILD_DIR)/dist \( -iname "*.tar.gz" \
	-o -iname "*.tar.bz2" \
	-o -iname "*.tgz" \
	-o -iname "*.zip" \
	-o -iname "*.tbz2" \) -print0 -exec mv -t $(RPMBUILD_DIR)/ {} \+
	-rename tar. t $(RPMBUILD_DIR)/*tar*

_setup_update:
	@echo "Running _setup_update target"
	$(MakeDir) $(RPMBUILD_DIR)

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
	sed -i 's#__package__#$(Package)#'                $(RPMBUILD_DIR)/setup.py
	sed -i 's#__packagedir__#$(PackagePath)#'         $(RPMBUILD_DIR)/setup.py
	sed -i 's#__packagename__#$(PackageName)#'        $(RPMBUILD_DIR)/setup.py
	sed -i 's#__longpackage__#$(LongPackage)#'        $(RPMBUILD_DIR)/setup.py
	sed -i 's#__pythonmodules__#$(PythonModules)#'    $(RPMBUILD_DIR)/setup.py
	sed -i 's#__prefix__#$(GEMPYTHON_ROOT)#'          $(RPMBUILD_DIR)/setup.py
	sed -i 's#__os__#$(CMSGEMOS_OS)#'                 $(RPMBUILD_DIR)/setup.py
	sed -i 's#__platform__#$(CMSGEMOS_PLATFORM)#'     $(RPMBUILD_DIR)/setup.py
	sed -i 's#__description__#None#'                  $(RPMBUILD_DIR)/setup.py
	sed -i 's#___gitrev___#$(GITREV)#'                $(RPMBUILD_DIR)/setup.py
	sed -i 's#___gitver___#$(GIT_VERSION)#'           $(RPMBUILD_DIR)/setup.py
	sed -i 's#___version___#$(PACKAGE_FULL_VERSION)#' $(RPMBUILD_DIR)/setup.py
	sed -i 's#___buildtag___#$(PREREL_VERSION)#'      $(RPMBUILD_DIR)/setup.py
	sed -i 's#___release___#$(BUILD_VERSION)#'        $(RPMBUILD_DIR)/setup.py
	sed -i 's#___packager___#$(GEMDEVELOPER)#'        $(RPMBUILD_DIR)/setup.py
	sed -i 's#___builddate___#$(BUILD_DATE)#'         $(RPMBUILD_DIR)/setup.py

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
	sed -i 's#__package__#$(Package)#'                $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__packagedir__#$(PackagePath)#'         $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__packagename__#$(PackageName)#'        $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__longpackage__#$(LongPackage)#'        $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__pythonmodules__#$(PythonModules)#'    $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__prefix__#$(GEMPYTHON_ROOT)#'          $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__os__#$(CMSGEMOS_OS)#'                 $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__platform__#$(CMSGEMOS_PLATFORM)#'     $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#__description__#None#'                  $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#___gitrev___#$(GITREV)#'                $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#___gitver___#$(GIT_VERSION)#'           $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#___version___#$(PACKAGE_FULL_VERSION)#' $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#___buildtag___#$(PREREL_VERSION)#'      $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#___release___#$(BUILD_VERSION)#'        $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#___packager___#$(GEMDEVELOPER)#'        $(RPMBUILD_DIR)/setup.cfg
	sed -i 's#___builddate___#$(BUILD_DATE)#'         $(RPMBUILD_DIR)/setup.cfg


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
	git log --full-diff --decorate=full > ChangeLog
