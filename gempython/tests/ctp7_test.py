#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *

#from glib_clock_src import *

#from optparse import OptionParser
#parser = OptionParser()

#(options, args) = parser.parse_args()

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

ipaddr = '192.168.250.53'
address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
uri = "ipbustcp-2.0://eagle45:60002"
ctp7  = uhal.getDevice( "CTP7" , uri, address_table )

########################################
# IP address
########################################
print
print "--=======================================--"
print "  Opening CTP7 with IP", ipaddr
print "--=======================================--"
print

print
print "--=======================================--"
print "-> DAQ INFORMATION"
print "--=======================================--"
print

print "-> DAQ control reg    :0x%08x"%(readRegister(ctp7,"GLIB.DAQ.CONTROL"))
print "-> DAQ status reg     :0x%08x"%(readRegister(ctp7,"GLIB.DAQ.STATUS"))

