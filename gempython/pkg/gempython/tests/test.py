#!/bin/env python

import cProfile, pstats, StringIO
pr = cProfile.Profile()
pr.enable()

import sys, re, signal
import time, datetime, os
import random

sys.path.append('${GEM_PYTHON_PATH}')
import logging
from gempython.utils.gemlogger import colors,printmsg

NGTX = 2

def countsExpected(a1,a2,b1,b2,exp):
    aexp = (a2-a1-exp) == 0
    bexp = (b2-b1-exp) == 0
    print "  %sL1A: %d, CalPulse: %d, Expected:%d%s"%(colors.GREEN if (aexp and bexp) else colors.RED,
                                                      a2-a1,
                                                      b2-b1,
                                                      exp,colors.ENDC)
    sys.stdout.flush()
    return

from gempython.tools.glib_system_info_uhal import *
from gempython.utils.rate_calculator import rateConverter,errorRate
from gempython.tools.glib_user_functions_uhal import *
from gempython.tools.optohybrid_user_functions_uhal import *
from gempython.tools.vfat_functions_uhal import *

from gempython.utils.standardopts import parser

parser.add_option("--t1test", action="store_true", dest="testt1",
		  help="Run tests on the OH T1 controller", metavar="testt1")
parser.add_option("--scantest", action="store_true", dest="scantest",
		  help="Run tests on OH scan controller", metavar="scantest")
parser.add_option("--getdata", action="store_true", dest="getdata",
		  help="Run return the data from the scan", metavar="getdata")
parser.add_option("--vfatchan", action="store_true", dest="vfatchan",
		  help="Run tests on the VFAT channel registers per VFAT", metavar="vfatchan")
parser.add_option("--broadcastchan", action="store_true", dest="broadcastchan",
		  help="Run tests on the VFAT channel registers with broadcast", metavar="broadcastchan")
parser.add_option("--tmd", type="int", dest="tmd", default=0,
		  help="Run tests using multiple dispatch (2) or regular single (1), default is off (0)", metavar="tmd")

(options, args) = parser.parse_args()

# logging.setLoggerClass(GEMLogger)
# gemlogger = GEMLogger("optohybrid_user_functions")
gemlogger = getGEMLogger(logclassname="optohybrid_user_functions")
print gemlogger
gemlogger.setLevel(logging.INFO)

uhal.setLogLevelTo( uhal.LogLevel.WARNING )

connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"
print "CREATING MANAGER"
manager         = uhal.ConnectionManager(connection_file )
print "CREATING AMC"
amc     = manager.getDevice( "gem.shelf%02d.amc%02d"%(options.shelf,options.slot) )
ohboard = manager.getDevice( "gem.shelf%02d.amc%02d.optohybrid%02d"%(options.shelf,options.slot,options.gtx) )
print "running script"
sys.stdout.flush()
########################################
# IP address
########################################
print
print "--=======================================--"
print "  Testing AMC with ID", amc
print "--=======================================--"
print
sys.stdout.flush()
## Test the AMC firmware
printmsg("BOARD_ID   0x%08x"%readRegister(amc,"GEM_AMC.GEM_SYSTEM.BOARD_ID",    options.debug),
         colors.GREEN,colors.ENDC)
printmsg("BOARD_TYPE 0x%08x"%readRegister(amc,"GEM_AMC.GEM_SYSTEM.BOARD_TYPE",  options.debug),
         colors.foreground.GREEN,colors.ENDC)
printmsg("FW DATE    0x%08x"%readRegister(amc,"GEM_AMC.GEM_SYSTEM.RELEASE.DATE",options.debug),
         colors.GREEN,colors.ENDC)
sys.stdout.flush()

