##
#
# This is the TriDAS/gem Project Makefile
#
##
BUILD_HOME:=$(shell pwd)/..

include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)
#include $(BUILD_HOME)/emu/rpm_version

Project=gemdaq-testing

### this will include other packages, like DAQ and DQM
ifeq ($(Set),framework)
Packages=\
	gembase \
        gemsupervisor
endif

### specific package target builds
#ifeq ($(Set),gemDAQ)
#Packages= \
#	gemDAQ/gemReadout \
#	gemDAQ/gemFU \
#	gemDAQ/gemRUI \
#	gemDAQ/gemTA \
#	gemDAQ/gemDAQManager \
#	gemDAQ/gemClient \
#	gemDAQ/gemUtil \
#	gemDAQ/drivers
#endif

ifeq ($(Set),gemsupervisor) 
Packages= \
          gembase \
          gemsupervisor
endif 

default:
Packages= \
	gembase \
	gemsupervisor

include $(XDAQ_ROOT)/config/Makefile.rules
include $(XDAQ_ROOT)/config/mfRPM.rules