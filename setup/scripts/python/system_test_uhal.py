#!/usr/bin/env python

import sys, os, random, re
import time, datetime, os

sys.path.append('${GEM_PYTHON_PATH}')

NGTX = 2

import uhal
from registers_uhal import *
from glib_system_info_uhal import *

from glib_user_functions_uhal import *
from vfat_functions_uhal import *
from optohybrid_user_functions_uhal import *

from rate_calculator import rateConverter,errorRate

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
		  help="GTX on the GLIB", metavar="gtx", default=0)
parser.add_option("-c", "--clksrc", type="string", dest="clksrc",
		  help="select the input for the XPoint1 outputs", metavar="clksrc", default="local")
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("--testbeam", action="store_true", dest="testbeam",
		  help="fixed IP address for testbeam", metavar="testbeam")
parser.add_option("--short", action="store_true", dest="short",
		  help="Skip extended information", metavar="short")

(options, args) = parser.parse_args()

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 170
if options.slot:
	uTCAslot = 160+options.slot

if options.debug:
        print options.slot, uTCAslot

ipaddr = '192.168.0.%d'%(uTCAslot)
if options.testbeam:
    ipaddr = raw_input("> Enter the GLIB's IP address: ")

address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
uri = "chtcp-2.0://localhost:10203?target=%s:50001"%(ipaddr)
glib  = uhal.getDevice( "glib" , uri, address_table )

####################################################

print
print "This python script will test the GLIB, optical links, OH, and VFAT2 functionalities."
print "Simply follow the instructions on the screen in order to diagnose the setup."
print "Thomas Lenzi - tlenzi@ulb.ac.be"
print

####################################################

Passed = '\033[92m   > Passed... \033[0m'
Failed = '\033[91m   > Failed... \033[0m'

def txtTitle(str):
    print '\033[1m' + str + '\033[0m'

GLIB_REG_TEST = raw_input("> Number of register tests to perform on the GLIB [100]: ")
OH_REG_TEST   = raw_input("> Number of register tests to perform on the OptoHybrid [100]: ")
I2C_TEST      = raw_input("> Number of I2C tests to perform on the VFAT2s [100]: ")
TK_RD_TEST    = raw_input("> Number of tracking data packets to readout [100]: ")
RATE_WRITE    = raw_input("> Write the data to disk when testing the rate [Y/n]: ")

GLIB_REG_TEST = 100 if GLIB_REG_TEST == "" else int(GLIB_REG_TEST)
OH_REG_TEST   = 100 if OH_REG_TEST == "" else int(OH_REG_TEST)
I2C_TEST      = 100 if I2C_TEST == "" else int(I2C_TEST)
TK_RD_TEST    = 100 if TK_RD_TEST == "" else int(TK_RD_TEST)
RATE_WRITE    = False if (RATE_WRITE == "N" or RATE_WRITE == "n") else True

print

####################################################

txtTitle("A. Testing the GLIB's presence")
print "   Trying to read the GLIB board ID... If this test fails, the script will stop."

if (readRegister(glib,"GLIB.SYSTEM.BOARD_ID") != 0): print Passed
else:
    print Failed
    sys.exit()

testA = True

print

####################################################

txtTitle("B. Testing the OH's presence")
print "   Trying to set the OptoHybrid registers... If this test fails, the script will stop."

setReferenceClock(glib,options.gtx, 0x1)
setTriggerSource(glib, options.gtx, 0x1)

if (getClockingInfo(glib,options.gtx)["refclock"] == 1): print Passed
else:
    print Failed
    #sys.exit()

testB = True

print

####################################################

txtTitle("C. Testing the GLIB registers")
print "   Performing single and FIFO reads on the GLIB counters and ensuring they increment."

countersSingle = []
countersFifo = []
countersTest = True

for i in range(0, GLIB_REG_TEST):
    countersSingle.append(readRegister(glib,"GLIB.COUNTERS.IPBus.Strobe.Counters"))

for i in range(1, GLIB_REG_TEST):
    if (countersSingle[i - 1] + 1 != countersSingle[i]): countersTest = False

if (countersTest): print Passed
else: print Failed

testC = countersTest

print

####################################################

txtTitle("D. Testing the OH registers")
print "   Performing single and FIFO reads on the OptoHybrid counters and ensuring they increment."

countersSingle = []
countersFifo = []
countersTest = True