## Test multiple dispatch in uHAL
if options.tmd > 0:
    if options.tmd == 2:
        reg_wr_list = cdict(dict)
        reg_rd_list = []
        # reg_rd_list.append("GEM_AMC.GEM_SYSTEM.BOARD_ID")
        # reg_rd_list.append("GEM_AMC.GEM_SYSTEM.BOARD_TYPE")
        # reg_rd_list.append("GEM_AMC.GEM_SYSTEM.RELEASE.DATE")
        baseNode = "GEM_AMC.OH.OH"
        # ohID   = random.randint(0, 9)
        # vfatID = random.randint(0, 23)
        # chanID = random.randint(0, 127)
        wr32   = random.randint(0, 0xffffff)
        # wr8    = random.randint(0, 0xff)
        reg_wr_list["%s%d.CONTROL.VFAT.TRK_MASK"%(baseNode,options.gtx)] = wr32
        for vf in range (10):
            vfatID = random.randint(0, 23)
            wr8    = random.randint(0, 0xff)
            reg_wr_list["%s%d.GEB.VFATS.VFAT%d.IShaperFeed"%(baseNode,options.gtx,vfatID)] = wr8
            reg_rd_list.append("%s%d.GEB.VFATS.VFAT%d.IShaperFeed"%(baseNode,options.gtx,vfatID))
            for ch in range(25):
                chanID = random.randint(0, 127)
                wr8    = random.randint(0, 0xff)
                reg_wr_list["%s%d.GEB.VFATS.VFAT%d.VFATChannels.ChanReg%d"%(baseNode,options.gtx,vfatID,chanID)] = wr8
                reg_rd_list.append("%s%d.GEB.VFATS.VFAT%d.VFATChannels.ChanReg%d"%(baseNode,options.gtx,vfatID,chanID))
                pass
            pass
        reg_rd_list.append("%s%d.CONTROL.VFAT.TRK_MASK"%(baseNode,options.gtx))

        # reg_read = readRegisterList(ohboard,reg_rd_list,options.debug)
        # print reg_read
        # for reg in reg_read:
        #     print reg
        #     print "0x%x"%reg_read[reg]
        #     pass
        writeRegisterList(ohboard,reg_wr_list,options.debug)
        reg_read = readRegisterList(ohboard,reg_rd_list,options.debug)
        print reg_read
        for reg in reg_read:
            print reg
            print "0x%x"%reg_read[reg]
            pass
    elif options.tmd==1:
        print "0x%x"%readRegister(amc,"GEM_AMC.GEM_SYSTEM.BOARD_ID")
        print "0x%x"%readRegister(amc,"GEM_AMC.GEM_SYSTEM.BOARD_TYPE")
        print "0x%x"%readRegister(amc,"GEM_AMC.GEM_SYSTEM.RELEASE.DATE")
        baseNode = "GEM_AMC.OH.OH"
        # ohID   = random.randint(0, 9)
        # vfatID = random.randint(0, 23)
        # chanID = random.randint(0, 127)
        wr32   = random.randint(0, 0xffffff)
        # wr8    = random.randint(0, 0xff)
        print "0x%x"%readRegister(ohboard,"%s%d.CONTROL.VFAT.TRK_MASK"%(baseNode,options.gtx))
        writeRegister(ohboard,"%s%d.CONTROL.VFAT.TRK_MASK"%(baseNode,options.gtx), wr32)
        print "0x%x"%readRegister(ohboard,"%s%d.CONTROL.VFAT.TRK_MASK"%(baseNode,options.gtx))
        for vf in range (10):
            vfatID = random.randint(0, 23)
            wr8    = random.randint(0, 0xff)
            print "0x%x"%readRegister(ohboard,"%s%d.GEB.VFATS.VFAT%d.IShaperFeed"%(baseNode,options.gtx,vfatID))
            writeRegister(ohboard,"%s%d.GEB.VFATS.VFAT%d.IShaperFeed"%(baseNode,options.gtx,vfatID),wr8)
            print "0x%x"%readRegister(ohboard,"%s%d.GEB.VFATS.VFAT%d.IShaperFeed"%(baseNode,options.gtx,vfatID))
            for ch in range(25):
                chanID = random.randint(0, 127)
                wr8    = random.randint(0, 0xff)
                print "0x%x"%readRegister(ohboard,"%s%d.GEB.VFATS.VFAT%d.VFATChannels.ChanReg%d"%(baseNode,options.gtx,vfatID,chanID))
                writeRegister(ohboard,"%s%d.GEB.VFATS.VFAT%d.VFATChannels.ChanReg%d"%(baseNode,options.gtx,vfatID,chanID), wr8)
                print "0x%x"%readRegister(ohboard,"%s%d.GEB.VFATS.VFAT%d.VFATChannels.ChanReg%d"%(baseNode,options.gtx,vfatID,chanID))
                pass
            pass
        pass
    pass
