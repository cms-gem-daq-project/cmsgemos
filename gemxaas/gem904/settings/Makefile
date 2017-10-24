# $Id: Makefile,v 1.22 2009/03/04 10:44:22 lorsini Exp $

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2014, CERN.			                #
# All rights reserved.                                                  #
# Authors: L. Orsini and A.petrucci					#
#                                                                       #
# For the licensing terms see LICENSE.		                        #
# For the list of contributors see CREDITS.   			        #
#########################################################################

##
#
# Makefile example for Slim settings 
#
##


#
# Packages to be built
#
BUILD_HOME:=$(shell pwd)/../../../../..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=daq
PackageName=$(shell pwd | awk -F"/" '{split($$0,a,"/");  print a[NF-1]}')
Package=xaas/slim/$(PackageName)

ifndef BUILD_VERSION
BUILD_VERSION=1
endif

PACKAGE_VER_MAJOR=1
PACKAGE_VER_MINOR=0
PACKAGE_VER_PATCH=0

TEMPLATEDIR=$(BUILD_HOME)/$(Project)/xaas/slim/template/settings

build: _buildall

_buildall: all

_all: all

default: all

all: 
	make -f $(TEMPLATEDIR)/settings.makefile ZONE_NAME=$(PackageName)
_installall: install

install: 

_cleanall: clean

clean:
	make -f $(TEMPLATEDIR)/settings.makefile ZONE_NAME=$(PackageName) clean

_rpmall: rpm

rpm:
	make -f $(TEMPLATEDIR)/settings.makefile ZONE_NAME=$(PackageName) BUILD_VERSION=$(BUILD_VERSION) PACKAGE_VER_MAJOR=$(PACKAGE_VER_MAJOR) PACKAGE_VER_MINOR=$(PACKAGE_VER_MINOR) PACKAGE_VER_PATCH=$(PACKAGE_VER_PATCH) rpm


_installrpmall: installrpm

installrpm:
	make -f $(TEMPLATEDIR)/settings.makefile ZONE_NAME=$(PackageName) installrpm

_cleanrpmall: cleanrpm

cleanrpm:
	make -f $(TEMPLATEDIR)/settings.makefile ZONE_NAME=$(PackageName) cleanrpm


