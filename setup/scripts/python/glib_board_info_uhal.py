#!/bin/env python

import sys, re
import time, datetime, os

sys.path.append('${GEM_PYTHON_PATH}')

NGTX = 2

import uhal
from registers_uhal import *
from glib_system_info_uhal import *
from rate_calculator import rateConverter,errorRate
from glib_user_functions_uhal import *

from gemlogger import GEMLogger
gemlogger = GEMLogger("glib_board_info_uhal").gemlogger

#from glib_clock_src import *

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
                  help="slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
                  help="GTX on the AMC", metavar="gtx", default=0)
parser.add_option("-c", "--clksrc", type="string", dest="clksrc",
                  help="select the input for the XPoint1 outputs", metavar="clksrc", default="local")
parser.add_option("-r", "--reset", action="store_true", dest="resetCounters",
                  help="reset link error counters", metavar="resetCounters")
parser.add_option("-x", "--external", type="int", dest="trgSrc",
                  help="change trigger source", metavar="trgSrc")
parser.add_option("-b", "--sbitout", type="int", dest="sbitSrc",
                  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-d", "--debug", action="store_true", dest="debug",
                  help="print extra debugging information", metavar="debug")
parser.add_option("-t", "--ttc", type="int", dest="gemttc", default=2,
                  help="choose the TTC encoding (gem/csc=0, amc13=1,default=2 meaning no modification)", metavar="gemttc")
parser.add_option("-e", "--errors", type="int", dest="errorRate", default=1,
                  help="calculate link error rates for N seconds", metavar="errorRate")
parser.add_option("-u", "--user", action="store_true", dest="userOnly",
                  help="print user information only", metavar="userOnly")
parser.add_option("--testbeam", action="store_true", dest="testbeam",
                  help="fixed IP address for testbeam", metavar="testbeam")
parser.add_option("--daq_enable", type="int", dest="daq_enable",
                  help="enable daq output", metavar="daq_enable", default=-1)
parser.add_option("--rd", type="int", dest="reset_daq",
                  help="reset daq", metavar="reset_daq", default=-1)
parser.add_option("--l1a_block", action="store_true", dest="l1a_block",
                  help="Inhibit the L1As at the TTC backplane link", metavar="l1a_block")
parser.add_option("--short", action="store_true", dest="short",
                  help="Skip extended information", metavar="short")
parser.add_option("--invertTX", type="string", dest="invertTX",default="",
                  help="Flip polarity for SFPs in list", metavar="invertTX")

(options, args) = parser.parse_args()

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 170
if options.slot:
    uTCAslot = 160+options.slot

if options.debug:
    msg = options.slot, uTCAslot
    gemlogger.debug(msg)

ipaddr = '192.168.0.%d'%(uTCAslot)
#ipaddr = '192.168.250.53'
if options.testbeam:
    # ipaddr        = '137.138.115.185'
    ipaddr        = '192.168.2.42'
    pass
## almost definitely better to use a connection file here?
#address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
address_table = "file://${GEM_ADDRESS_TABLE_PATH}/uhal_gem_amc_glib.xml"
uri = "chtcp-2.0://localhost:10203?target=%s:50001"%(ipaddr)
if options.testbeam:
    uri = "ipbustcp-2.0://192.168.250.42:60002"
    # uri = "ipbustcp-2.0://eagle45:60002"
    pass
glib  = uhal.getDevice( "glib" , uri, address_table )

########################################
# IP address
########################################
print
print "--=======================================--"
print "  Opening AMC with IP", ipaddr
print "--=======================================--"
print

# set all links to un-inverted
for sfp in range(4):
    writeRegister(glib,"GEM_AMC.GEM_SYSTEM.TK_LINK_TX_POLARITY",0x0)
    pass
# invert specified links
invertList = []
try:
    invertList = map(int, (options.invertTX).split(','))
except ValueError:
    print "Cannot convert %s to list of ints"%(options.invertTX)
    pass