## Test the OH firmware
printmsg("OH FW VERSION %s"%getFirmwareVersion(ohboard,options.gtx,debug=options.debug),colors.foreground.MAGENTA,colors.ENDC)
date = getFirmwareDate(ohboard,options.gtx,debug=options.debug)
printmsg("OH FW DATE  %02x/%02x/%04x"%(date["d"],date["m"],date["y"]),colors.MAGENTA,colors.ENDC)
sys.stdout.flush()

## Test the T1 controller
if options.testt1:
    print "Testing the T1 controller"
    setTriggerSource(ohboard,options.gtx,0x5,debug=options.debug)
    initialSrc = getTriggerSource(ohboard,options.gtx,debug=options.debug)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    resetLocalT1(ohboard,options.gtx,debug=options.debug)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    sys.stdout.flush()
    countersInitial      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulsesInitial = countersInitial["SENT"]["CalPulse"]
    sentL1AsInitial      = countersInitial["SENT"]["L1A"]
    time.sleep(1)
    counters      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulses = counters["SENT"]["CalPulse"]
    sentL1As      = counters["SENT"]["L1A"]
    countsExpected(sentL1AsInitial,sentL1As,sentCalPulsesInitial,sentCalPulses,0)
    setTriggerSource(ohboard,options.gtx,0x1,debug=options.debug)
    print "  T1 source: 0x%x (was 0x%x)"%(getTriggerSource(ohboard,options.gtx),initialSrc)
    sys.stdout.flush()

    resetLocalT1(ohboard,options.gtx,debug=options.debug)
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    configureLocalT1(ohboard, options.gtx, 1, 0, 40, 250, 0, options.debug)
    # configureLocalT1(ohboard, options.gtx, mode=1, t1type=0, delay=40, interval=250, number=0, debug=options.debug)
    startLocalT1(ohboard, options.gtx)
    print "  T1 MONITOR returns ",readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(options.gtx))
    # sendL1ACalPulse(ohboard,options.gtx,delay=40,interval=250,number=0,debug=options.debug)
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    resetLocalT1(ohboard,options.gtx,debug=options.debug)
    sys.stdout.flush()
    if options.debug:
        exit(1)
        pass

    # send 1000 L1As
    print "  Testing sequential L1As"
    counters = optohybridCounters(ohboard,options.gtx,doReset=True)
    countersInitial      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulsesInitial = countersInitial["SENT"]["CalPulse"]
    sentL1AsInitial      = countersInitial["SENT"]["L1A"]
    gemlogger
    gemlogger.setLevel(logging.DEBUG)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    sendL1A(ohboard,options.gtx,interval=100,number=1000,debug=options.debug)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    sys.stdout.flush()
    time.sleep(1)
    counters      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulses = counters["SENT"]["CalPulse"]
    sentL1As      = counters["SENT"]["L1A"]
    countsExpected(sentL1AsInitial,sentL1As,sentCalPulsesInitial,sentCalPulses,1000)
    countersInitial      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulsesInitial = countersInitial["SENT"]["CalPulse"]
    sentL1AsInitial      = countersInitial["SENT"]["L1A"]
    time.sleep(1)
    counters      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulses = counters["SENT"]["CalPulse"]
    sentL1As      = counters["SENT"]["L1A"]
    countsExpected(sentL1AsInitial,sentL1As,sentCalPulsesInitial,sentCalPulses,0)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    sys.stdout.flush()

    # send 1000 CalPulses+L1As
    countersInitial      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulsesInitial = countersInitial["SENT"]["CalPulse"]
    sentL1AsInitial      = countersInitial["SENT"]["L1A"]
    sendL1ACalPulse(ohboard,options.gtx,delay=0x40,interval=300,number=1000,debug=options.debug)
    time.sleep(1)
    counters      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulses = counters["SENT"]["CalPulse"]
    sentL1As      = counters["SENT"]["L1A"]
    print "  Testing sequential CalPulse+L1As"
    countsExpected(sentL1AsInitial,sentL1As,sentCalPulsesInitial,sentCalPulses,1000)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    sys.stdout.flush()

    # return to original state
    resetLocalT1(ohboard,options.gtx,debug=options.debug)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    sys.stdout.flush()
    countersInitial      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulsesInitial = countersInitial["SENT"]["CalPulse"]
    sentL1AsInitial      = countersInitial["SENT"]["L1A"]
    time.sleep(1)
    counters      = optohybridCounters(ohboard,options.gtx,debug=options.debug)["T1"]
    sentCalPulses = counters["SENT"]["CalPulse"]
    sentL1As      = counters["SENT"]["L1A"]
    print "  Testing sequential CalPulse+L1As"
    countsExpected(sentL1AsInitial,sentL1As,sentCalPulsesInitial,sentCalPulses,0)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    setTriggerSource(ohboard,options.gtx,initialSrc,debug=options.debug)
    # print "  T1 (toggle) 0x%x"%(readRegister(ohboard,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(options.gtx),options.debug))
    print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
    gemlogger.setLevel(logging.INFO)
    sys.stdout.flush()
