#!/bin/env python

from gempython.tools.optohybrid_user_functions_uhal import *
from gempython.utils.rate_calculator import getErrorRate,errorRate

import logging
from gempython.utils.gemlogger import colors,getGEMLogger

from gempython.utils.standardopts import parser
parser.add_option("-k", "--clkSrc", type="int", dest="clkSrc",
		  help="which reference clock to use on OH", metavar="clkSrc")
parser.add_option("-l", "--localT1", action="store_true", dest="localT1",
		  help="enable the localT1 controller", metavar="localT1")
parser.add_option("--v2a", action="store_true", dest="v2a",
		  help="Specific functionality only in v2a", metavar="v2a")
parser.add_option("--sbitmask", type="int", dest="sbitmask",default=0x0,
		  help="use s-bit mask", metavar="sbitmask")
parser.add_option("--sbitout", type="int", dest="sbitSrc",
                  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-x", "--external", type="int", dest="trgSrc",
                  help="change trigger source", metavar="trgSrc")

(options, args) = parser.parse_args()

gemlogger = getGEMLogger(__name__)
gemlogger.setLevel(logging.INFO)

uhal.setLogLevelTo( uhal.LogLevel.WARNING )

ohboard = getOHObject(options.slot,options.gtx,options.shelf,options.debug)
setOHLogLevel(logging.INFO)
if options.debug:
    setOHLogLevel(logging.DEBUG)

SAMPLE_TIME = 0.25

print
print "-> --------------------------"
print "->     OPTOHYBRID STATUS     "
print "-> --------------------------"

fwver   = getFirmwareVersion(ohboard,options.gtx)
date    = getFirmwareDateString(ohboard,options.gtx)
print "-> OH FW %10s  %10s"%("Version","Date")
print "         %s%10s%s  %s%10s%s"%(colors.YELLOW,fwver,colors.ENDC,
                                     colors.YELLOW,date,colors.ENDC)
print
print "Connected VFATs mask: 0x%08x"%(getConnectedVFATsMask(ohboard,options.gtx,options.debug))
setVFATsBitMask(ohboard,options.gtx,options.sbitmask,options.debug)
print "VFATs s-bit mask:     0x%08x"%(getVFATsBitMask(ohboard,options.gtx,options.debug))
print

clocking = getClockingInfo(ohboard,options.gtx)
print "Lock status:  %10s  %13s"%("GBT MMCM","LOGIC MMCM")
print "                     0x%x            0x%x"%(
    clocking["gbtmmcmlock"],
    clocking["logicmmcmlock"])
errorCounts = nesteddict()
print "Unlock count: 0x%08x     0x%08x"%(
    clocking["gbtmmcmlockcnt"],
    clocking["logicmmcmlockcnt"])

print
print "--=======================================--"