for i in range(0, OH_REG_TEST):
    countersSingle.append(readRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS.WB.MASTER.Strobe.GTX"%(options.gtx)))

for i in range(1, OH_REG_TEST):
    if (countersSingle[i - 1] + 1 != countersSingle[i]): countersTest = False

if (countersTest): print Passed
else: print Failed

testD = countersTest

print

####################################################

txtTitle("E. Detecting the VFAT2s over I2C")
print "   Detecting VFAT2s on the GEM by reading out their chip ID."

presentVFAT2sSingle = []
presentVFAT2sFifo = []

baseMask = 0xff000000

writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Reset"%(options.gtx), 0)
writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Mask"%(options.gtx), baseMask)
readRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Request.ChipID0"%(options.gtx))
chipIDs = readBlock(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Results"%(options.gtx),24)

for i in range(0, 24):
    # missing VFAT shows 0x0003XX00 in I2C broadcast result
    #                    0x05XX0800
    # XX is slot number
    # so if ((result >> 16) & 0x3) == 0x3, chip is missing
    # or if ((result) & 0x30000)   == 0x30000, chip is missing
    if (((readRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.VFATS.VFAT%d.ChipID0"%(options.gtx,i)) >> 24) & 0x5) != 0x5):
        presentVFAT2sSingle.append(i)
    if (((chipIDs[i] >> 16)  & 0x3) != 0x3):
        presentVFAT2sFifo.append(i)

if (presentVFAT2sSingle == presentVFAT2sFifo): print Passed
else: print Failed

testE = True

print "   Detected", str(len(presentVFAT2sSingle)), "VFAT2s:", str(presentVFAT2sSingle)
print

####################################################

txtTitle("F. Testing the I2C communication with the VFAT2s")
print "   Performing random read/write operation on each connect VFAT2."

testF = True

for i in presentVFAT2sSingle:
    validOperations = 0
    testReg = "GLIB.OptoHybrid_%d.OptoHybrid.GEB.VFATS.VFAT%d.ContReg3"%(options.gtx,i)
    for j in range(0, I2C_TEST):
        writeData = random.randint(0, 255)
        writeRegister(glib, testReg, writeData)
        readData = readRegister(glib,testReg) & 0xff
        if (readData == writeData): validOperations += 1
    writeRegister(glib, testReg, 0)
    if (validOperations == I2C_TEST):  print Passed, "#" + str(i)
    else:
        print Failed, "#" + str(i)
        testF = False

print

####################################################

txtTitle("G. Reading out tracking data")
print "   Sending triggers and testing if the Event Counter adds up."

writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Reset"%(options.gtx), 0)
writeAllVFATs(glib, options.gtx, baseMask, "ContReg0", 0)

testG = True
for i in presentVFAT2sSingle:

    writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.GEB.VFATS.VFAT%d.ContReg0"%(options.gtx,i), 0x37)
    writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.VFAT.MASK"%(options.gtx), (0xffffffff&(~(0x1<<i))))
    if (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.DEPTH"%(options.gtx)) > 0):
        writeRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.FLUSH"%(options.gtx), 0x1)

    nPackets = 0
    timeOut = 0
    ecs = []

    sendL1A(glib, options.gtx, 200, TK_RD_TEST)

    while (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.DEPTH"%(options.gtx)) != 7 * TK_RD_TEST) :
        timeOut += 1
        if (timeOut == 10 * TK_RD_TEST): break
        
    while (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.ISEMPTY"%(options.gtx)) != 1) :
        packets = readBlock(glib,"GLIB.TRK_DATA.OptoHybrid_%d.FIFO"%(options.gtx), 7)
        ec = int((0x00000ff0 & packets[0]) >> 4)
        nPackets += 1
        ecs.append(ec)

    writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.GEB.VFATS.VFAT%d.ContReg0"%(options.gtx,i), 0x0)

    if (nPackets != TK_RD_TEST): print Failed, "#" + str(i)
    else:
        followingECS = True
        for j in range(0, TK_RD_TEST - 1):
            if (ecs[j + 1] - ecs[j] != 1): followingECS = False
        if (followingECS): print Passed, "#" + str(i)
        else:
            print Failed, "#" + str(i)
            testG = False

print

####################################################

txtTitle("H. Reading out tracking data")
print "   Turning on all VFAT2s and looking that all the Event Counters add up."

testH = True

if (testG):
    writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Reset"%(options.gtx), 0)
    writeAllVFATs(glib, options.gtx, baseMask, "ContReg0", 0x37)

    mask = 0
    for i in presentVFAT2sSingle:
        mask |= (0x1 << i)
    writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.VFAT.MASK"%(options.gtx), 0xffffff&(~mask))

    sendResync(glib, options.gtx)

    if (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.DEPTH"%(options.gtx)) > 0):
        writeRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.FLUSH"%(options.gtx), 0x1)

    sendL1A(glib, options.gtx, 300, TK_RD_TEST)

    nPackets = 0
    timeOut = 0
    ecs = []

    while (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.DEPTH"%(options.gtx)) != len(presentVFAT2sSingle) * TK_RD_TEST) :
        timeOut += 1
        if (timeOut == 20 * TK_RD_TEST): break

    while (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.ISEMPTY"%(options.gtx)) != 1) :
        packets = readBlock(glib,"GLIB.TRK_DATA.OptoHybrid_%d.FIFO"%(options.gtx), 7)
        ec = int((0x00000ff0 & packets[0]) >> 4)
        nPackets += 1
        ecs.append(ec)

    writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Reset"%(options.gtx), 0)
    writeAllVFATs(glib, options.gtx, baseMask, "ContReg0", 0x0)

    if (nPackets != len(presentVFAT2sSingle) * TK_RD_TEST): print Failed, "#" + str(i)
    else:
        followingECS = True
        for i in range(0, TK_RD_TEST - 1):
            for j in range(0, len(presentVFAT2sSingle) - 1):
                if (ecs[i * len(presentVFAT2sSingle) + j + 1] != ecs[i * len(presentVFAT2sSingle) + j]): followingECS = False
            if (ecs[(i + 1) * len(presentVFAT2sSingle)] - ecs[i * len(presentVFAT2sSingle)] != 1): followingECS = False
        if (followingECS):
            print Passed
        else:
            print Failed
            testH = False

    writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.RESET"%(options.gtx),0x1)

else:
    print "   Skipping this test as the previous test did not succeed..."
    testH = False

print

####################################################

txtTitle("I. Testing the tracking data readout rate")
print "   Sending triggers at a given rate and looking at the maximum readout rate that can be achieved."

writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast.Reset"%(options.gtx), 0)
writeAllVFATs(glib, options.gtx, baseMask, "ContReg0", 0x0)
writeVFAT(glib, options.gtx, presentVFAT2sSingle[0], "ContReg0", 0x37)
writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.VFAT.MASK"%(options.gtx), (0xffffffff&(~(0x1<<presentVFAT2sSingle[0]))))

f = open('out.log', 'w')

values = [
          100, 200, 300, 400, 500, 600, 700, 800, 900,
          1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000,
          10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000,
          100000, 125000, 150000, 175000, 200000
        ]

previous = 0

for i in values:
    isFull = False

    if (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.DEPTH"%(options.gtx)) > 0):
        writeRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.FLUSH"%(options.gtx), 0x1)

    sendL1A(glib, options.gtx, 40000000 / i, 0)

    if (RATE_WRITE):
        for j in range(0, 1000):
            depth = readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.DEPTH"%(options.gtx))
            if (depth > 0):
                data = readBlock(glib,"GLIB.TRK_DATA.OptoHybrid_%d.FIFO"%(options.gtx), int(depth))
                for d in data: f.write(str(d))
            if (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.ISFULL"%(options.gtx)) == 1):
                isFull = True
                break
    else:
        for j in range(0, 1000):
            depth = readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.DEPTH"%(options.gtx))
            data = readBlock(glib,"GLIB.TRK_DATA.OptoHybrid_%d.FIFO"%(options.gtx), int(depth))
            if (readRegister(glib, "GLIB.TRK_DATA.OptoHybrid_%d.ISFULL"%(options.gtx)) == 1):
                isFull = True
                break

    if (isFull):
        print "   Maximum readout rate\t", str(previous), "Hz"
        break

    previous = i

    time.sleep(0.01)

f.close()

writeRegister(glib,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.RESET"%(options.gtx),0x1)
writeVFAT(glib, options.gtx, presentVFAT2sSingle[0], "ContReg0", 0x0)

testI = True

writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.VFAT.MASK"%(options.gtx), baseMask)

print

####################################################

txtTitle("J. Testing the optical link error rate")

writeRegister(glib, "GLIB.COUNTERS.GTX%d.TRK_ERR.Reset"%(options.gtx), 1)
time.sleep(1)
glib_tk_error_reg = readRegister(glib, "GLIB.COUNTERS.GTX%d.TRK_ERR"%(options.gtx))

writeRegister(glib, "GLIB.COUNTERS.GTX%d.TRG_ERR.Reset"%(options.gtx), 1)
time.sleep(1)
glib_tr_error_reg = readRegister(glib, "GLIB.COUNTERS.GTX%d.TRG_ERR"%(options.gtx))

writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS.GTX.TRK_ERR.Reset"%(options.gtx), 1)
time.sleep(1)
oh_tk_error_reg = readRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS.GTX.TRK_ERR"%(options.gtx))

writeRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS.GTX.TRG_ERR.Reset"%(options.gtx), 1)
time.sleep(1)
oh_tr_error_reg = readRegister(glib, "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS.GTX.TRG_ERR"%(options.gtx))

print "   GLIB tracking link error rate is of\t\t", str(glib_tk_error_reg), "Hz"
print "   GLIB trigger link error rate is of\t\t", str(glib_tr_error_reg), "Hz"
print "   OptoHybrid tracking link error rate is of\t", str(oh_tk_error_reg), "Hz"
print "   OptoHybrid trigger link error rate is of\t", str(oh_tr_error_reg), "Hz"

testJ = True

print

####################################################

txtTitle("K. Results")

print "   A.", (Passed if testA else Failed)
print "   B.", (Passed if testB else Failed)
print "   C.", (Passed if testC else Failed)
print "   D.", (Passed if testD else Failed)
print "   E.", (Passed if testE else Failed)
print "   F.", (Passed if testF else Failed)
print "   G.", (Passed if testG else Failed)
print "   H.", (Passed if testH else Failed)
print "   I.", (Passed if testI else Failed)
print "   J.", (Passed if testJ else Failed)

print
