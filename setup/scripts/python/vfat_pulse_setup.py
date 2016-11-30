#!/bin/env python

import sys, re, time, datetime, os

sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *
from glib_system_info_uhal import *
from vfat_functions_uhal import *
from rate_calculator import errorRate
from glib_user_functions_uhal import *

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="AMC slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
		  help="GTX on the AMC", metavar="gtx", default=0)
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("--highword", type="int", dest="highword", default=0x0,
		  help="Bits to set on channels 127:96", metavar="highword")
parser.add_option("--lowword", type="int", dest="lowword", default=0x0,
		  help="Bits to set on channels 31:0", metavar="lowword")
parser.add_option("--vcal", type="int", dest="vcal", default=250,
		  help="VCal value to set", metavar="vcal")

(options, args) = parser.parse_args()

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 10
if options.slot:
    uTCAslot = 160+options.slot
print options.slot, uTCAslot
ipaddr        = '192.168.0.%d'%(uTCAslot)
uri           = "chtcp-2.0://localhost:10203?target=%s:50001"%(ipaddr)
address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
amc           = uhal.getDevice( "amc"       , uri, address_table )

print
print "--==============Low Word 0x%08x =========================--"%(options.lowword)
for chan in range(32):
    shift = chan
    if (((options.lowword) >> shift)&0x1):
        print "Setting channel %d to receivce calPulse"%(chan+1)
        writeAllVFATs(amc,options.gtx,"VFATChannels.ChanReg%d"%(chan+1),0x40,0x0,options.debug)
	if options.debug:
            readAllVFATs(amc,options.gtx,"VFATChannels.ChanReg%d"%(chan+1),0x0,options.debug)
	    pass
    else:
        writeAllVFATs(amc,options.gtx,"VFATChannels.ChanReg%d"%(chan+1),0x0,0x0)
    pass
print

print "--==============High Word 0x%08x ========================--"%(options.highword)
for chan in range(96,128):
    shift = chan - 96
    if (((options.highword) >> shift)&0x1):
        print "Setting channel %d to receivce calPulse"%(chan+1)
        writeAllVFATs(amc,options.gtx,"VFATChannels.ChanReg%d"%(chan+1),0x40,0x0,options.debug)
	if options.debug:
            readAllVFATs(amc,options.gtx,"VFATChannels.ChanReg%d"%(chan+1),0x0,options.debug)
	    pass
    else:
        writeAllVFATs(amc,options.gtx,"VFATChannels.ChanReg%d"%(chan+1),0x0,0x0)
    pass
print

print "Setting VCal to %d"%(options.vcal)
writeAllVFATs(amc,options.gtx,"VCal",options.vcal,0x0,options.debug)
if options.debug:
    readAllVFATs(amc,options.gtx,"VCal",0x0,options.debug)
    pass
