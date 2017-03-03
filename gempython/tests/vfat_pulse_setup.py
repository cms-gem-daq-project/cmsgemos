#!/bin/env python

import sys, re, time, datetime, os

sys.path.append('${GEM_PYTHON_PATH}')

from gempython.tools.glib_system_info_uhal import *
from gempython.tools.vfat_functions_uhal import *
from gempython.tools.glib_user_functions_uhal import *
from gempython.utils.rate_calculator import errorRate

from gempython.utils.standardopts import parser

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

gemlogger = GEMLogger("vfat_pulse_setup").gemlogger
gemlogger.setLevel(GEMLogger.INFO)

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"
manager         = uhal.ConnectionManager(connection_file )

amc  = manager.getDevice( "gem.shelf%02d.glib%02d"%(options.shelf,options.slot) )

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
