# Created with insights from
## amc13/config/mfPythonRPMRules.mk

include $(BUILD_HOME)/config/mfCommonDefs.mk

INSTALL_PATH = /opt/cmsgemos
ProjectPath  = $(BUILD_HOME)
PackagePath  = $(ProjectPath)/$(Package)
RPM_DIR:=$(PackagePath)/rpm
RPMBUILD_DIR:=$(RPM_DIR)/build
CMSGEMOS_OS ?= $(XDAQ_OS)

ifndef PACKAGE_FULL_RELEASE
PACKAGE_FULL_RELEASE = $(BUILD_VERSION).git$(GITREV).$(CMSGEMOS_OS)
endif

ifndef PythonModules
	$(error Python module names missing "PythonModules")
endif

TargetPIPName?=$(RPM_DIR)/$(PackageName).zip
TargetSRPMName?=$(RPM_DIR)/$(PackageName).src.rpm
TargetRPMName?=$(RPM_DIR)/$(PackageName).$(CMSGEMOS_ARCH).rpm
PackageSetupFile?=$(RPMBUILD_DIR)/setup.py
PackagePrepFile?=$(PackageDir)/$(PackageName).prep

PackagingTargets=$(TargetPIPName)
PackagingTargets+=$(TargetSRPMName)
PackagingTargets+=$(TargetRPMName)

.PHONY: install-pip install-site uninstall-pip uninstall-site

## @python-common install the python pip package
install-pip: pip
ifneq ($(or $(RPM_DIR),$(PackageName),$(PACKAGE_FULL_VERSION),$(PREREL_VERSION)),)
	pip install $(RPM_DIR)/$(PackageName)-$(PACKAGE_FULL_VERSION)$(PREREL_VERSION).zip
else
	@echo "install-pip requires that certain arguments are set"
	@echo "RPM_DIR is $(RPM_DIR)"
	@echo "PackageName is $(PackageName)"
	@echo "PACKAGE_FULL_VERSION is $(PACKAGE_FULL_VERSION)"
	@echo "PREREL_VERSION is $(PREREL_VERSION)"
	@exit 2
#	$(error "Unable to run install-site due to unset variables")
endif

ifeq ($(and $(Namespace),$(ShortPackage),$(INSTALL_PREFIX),$(PYTHON_SITE_PREFIX),$(CMSGEMOS_ROOT)),)
install-site uninstall-site: fail-pyinstall
fail-pyinstall:
	@echo "install-site requires that certain arguments are set"
	@echo "Namespace is $(Namespace)"
	@echo "ShortPackage is $(ShortPackage)"
	@echo "INSTALL_PREFIX is $(INSTALL_PREFIX)"
	@echo "PYTHON_SITE_PREFIX is $(PYTHON_SITE_PREFIX)"
	@exit 2
#	$(error "Unable to run install-site due to unset variables")
endif

install: install-site
## @python-common install the python site-package
install-site: rpmprep
	if [ -d pkg ]; then \
	   cd pkg; \
	   find $(Namespace) \( -type f -wholename $(PackagePrepFile) \) -prune -o -type f \
	       -exec install -D -m 755 {} $(INSTALL_PREFIX)$(PYTHON_SITE_PREFIX)/{} \; ; \
	   cd $(Namespace)/scripts; \
	   find . -type f \
	       -exec install -D -m 755 {} $(INSTALL_PREFIX)$(INSTALL_PATH)/bin/{} \; ; \
	fi

uninstall: uninstall-site
## @python-common uninstall the python pip package
uninstall-pip:
	pip uninstall $(PackageName)

## @python-common uninstall the python site-package
uninstall-site:
	$(RM) $(INSTALL_PREFIX)$(PYTHON_SITE_PREFIX)/$(Namespace)/$(ShortPackage)
	$(RM) $(INSTALL_PREFIX)$(INSTALL_PATH)/bin