pass

## Test the scan controller
if options.scantest:
    from gempython.tools.scan_utils_uhal import *
    print "Testing the scan controller"
    sys.stdout.flush()
    counter = 0
    if options.getdata:
        biasAllVFATs(ohboard,  options.gtx,
                     0x0,enable=False)
        writeAllVFATs(ohboard, options.gtx, "VThreshold1", 100)
        writeAllVFATs(ohboard, options.gtx, "Latency", 37)
        writeAllVFATs(ohboard, options.gtx, "ContReg0",0x37)
        writeAllVFATs(ohboard, options.gtx, "ContReg2",48)
        pass
    # run regular scans on random VFAT
    for i in range(1000):
        counter = i
        ch   = random.randint(0, 127)
        mode = random.randint(0,4)
        configureScanModule(ohboard, options.gtx,
                            mode,0,channel=ch,
                            scanmin=0,scanmax=254,
                            stepsize=1,numtrigs=100,
                            useUltra=True,
                            debug=options.debug)
        printScanConfiguration(ohboard, options.gtx,
                               useUltra=True,
                               debug=options.debug)
        sys.stdout.flush()

        if options.getdata:
            setTriggerSource(ohboard,options.gtx,1)
            configureLocalT1(ohboard,options.gtx,1,
                             0, 40, 250, 0, options.debug)
            startLocalT1(ohboard, options.gtx)
            print "  T1 Controller Status: 0x%08x"%(getLocalT1Status(ohboard,options.gtx,debug=options.debug))
            sys.stdout.flush()
            writeAllVFATs(ohboard, options.gtx, "VFATChannels.ChanReg%d"%(ch), 64)
            startScanModule(ohboard, options.gtx,
                            useUltra=True,
                            debug=options.debug)
            pass

        printScanConfiguration(ohboard, options.gtx,
                               useUltra=True,
                               debug=options.debug)
        sys.stdout.flush()

        if options.getdata:
            scanData = getUltraScanResults(ohboard, options.gtx, 255, options.debug)
            writeAllVFATs(ohboard, options.gtx, "VFATChannels.ChanReg%d"%(ch), 0)
            print "  Scan data size is %d"%(len(scanData))
            pass
        # print scanThresholdByVFAT(ohboard,options.gtx,3)
        # run ultra scans
        print "Counter is %d"%(counter)
        sys.stdout.flush()
        pass
    print "Final counter is %d"%(counter)
    sys.stdout.flush()
    pass

