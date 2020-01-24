# $Id: Makefile,v 1.21 2012-04-26 20:06:21 mansj Exp $

#########################################################################
# XDAQ Components for Distributed Data Acquisition                      #
# Copyright (C) 2000-2004, CERN.			                #
# All rights reserved.                                                  #
# Authors: J. Gutleber and L. Orsini					#
#                                                                       #
# For the licensing terms see LICENSE.		                        #
# For the list of contributors see CREDITS.   			        #
#########################################################################

##
#
# 
# 
#
##


#
# Packages to be built
#

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfAutoconf.rules
include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfDefs.$(XDAQ_OS)

PackageName=hcal
Project=daq
Package=setup/$(PackageName)

PACKAGE_VER_MAJOR=11
PACKAGE_VER_MINOR=4
PACKAGE_VER_PATCH=0

Summary=Setup data for the HCAL subcluster

Description=HCAL Subcluster setup (downstairs)

Authors=Jeremiah Mans and Dragoslav Lazic

Link=http://xdaqwiki.cern.ch

_all: basic

default: basic

all:
	echo ${PACKAGE_VER_MAJOR}_${PACKAGE_VER_MINOR}_${PACKAGE_VER_PATCH} > conf/hcal.release
	rm -rf $(BUILD_HOME)/daq/setup
	mkdir -p $(BUILD_HOME)/${Project}
	mkdir -p $(BUILD_HOME)/${Project}/setup
	cp -rf $(BUILD_HOME)/hcal/setup/${PackageName} $(BUILD_HOME)/${Project}/setup/.  
	mkdir -p etc/cron.d
	touch etc/cron.d/hcal.cron 

basic:
	echo ${PACKAGE_VER_MAJOR}_${PACKAGE_VER_MINOR}_${PACKAGE_VER_PATCH} > conf/hcal.release
	rm -rf $(BUILD_HOME)/daq/setup
	mkdir -p $(BUILD_HOME)/${Project}
	mkdir -p $(BUILD_HOME)/${Project}/setup
	cp -rf $(BUILD_HOME)/hcal/setup/${PackageName} $(BUILD_HOME)/${Project}/setup/.  
	mkdir -p etc/cron.d
	touch etc/cron.d/hcal.cron 

_cleanall: clean

clean:
	-rm -f conf/hcal.release
	-rm -rf $(BUILD_HOME)/daq/setup

include $(XDAQ_ROOT)/$(BUILD_SUPPORT)/mfSetupRPM.rules