.PHONY: pip rpm rpmprep
## @python-rpm Create a python package installable via pip
pip: _sdistbuildall _pipharvestall
## @python-rpm Create a python RPM package
rpm: _rpmbuildall _rpmharvestall
## @python-rpm Perform any specific setup before packaging, is a dependency of both `pip` and `rpm`
#rpmprep: _rpmprepall
_rpmprepall: | $(PackagePrepFile)
# Copy the package skeleton
# Ensure the existence of the module directory
# Copy the libraries into python module
# Change directory into pkg and copy everything into rpm build dir
	cd pkg && \
	    find . -iname 'setup.*' -prune -o -name "*" -exec install -D \{\} $(RPMBUILD_DIR)/\{\} \;
# Add a manifest file (may not be necessary
#	echo "include */*.so" > $(RPMBUILD_DIR)/MANIFEST.in

#.PHONY: _sdistbuildall _bdistbuildall
#.PHONY: _pipharvestall _rpmharvestall _rpmbuildall

_sdistbuildall: $(PackagePrepFile) $(TargetPIPName)

_rpmbuildall: $(PackagePrepFile) $(PackagingTargets)

$(TargetSRPMName): $(PackagePrepFile) $(PackageSetupFile) | rpmprep
	$(rpm-python-spec)
	rpmbuild --quiet -bs --clean \
	    --define "release $(PACKAGE_NOARCH_RELEASE)" \
	    --define "_binary_payload 1" \
	    --define "_topdir $(RPMBUILD_DIR)/$(CMSGEMOS_ARCH)" \
	    $(RPMBUILD_DIR)/dist/$(PackageName).spec;
	touch $@

$(TargetRPMName): $(PackagePrepFile) $(PackageSetupFile) | rpmprep
	$(rpm-python-spec)
	rpmbuild --quiet -bb --clean \
	    --define "release $(PACKAGE_NOARCH_RELEASE).$(CMSGEMOS_OS).python$(PYTHON_VERSION)" \
	    --define "_binary_payload 1" \
	    --define "_topdir $(RPMBUILD_DIR)/$(CMSGEMOS_ARCH)" \
	    $(RPMBUILD_DIR)/dist/$(PackageName).spec
	rename $(PACKAGE_FULL_VERSION) $(PACKAGE_FULL_VERSION)-$(PACKAGE_NOARCH_RELEASE) $(RPMBUILD_DIR)/$(CMSGEMOS_ARCH)/SOURCES/*$(PACKAGE_FULL_VERSION).tar.gz
	touch $@

$(TargetPIPName):  $(PackagePrepFile) $(PackageSetupFile) | rpmprep
	cd $(RPMBUILD_DIR) && python setup.py \
	    egg_info --tag-build=$(PREREL_VERSION) \
	    sdist --formats=bztar,gztar,zip
	touch $@

_bdistbuildall: rpmprep
	cd $(RPMBUILD_DIR) && python setup.py \
	    egg_info --tag-build=$(PREREL_VERSION) \
	    bdist --formats=bztar,gztar,zip

_pipharvestall: $(TargetPIPName)
	$(ProjectPath)/.ci/generate_repo.sh $(CMSGEMOS_OS) $(CMSGEMOS_ARCH) $(RPM_DIR) $(RPMBUILD_DIR) $(Project)

_rpmharvestall: $(TargetSRPMName) $(TargetRPMName)
	$(ProjectPath)/.ci/generate_repo.sh $(CMSGEMOS_OS) $(CMSGEMOS_ARCH) $(RPM_DIR) $(RPMBUILD_DIR) $(Project)

$(PackageSetupFile): $(ProjectPath)/config/build/setupTemplate.py $(ProjectPath)/config/build/setupTemplate.cfg
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

define rpm-python-spec =
cd $(RPMBUILD_DIR) && python setup.py \
    sdist --formats=gztar \
    bdist_rpm --quiet \
    --force-arch=$(CMSGEMOS_ARCH) \
    --spec-only;
mkdir -p $(RPMBUILD_DIR)/$(CMSGEMOS_ARCH)/SOURCES;
mv $(RPMBUILD_DIR)/dist/*.tar.gz $(RPMBUILD_DIR)/$(CMSGEMOS_ARCH)/SOURCES/;
sed -i '/%define release/d' $(RPMBUILD_DIR)/dist/$(PackageName).spec
endef

#############OLD NEEDED BY XDAQ SYSTEM...##############
#.PHONY: _rpmall
#############OLD OBSOLETE##############
# .PHONY: _all
# .PHONY: pip rpm _rpmall _rpmprep _setup_update _rpmbuild _rpmdevbuild _rpmsetup _bdistbuild _sdistbuild _harvest
# pip: _sdistbuild _harvest
# 	@echo "Running pip target"

# rpm: _rpmbuild _harvest
# 	@echo "Running rpm target"
# 	find $(RPMBUILD_DIR)/dist -iname "*.rpm" -print0 -exec mv -t $(RPMBUILD_DIR) {} \+

# _rpm: _all _rpmbuild
# 	@echo "Running _rpmall target"

# _rpmsetup: _rpmprep _setup_update
# 	@echo "Running _rpmsetup target"
# 	cd pkg && \
# 	find . -iname 'setup.*' -prune -o -iname "*" -exec install -D {} $(RPMBUILD_DIR)/{} \;
# # Add a manifest file (may not be necessary
# #	echo "include */*.so" > $(RPMBUILD_DIR)/MANIFEST.in

