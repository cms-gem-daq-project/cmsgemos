# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS?=
SPHINXBUILD?=sphinx-build
SPHINXAPIBUILD?=sphinx-apidoc
SOURCEDIR=source
SPHINXAPIDIR=sphinx-api
BUILDDIR=build
DOXYREST_PREFIX?=$(CONDA_PREFIX)

EOS_SITE_NAME?=test-gemdaq-ci
EOS_SITE_PATH?=/tmp/$(USER)/$(EOS_SITE_NAME)

export EOS_SITE_URL?=https://$(EOS_SITE_NAME).web.cern.ch/$(EOS_SITE_NAME)
export GEM_DOCS_URL?=$(EOS_SITE_URL)

ENV_NAME=condadocs

.EXPORT_ALL_VARIABLES:

.PHONY: help Makefile

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: docs
docs: html

.PHONY: setupenv envactivate envdeactivate
setupenv: envsetup
	conda create $(ENV_NAME) python=2.7
	conda actiavte $(ENV_NAME)
	pip install -r requirements.docs.txt
	conda install -y doxyrest

envactivate:
	conda activate $(ENV_NAME)

envdeactivate:
	conda deactivate

.PHONY: autoapi doxygen doxyrest
autoapi:
	$(SPHINXAPIBUILD) -M -o "$(SOURCEDIR)/$(SPHINXAPIDIR)" ../gempython/pkg/gempython

doxygen:
	cp cmsgemos.cfg.in cmsgemos.cfg
	perl -pi -e "s|__GEM_OS_PROJECT__|$(GEM_OS_PROJECT)|g" cmsgemos.cfg
	perl -pi -e "s|__GIT_VERSION__|$(GIT_VERSION)|g" cmsgemos.cfg
	doxygen cmsgemos.cfg

doxyrest: doxygen
	cp  doxyrest-config.lua.in doxyrest-config.lua
	perl -pi -e "s|__DOXYREST_PREFIX__|$(DOXYREST_PREFIX)|g" doxyrest-config.lua
	perl -pi -e "s|__DOXYREST_PREFIX__|$(DOXYREST_PREFIX)|g" doxyrest-config.lua
	doxyrest -c doxyrest-config.lua

.PHONY: clean cleanapi cleanall cleandoxy cleandoxyrest cleanexhale
clean:
	rm -rf $(BUILDDIR)

cleanapi:
	rm -rf $(SOURCEDIR)/$(SPHINXAPIDIR)

cleanexhale:
	rm -rf exhalebuild
	rm -rf $(SOURCEDIR)/exhale-api

cleandoxy:
	rm -rf doxybuild

cleandoxyrest:
	rm -rf $(SOURCEDIR)/doxyrest-api

cleanall: clean cleanapi cleanexhale cleandoxy cleandoxyrest

html: autoapi

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
