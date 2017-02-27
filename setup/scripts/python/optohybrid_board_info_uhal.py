#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *
from glib_user_functions_uhal import *
from optohybrid_user_functions_uhal import *
from rate_calculator import getErrorRate,errorRate

from standardopts import parser

(options, args) = parser.parse_args()

uhal.setLogLevelTo( uhal.LogLevel.FATAL )
from gemlogger import GEMLogger
gemlogger = GEMLogger("optohybrid_board_info_uhal").gemlogger
gemlogger.setLevel(GEMLogger.INFO)

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections_ch.xml"
manager         = uhal.ConnectionManager(connection_file )

amc  = manager.getDevice( "gem.shelf%02d.glib%02d"%(options.shelf,options.slot) )

SAMPLE_TIME = 1.

########################################
# IP address
########################################
print
print "-> -----------------"
print "-> OPTOHYBRID STATUS     "
print "-> -----------------"

fwver = getFirmwareVersion(amc,options.gtx)
date = '%02x/%02x/%04x'%(fwver["d"],fwver["m"],fwver["y"])
print "-> oh fw date : %s%s%s"%(colors.YELLOW,date,colors.ENDC)
print
printSysmonInfo(amc,options.gtx)
print
print "Connected VFATs mask: 0x%08x"%(getConnectedVFATsMask(amc,options.gtx,options.debug))
print "VFATs s-bit mask:     0x%08x"%(getVFATsBitMask(amc,options.gtx,options.debug))
setVFATsBitMask(amc,options.gtx,options.sbitmask,options.debug)
print "VFATs s-bit mask:     0x%08x"%(getVFATsBitMask(amc,options.gtx,options.debug))
print

if options.clkSrc in [0,1,2]:
    setReferenceClock(amc,options.gtx,options.clkSrc)
    pass
#print "-> OH VFATs accessible: 0x%x"%(readRegister(amc,"VFATs_TEST"))
if options.trgSrc in [0,1,2,3,4]:
    setTriggerSource(amc,options.gtx,options.trgSrc)
    pass

if options.localT1:
    #configureLocalT1(amc,options.gtx,0x0,0x0,0,25,100)
    sendL1ACalPulse(amc,options.gtx,15)
    pass

if options.sbitSrc in [1,2,3,4,5,6]:
    setTriggerSBits(False,amc,options.gtx,options.sbitSrc)
    pass

clocking = getClockingInfo(amc,options.gtx)
#OH:  TrgSrc  SBitSrc  FPGA PLL    EXT PLL    CDCE     GTX  RefCLKSrc
#->:     0x0      0x0       0x0       0x0      0x1     0x1        0x1
if options.v2b:
    print "Sources:  %6s  %7s"%("TrgSrc","SBitSrc")
    print "             0x%x      0x%x"%(
            readRegister(amc,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(options.gtx)),
            readRegister(amc,"GEM_AMC.OH.OH%d.CONTROL.HDMI_OUTPUT.SBITS"%(options.gtx)))

    print "Lock status:  %10s  %13s"%("QPLL","QPLL FPGA PLL")
    print "                     0x%x            0x%x"%(
            clocking["qplllock"],
            clocking["qpllfpgaplllock"])
    errorCounts = {}
    errorCounts["QPLL"] = []
    errorCounts["FPGA"] = []
    for trial in range(options.errorRate):
        errorCounts["QPLL"].append(calculateLockErrors(amc,options.gtx,"QPLL",SAMPLE_TIME))
        errorCounts["FPGA"].append(calculateLockErrors(amc,options.gtx,"QPLL_FPGA_PLL",SAMPLE_TIME))
        pass
    qrates = getErrorRate(errorCounts["QPLL"],SAMPLE_TIME)
    frates = getErrorRate(errorCounts["FPGA"],SAMPLE_TIME)
    print "Unlock count: 0x%08x     0x%08x"%(qrates[0],frates[0])
    0x00000107
    print "Unlock rate:%10sHz   %10sHz"%("%2.2f%s"%(qrates[1],qrates[2]),"%2.2f%s"%(frates[1],frates[2]))
else:
    print "Sources:  %6s  %7s  %9s"%("TrgSrc","SBitSrc","RefCLKSrc")
    print "             0x%x      0x%x        0x%x"%(
            readRegister(amc,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(options.gtx)),
            readRegister(amc,"GEM_AMC.OH.OH%d.CONTROL.OUTPUT.SBITS"%(options.gtx)),
            clocking["refclock"])

    print "Lock status:  %8s  %7s  %4s  %3s"%("FPGA PLL","EXT PLL","CDCE","GTX")
    print "                   0x%x      0x%x   0x%x  0x%x"%(
            clocking["fpgaplllock"],
            clocking["extplllock"],
            clocking["cdcelock"],
            clocking["gtxreclock"])
    pass
print
#print "-> OH Clocking (src, bkp):     VFAT         CDCE"
#print "-> %22s%d       (0x%x  0x%x)   (0x%x  0x%x)"%("link",links[link],
#						     clocking["vfatsrc"],clocking["vfatbkp"],
#						     clocking["cdcesrc"],clocking["cdcebkp"])
#exit(1)

if (options.resetCounters):
    optohybridCounters(amc,options.gtx,True)
    pass

print "-> Counters    %8s     %8s     %8s     %8s"%("L1A","Cal","Resync","BC0")
counters = optohybridCounters(amc,options.gtx)
for key in counters["T1"]:
    print "   %8s  0x%08x   0x%08x   0x%08x   0x%08x"%(key,
                                                       counters["T1"][key]["L1A"],
                                                       counters["T1"][key]["CalPulse"],
                                                       counters["T1"][key]["Resync"],
                                                       counters["T1"][key]["BC0"])
    pass

print
print "--=======================================--"
print "-> OH: %10s  %12s"%("ErrCnt","(rate)")
errorCounts = []
for trial in range(options.errorRate):
    errorCounts.append(calculateLinkErrors(False,amc,options.gtx,SAMPLE_TIME))
    pass
sys.stdout.flush()

rates = errorRate(errorCounts,SAMPLE_TIME)
#counters = optohybridCounters(amc,options.gtx)
print "-> TRK: 0x%08x  (%6.2f%1sHz)"%(rates["TRK"][0],rates["TRK"][1],rates["TRK"][2])
print "-> TRG: 0x%08x  (%6.2f%1sHz)"%(rates["TRG"][0],rates["TRG"][1],rates["TRG"][2])

print
# print "FIFO:  %8s  %7s  %10s"%("isEmpty",  "isFull", "depth")
# fifoInfo = readFIFODepth(amc,options.gtx)
# print "       %8s  %7s  %10s"%("0x%x"%(fifoInfo["isEMPTY"]),
#                                "0x%x"%(fifoInfo["isFULL"]),
#                                "0x%x"%(fifoInfo["Occupancy"]))
#
#print
print "--=======================================--"