# _rpmbuild: _sdistbuild
# 	@echo "Running _rpmbuild target for release: $(PACKAGE_NOARCH_RELEASE).$(CMSGEMOS).python$(PYTHON_VERSION)"
# 	cd $(RPMBUILD_DIR) && python setup.py \
# 	bdist_rpm \
# 	--release $(PACKAGE_NOARCH_RELEASE).$(CMSGEMOS_OS).python$(PYTHON_VERSION) \
# 	--force-arch=`uname -m`

# _bdistbuild: _rpmsetup
# 	@echo "Running _tarbuild target"
# 	cd $(RPMBUILD_DIR) && python setup.py \
# 	egg_info --tag-build=$(PREREL_VERSION) \
# 	bdist --formats=bztar,gztar,zip

# _sdistbuild: _rpmsetup
# 	@echo "Running _tarbuild target"
# 	cd $(RPMBUILD_DIR) && python setup.py \
# 	egg_info --tag-build=$(PREREL_VERSION) \
# 	sdist --formats=bztar,gztar,zip

# _harvest:
# # Harvest the crop
# 	find $(RPMBUILD_DIR)/dist \( -iname "*.tar.gz" \
# 	-o -iname "*.tar.bz2" \
# 	-o -iname "*.tgz" \
# 	-o -iname "*.zip" \
# 	-o -iname "*.tbz2" \) -print0 -exec mv -t $(RPMBUILD_DIR)/ {} \+
# 	-rename tar. t $(RPMBUILD_DIR)/*tar*

# _setup_update:
# 	@echo "Running _setup_update target"
# 	$(MakeDir) $(RPMBUILD_DIR)

# 	if [ -e $(PackagePath)/pkg/setup.py ]; then \
# 		echo Found $(PackagePath)/pkg/setup.py; \
# 		cp $(PackagePath)/pkg/setup.py $(RPMBUILD_DIR)/setup.py; \
# 	elif [ -e $(PackagePath)/setup.py ]; then \
# 		echo $(PackagePath) found setup.py; \
# 		cp $(PackagePath)/setup.py $(RPMBUILD_DIR)/setup.py; \
# 	elif [ -e $(PackagePath)/setup/setup.py ]; then \
# 		echo Found $(PackagePath)/setup/setup.py; \
# 		cp $(PackagePath)/setup/setup.py $(RPMBUILD_DIR)/setup.py; \
# 	elif [ -e $(PackagePath)/setup/build/setup.py ]; then \
# 		echo Found $(PackagePath)/setup/build/setup.py; \
# 		cp $(PackagePath)/setup/build/setup.py $(RPMBUILD_DIR)/setup.py; \
# 	elif [ -e $(ProjectPath)/setup/config/setupTemplate.py ]; then \
# 		echo Found $(ProjectPath)/setup/config/setupTemplate.py; \
# 		cp $(ProjectPath)/setup/config/setupTemplate.py $(RPMBUILD_DIR)/setup.py; \
# 	elif [ -e $(BUILD_HOME)/config/build/setupTemplate.py ]; then \
# 		echo Found $(BUILD_HOME)/config/build/setupTemplate.pyz; \
# 		cp $(BUILD_HOME)/config/build/setupTemplate.py $(RPMBUILD_DIR)/setup.py; \
# 	else \
# 		echo Unable to find any setupTemplate.py; \
# 		exit 1; \
# 	fi

