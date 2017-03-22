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

SAMPLE_TIME = 1.

print
print "-> --------------------------"
print "->     OPTOHYBRID STATUS     "
print "-> --------------------------"

fwver   = getFirmwareVersion(ohboard,options.gtx)
date    = getFirmwareDateString(ohboard,options.gtx)
print "-> OH FW %10s  %10s"%("Version","Date")
print "  ->     %s%10s%s  %s%10s%s"%(colors.YELLOW,date,colors.ENDC,
                                     colors.YELLOW,fwver,colors.ENDC)
print
#printSysmonInfo(ohboard,options.gtx)
print
print "Connected VFATs mask: 0x%08x"%(getConnectedVFATsMask(ohboard,options.gtx,options.debug))
# print "VFATs s-bit mask:     0x%08x"%(getVFATsBitMask(ohboard,options.gtx,options.debug))
setVFATsBitMask(ohboard,options.gtx,options.sbitmask,options.debug)
print "VFATs s-bit mask:     0x%08x"%(getVFATsBitMask(ohboard,options.gtx,options.debug))
print

if options.clkSrc in [0,1,2]:
    setReferenceClock(ohboard,options.gtx,options.clkSrc)
    pass
#print "-> OH VFATs accessible: 0x%x"%(readRegister(ohboard,"VFATs_TEST"))
if options.trgSrc in [0,1,2,3,4]:
    setTriggerSource(ohboard,options.gtx,options.trgSrc)
    pass

if options.localT1:
    #configureLocalT1(ohboard,options.gtx,0x0,0x0,0,25,100)
    sendL1ACalPulse(ohboard,options.gtx,15)
    pass

if options.sbitSrc in [1,2,3,4,5,6]:
    setTriggerSBits(False,amc,options.gtx,options.sbitSrc)
    pass

clocking = getClockingInfo(ohboard,options.gtx)
#OH:  TrgSrc  SBitSrc  FPGA PLL    EXT PLL    CDCE     GTX  RefCLKSrc
#->:     0x0      0x0       0x0       0x0      0x1     0x1        0x1
if options.v2a:
    print "Sources:  %6s  %7s  %9s"%("TrgSrc","SBitSrc","RefCLKSrc")
    print "             0x%x      0x%x        0x%x"%(
            readRegister(ohboard,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(options.gtx)),
            readRegister(ohboard,"GEM_AMC.OH.OH%d.CONTROL.HDMI_OUTPUT.SBITS"%(options.gtx)),
            clocking["refclock"])

    print "Lock status:  %8s  %7s  %4s  %3s"%("FPGA PLL","EXT PLL","CDCE","GTX")
    print "                   0x%x      0x%x   0x%x  0x%x"%(
            clocking["fpgaplllock"],
            clocking["extplllock"],
            clocking["cdcelock"],
            clocking["gtxreclock"])
    pass
else:
    print "Sources:  %6s  %7s  %9s"%("TrgSrc","SBitSrc","RefCLKSrc")
    print "             0x%x      0x%x        0x%x"%(
            readRegister(ohboard,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(options.gtx)),
            readRegister(ohboard,"GEM_AMC.OH.OH%d.CONTROL.HDMI_OUTPUT.SBITS"%(options.gtx)),
            clocking["refclock"])

    print "Lock status:  %10s  %13s"%("QPLL","QPLL FPGA PLL")
    print "                     0x%x            0x%x"%(
            clocking["qplllock"],
            clocking["qpllfpgaplllock"])
    errorCounts = nesteddict()
    errorCounts["QPLL"] = []
    errorCounts["FPGA"] = []
    for trial in range(options.errorRate):
        errorCounts["QPLL"].append(calculateLockErrors(ohboard,options.gtx,"QPLL",SAMPLE_TIME))
        errorCounts["FPGA"].append(calculateLockErrors(ohboard,options.gtx,"QPLL_FPGA_PLL",SAMPLE_TIME))
        pass
    qrates = getErrorRate(errorCounts["QPLL"],SAMPLE_TIME)
    frates = getErrorRate(errorCounts["FPGA"],SAMPLE_TIME)
    print "Unlock count: 0x%08x     0x%08x"%(qrates[0],frates[0])
    print "Unlock rate:%10sHz   %10sHz"%("%2.2f%s"%(qrates[1],qrates[2]),"%2.2f%s"%(frates[1],frates[2]))
print
#print "-> OH Clocking (src, bkp):     VFAT         CDCE"
#print "-> %22s%d       (0x%x  0x%x)   (0x%x  0x%x)"%("link",links[link],
#						     clocking["vfatsrc"],clocking["vfatbkp"],
#						     clocking["cdcesrc"],clocking["cdcebkp"])
#exit(1)

if (options.resetCounters):
    optohybridCounters(ohboard,options.gtx,True)
    pass

print "-> Counters    %8s     %8s     %8s     %8s"%("L1A","Cal","Resync","BC0")
counters = optohybridCounters(ohboard,options.gtx)
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
    errorCounts.append(calculateLinkErrors(ohboard,options.gtx,SAMPLE_TIME))
    pass
sys.stdout.flush()

rates = errorRate(errorCounts,SAMPLE_TIME)
#counters = optohybridCounters(ohboard,options.gtx)
print "-> LINK: %8d  (%6.2f%1sHz)"%(rates["LINK"][0],rates["LINK"][1],rates["LINK"][2])
print "-> GTX:  %8d  (%6.2f%1sHz)"%(rates["GTX"][0],rates["GTX"][1],rates["GTX"][2])
print "-> GBT:  %8d  (%6.2f%1sHz)"%(rates["GBT"][0],rates["GBT"][1],rates["GBT"][2])

print
print "--=======================================--"