if len(invertList) :
    for sfp in invertList:
        if (sfp > 4 or sfp < 0):
            print "invalid link specified, not inverting"
            continue
        writeRegister(glib,"GEM_AMC.GEM_SYSTEM.TK_LINK_TX_POLARITY",0x1)
        pass
    pass

if not options.userOnly:
	getSystemInfo(glib)
        pass
print
print "--=======================================--"
print "-> GEM SYSTEM INFORMATION"
print "--=======================================--"
print
print "TK_LINK_RX_POLARITY 0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.TK_LINK_RX_POLARITY"))
print "TK_LINK_TX_POLARITY 0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.TK_LINK_TX_POLARITY"))
print "BOARD_ID            0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.BOARD_ID"))
print "BOARD_TYPE          0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.BOARD_TYPE"))
print "RELEASE             0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.RELEASE"))
print "NUM_OF_OH           0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.CONFIG.NUM_OF_OH"))
print "USE_GBT             0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.CONFIG.USE_GBT"))
print "USE_TRIG_LINKS      0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.CONFIG.USE_TRIG_LINKS"))
print
print "--=======================================--"
print "-> DAQ INFORMATION"
print "--=======================================--"
print
if (options.l1a_block):
    blockL1A(glib)
else:
    enableL1A(glib)
    pass
if (options.resetCounters):
    glibCounters(glib,options.gtx,True)
    resetDAQLink(glib)
    pass
if (options.daq_enable>=0):
    enableDAQLink(glib)
    pass
print
print "-> DAQ control reg    :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.CONTROL"))
print "-> DAQ status reg     :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.STATUS"))
print "-> DAQ L1A ID         :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_STATUS.L1AID"))
print "-> DAQ sent events cnt:0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_STATUS.EVT_SENT"))
print
print "-> DAQ DAV_TIMEOUT  :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.CONTROL.DAV_TIMEOUT"))
print "-> DAQ RUN_TYPE     :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE"))
print "-> DAQ RUN_PARAMS   :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS"))
print
print "-> DAQ GTX NOT_IN_TABLE error counter:0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_STATUS.NOTINTABLE_ERR"))
print "-> DAQ GTX dispersion error counter  :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_STATUS.DISPER_ERR"))
print
print "-> AMC MAX_DAV_TIMER :0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_STATUS.MAX_DAV_TIMER"))
print "-> AMC LAST_DAV_TIMER:0x%08x"%(readRegister(glib,"GEM_AMC.DAQ.EXT_STATUS.LAST_DAV_TIMER"))

print
print "-> TTC Control :0x%08x"%(readRegister(glib,"GEM_AMC.TTC.CTRL"))
print "-> TTC Spy     :0x%08x"%(readRegister(glib,"GEM_AMC.TTC.TTC_SPY_BUFFER"))
print
print "-> RX Link Control :0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.TK_LINK_RX_POLARITY"))
print "-> TX Link Control :0x%08x"%(readRegister(glib,"GEM_AMC.GEM_SYSTEM.TK_LINK_TX_POLARITY"))

print
print "--=======================================--"
print "-> SSSSSSSSSSSSSSSSSSSSSSBITSSSSSSSSSSSSSS"
print "--=======================================--"
print "-> TRIGGER_RATE:%d %sHz"%(rateConverter(int(readRegister(glib,"GEM_AMC.TRIGGER.STATUS.OR_TRIGGER_RATE"))))
print
for olink in range(NGTX):
    print "-> DAQ OH%d clusters 0:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_0_RATE"%(olink)))
    print "-> DAQ OH%d clusters 1:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_1_RATE"%(olink)))
    print "-> DAQ OH%d clusters 2:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_2_RATE"%(olink)))
    print "-> DAQ OH%d clusters 3:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_3_RATE"%(olink)))
    print "-> DAQ OH%d clusters 4:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_4_RATE"%(olink)))
    print "-> DAQ OH%d clusters 5:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_5_RATE"%(olink)))
    print "-> DAQ OH%d clusters 6:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_6_RATE"%(olink)))
    print "-> DAQ OH%d clusters 7:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_7_RATE"%(olink)))
    print "-> DAQ OH%d clusters 8:0x%08x"%(olink,readRegister(glib,"GEM_AMC.TRIGGER.OH%d.CLUSTER_SIZE_8_RATE"%(olink)))
    print