if options.vfatchan:
    ## Test the VFAT channels
    print "Testing the VFAT channel registers"
    validOperations = []
    for vfat in range(24):
        validOperations.append(0)
        for chan in range(128):
            initialValue = (getChannelRegister(ohboard,options.gtx,vfat,chan)&0xff)
            for j in range(0,100):
                writeData    = random.randint(0, 255)
                # writeVFAT(ohboard, options.gtx, vfat, "VFATChannels.ChanReg%d"%(chan),writeData)
                setChannelRegister(ohboard,options.gtx,vfat,chan,chanreg=writeData)
                readData     = (getChannelRegister(ohboard,options.gtx,vfat,chan)&0xff)
                if (readData == writeData):
                    validOperations[vfat] += 1
                    pass
                else:
                    msg = "vfat%d chan%d %d: 0x%02x not 0x%02x"%(vfat,chan,j,
                                                                 readData,writeData)
                    printmsg(msg,loglevel=logging.ERROR)
                    pass
                pass
            # writeVFAT(ohboard, options.gtx, vfat, "VFATChannels.ChanReg%d"%(chan), initialValue)
            setChannelRegister(ohboard,options.gtx,vfat,chan,chanreg=initialValue)
            pass
        print "VFAT%02d had %d successful operations"%(vfat,validOperations[vfat])
        sys.stdout.flush()
        pass
    pass
# exit(1)

if options.broadcastchan:
    print "Testing the VFAT registers (broadcast)"
    validOperations = []
    for vfat in range(24):
        validOperations.append(0)
    pass

    for chan in range(128):
        initialValues = []
        for vfat in range(24):
            initialValues.append(getChannelRegister(ohboard,options.gtx,vfat,chan)&0xff)
        pass
        for j in range(0,100):
            writeData    = random.randint(0, 255)
            # writeAllVFATs(ohboard, options.gtx, "VFATChannels.ChanReg%d"%(chan), writeData)
            setAllChannelRegisters(ohboard,options.gtx,chan,writeData)
            readData = getAllChannelRegisters(ohboard,options.gtx,chan,debug=options.debug)
            for vfat in range(24):
                if ((readData[vfat]&0xff) == writeData):
                    validOperations[vfat] += 1
                    pass
                else:
                    msg = "vfat%d chan%d %d: 0x%02x not 0x%02x"%(vfat,chan,j,
                                                                 readData[vfat]&0xff,writeData)
                    printmsg(msg,loglevel=logging.ERROR)
                    pass
                pass
            pass
        # for vfat in range(24):
        #     # writeVFAT(ohboard, options.gtx, vfat, "VFATChannels.ChanReg%d"%(chan), initialValue[vfat])
        #     setChannelRegister(ohboard,options.gtx,vfat,chan,chanreg=initialValues[vfat])
        #     print "VFAT%02d had %d successful operations"%(vfat,validOperations[vfat])
        #     sys.stdout.flush()
        #     pass
        pass
    for vfat in range(24):
        # writeVFAT(ohboard, options.gtx, vfat, "VFATChannels.ChanReg%d"%(chan), initialValue[vfat])
        setChannelRegister(ohboard,options.gtx,vfat,chan,chanreg=initialValues[vfat])
        printmsg("VFAT%02d had %d successful operations"%(vfat,validOperations[vfat]),
                 loglevel=logging.INFO,
                 printonly=True)
        sys.stdout.flush()
        pass
    pass

print "--=======================================--"
sys.stdout.flush()

pr.disable()
s = StringIO.StringIO()
sortby = 'cumulative'
ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
ps.print_stats()
print s.getvalue()
sys.stdout.flush()