# 	sed -i 's#__author__#$(Packager)#'                $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__project__#$(Project)#'                $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__summary__#None#'                      $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__package__#$(Package)#'                $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__packagedir__#$(PackagePath)#'         $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__packagename__#$(PackageName)#'        $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__longpackage__#$(LongPackage)#'        $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__pythonmodules__#$(PythonModules)#'    $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__prefix__#$(GEMPYTHON_ROOT)#'          $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__os__#$(CMSGEMOS_OS)#'                 $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__platform__#$(CMSGEMOS_PLATFORM)#'     $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#__description__#None#'                  $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#___gitrev___#$(GITREV)#'                $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#___gitver___#$(GIT_VERSION)#'           $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#___version___#$(PACKAGE_FULL_VERSION)#' $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#___buildtag___#$(PREREL_VERSION)#'      $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#___release___#$(BUILD_VERSION)#'        $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#___packager___#$(GEMDEVELOPER)#'        $(RPMBUILD_DIR)/setup.py
# 	sed -i 's#___builddate___#$(BUILD_DATE)#'         $(RPMBUILD_DIR)/setup.py

# 	if [ -e $(PackagePath)/pkg/setup.cfg ]; then \
# 		echo Found $(PackagePath)/pkg/setup.cfg; \
# 		cp $(PackagePath)/pkg/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
# 	elif [ -e $(PackagePath)/setup.cfg ]; then \
# 		echo Found $(PackagePath)/setup.cfg; \
# 		cp $(PackagePath)/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
# 	elif [ -e $(PackagePath)/setup/setup.cfg ]; then \
# 		echo Found $(PackagePath)/setup/setup.cfg; \
# 		cp $(PackagePath)/setup/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
# 	elif [ -e $(PackagePath)/setup/build/setup.cfg ]; then \
# 		echo Found $(PackagePath)/setup/build/setup.cfg; \
# 		cp $(PackagePath)/setup/build/setup.cfg $(RPMBUILD_DIR)/setup.cfg; \
# 	elif [ -e $(ProjectPath)/setup/config/setupTemplate.cfg ]; then \
# 		echo Found $(ProjectPath)/setup/config/setupTemplate.cfg; \
# 		cp $(ProjectPath)/setup/config/setupTemplate.cfg $(RPMBUILD_DIR)/setup.cfg; \
# 	elif [ -e $(BUILD_HOME)/config/build/setupTemplate.cfg ]; then \
# 		echo Found $(BUILD_HOME)/config/setupTemplate.cfg; \
# 		cp $(BUILD_HOME)/config/build/setupTemplate.cfg $(RPMBUILD_DIR)/setup.cfg; \
# 	else \
# 		echo Unable to find any setupTemplate.cfg; \
# 		exit 1; \
# 	fi

# 	sed -i 's#__author__#$(Packager)#'                $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__project__#$(Project)#'                $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__summary__#None#'                      $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__package__#$(Package)#'                $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__packagedir__#$(PackagePath)#'         $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__packagename__#$(PackageName)#'        $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__longpackage__#$(LongPackage)#'        $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__pythonmodules__#$(PythonModules)#'    $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__prefix__#$(GEMPYTHON_ROOT)#'          $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__os__#$(CMSGEMOS_OS)#'                 $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__platform__#$(CMSGEMOS_PLATFORM)#'     $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#__description__#None#'                  $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#___gitrev___#$(GITREV)#'                $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#___gitver___#$(GIT_VERSION)#'           $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#___version___#$(PACKAGE_FULL_VERSION)#' $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#___buildtag___#$(PREREL_VERSION)#'      $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#___release___#$(BUILD_VERSION)#'        $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#___packager___#$(GEMDEVELOPER)#'        $(RPMBUILD_DIR)/setup.cfg
# 	sed -i 's#___builddate___#$(BUILD_DATE)#'         $(RPMBUILD_DIR)/setup.cfg


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