if options.short:
    exit(0)

print "--=======================================--"
print "-> DAQ GTX INFO"
print "--=======================================--"
for olink in range(NGTX):
    print "-------------------------================--"
    print "----------> DAQ OH%d INFO <---------------"%(olink)
    print "-------------------------================--"
    print "---------> DAQ OH%d status                      :0x%08x"%(olink,readRegister(glib,"GEM_AMC.DAQ.OH%d.STATUS"%(olink)))
    print "---------> DAQ OH%d corrupted VFAT block counter:0x%08x"%(olink,readRegister(glib,"GEM_AMC.DAQ.OH%d.COUNTERS.CORRUPT_VFAT_BLK_CNT"%(olink)))
    print "---------> DAQ OH%d evn                         :0x%08x"%(olink,readRegister(glib,"GEM_AMC.DAQ.OH%d.COUNTERS.EVN"%(olink)))
    print
    print "---------> DAQ OH%d MAX_EOE_TIMER :0x%08x"%(olink,readRegister(glib,"GEM_AMC.DAQ.OH%d.COUNTERS.MAX_EOE_TIMER" %(olink)))
    print "---------> DAQ OH%d LAST_EOE_TIMER:0x%08x"%(olink,readRegister(glib,"GEM_AMC.DAQ.OH%d.COUNTERS.LAST_EOE_TIMER"%(olink)))
    print "---------> DAQ OH%d EOE_TIMEOUT   :0x%08x"%(olink,readRegister(glib,"GEM_AMC.DAQ.OH%d.CONTROL.EOE_TIMEOUT"%(olink)))
    print "-------------------------================--"
    print "-------------> DEBUG INFO <----------------"
    print "-------------------------================--"
    dbgWords = readBlock(glib,"GEM_AMC.DAQ.OH%d.LASTBLOCK"%(olink),7)
    for word in dbgWords:
        print "-> DAQ OH%d debug:0x%08x"%(olink,word)

print
print "--=======================================--"
print "-> BOARD USER INFORMATION"
print "--=======================================--"
print

if (options.resetCounters):
    glibCounters(glib,options.gtx,True)
    writeRegister(glib,"GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x1)
    writeRegister(glib,"GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x0)
print

sys.stdout.flush()
#for olink in range(NGTX):
#        print "--=====OH%d==============================--"%(olink)
#        errorCounts = []
#        SAMPLE_TIME = 1.
#        for trial in range(options.errorRate):
#                errorCounts.append(calculateLinkErrors(True,glib,olink,SAMPLE_TIME))
#        sys.stdout.flush()
#
#        rates = errorRate(errorCounts,SAMPLE_TIME)
#        print "-> TRK: 0x%08x  (%6.2f%1sHz)"%(rates["TRK"][0],rates["TRK"][1],rates["TRK"][2])
#        print "-> TRG: 0x%08x  (%6.2f%1sHz)"%(rates["TRG"][0],rates["TRG"][1],rates["TRG"][2])
#        print
#
#        sys.stdout.flush()
#        print "-> Counters    %8s     %8s     %8s     %8s"%("L1A","Cal","Resync","BC0")
#        counters = glibCounters(glib,olink)
#        print "   %8s  0x%08x   0x%08x   0x%08x   0x%08x"%(
#                "",
#                counters["T1"]["L1A"],
#                counters["T1"]["CalPulse"],
#                counters["T1"]["Resync"],
#                counters["T1"]["BC0"])
print "--=======================================--"
sys.stdout.flush()
