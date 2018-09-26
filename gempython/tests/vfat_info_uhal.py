#!/bin/env python

from gempython.tools.glib_system_info_uhal import getSystemFWVer,getSystemFWDate
from gempython.tools.vfat_user_functions_uhal import *
from gempython.tools.amc_user_functions_uhal import *
from gempython.utils.rate_calculator import errorRate

import logging
from gempython.utils.gemlogger import colors,getGEMLogger

from gempython.utils.nesteddict import nesteddict
from gempython.utils.standardopts import parser

parser.add_option("-z", "--sleep", action="store_true", dest="sleepAll",
		  help="set all chips into sleep mode", metavar="sleepAll")
parser.add_option("--bias", action="store_true", dest="biasAll",
		  help="set all chips with default bias parameters", metavar="biasAll")
parser.add_option("--enable", type="string", dest="enabledChips",
		  help="list of chips to enable, comma separated", metavar="enabledChips", default=[])
parser.add_option("--testi2c", type="int", dest="testi2c", default=-1,
		  help="Testing the I2C lines (select I2C line 0-5, or 6 for all", metavar="testi2c")
parser.add_option("--testsingle", action="store_true", dest="testsingle",
		  help="Testing single VFAT transactions (turn on every other VFAT)", metavar="testsingle")

(options, args) = parser.parse_args()

gemlogger = getGEMLogger(__name__)
gemlogger.setLevel(logging.INFO)

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

chips = []
if options.enabledChips:
    chips = [int(n) for n in options.enabledChips.split(",")]
    msg = "chips", chips
    gemlogger.info(msg)
    pass

amc     = getAMCObject(options.slot,options.shelf,options.debug)
ohboard = getOHObject(options.slot,options.gtx,options.shelf,options.debug)
setOHLogLevel(logging.INFO)
setOHLogLevel(logging.INFO)
setAMCLogLevel(logging.INFO)
setVFATLogLevel(logging.INFO)
########################################
# IP address
########################################
if options.debug and False:
    print
    print "--=======================================--"
    print "  Using AMC ", amc
    print "  Using OH ", ohboard
    print "--=======================================--"
    getSystemInfo(amc)
    msg = "The nodes within GEM_AMC are:"
    gemlogger.debug(msg)
    for inode in amc.getNode("GEM_AMC").getNodes():
        msg = inode, "attributes ..."
        gemlogger.debug(msg)
        node = amc.getNode("GEM_AMC."+inode)
        msg = "Address:0x%08x"%(node.getAddress())
        gemlogger.debug(msg)
        # Bit-mask of node
        msg = "Mask:   0x%08x"%(node.getMask())
        gemlogger.debug(msg)
        # Mode enum - one of uhal.BlockReadWriteMode.SINGLE (default), INCREMENTAL and NON_INCREMENTAL,
        #  or HIERARCHICAL for top-level nodes nesting other nodes.
        msg = "Mode:", node.getMode()
        gemlogger.debug(msg)
        # One of uhal.NodePermission.READ, WRITE and READWRITE
        msg = "R/W:", node.getPermission()
        gemlogger.debug(msg)
        # In units of 32-bits. All single registers and FIFOs have default size of 1
        msg = "Size (in units of 32-bits):", node.getSize()
        gemlogger.debug(msg)
        # User-definable string from address table - in principle a comma separated list of values
        #msg = "Tags:", node.getTags()
        #gemlogger.debug(msg)
        ## Map of user-definable, semicolon-delimited parameter=value pairs specified in the "parameters"
        ##  xml address file attribute.
        #msg = "Parameters:", node.getParameters()
        #gemlogger.debug(msg)
        pass
    pass

print
print "--=======================================--"
print

ebmask = 0x000000ff

vfats = []
for gebslot in range(24):
    vfats.append("slot%d"%(gebslot+1))

##time.sleep(5)
emptyMask = 0xFFFF
thechipid = 0x0000

print "Firmware:  %10s  %10s"%("Version","Date")
print "AMC     :  %10s  %10s"%(getSystemFWVer(amc),getSystemFWDate(amc))
print "OH      :  %10s  %10s"%(getFirmwareVersion(ohboard,options.gtx),
                               getFirmwareDateString(ohboard,options.gtx))

controls = []
chipmask = 0xff000000
if options.testi2c > -1:
    chipmask = 0xff000000
    line = options.testi2c
    while (True):
        if line < 6:
            for idx in range((line/2)*8,((line/2)+1)*8):
                if (line%2):
                    # 0  0-3
                    # 1  4-7
                    # 2  8-11
                    # 3 12-15
                    # 4 16-19
                    # 5 20-23
                    if (idx%8) > 3:
                        print "VFAT%02d: 0x%08x"%(idx,getChipID(ohboard,options.gtx,idx))
                        pass
                    pass
                else:
                    if (idx%8) < 4:
                        print "VFAT%02d: 0x%08x"%(idx,getChipID(ohboard,options.gtx,idx))
                    pass
                pass
            pass
        else:
            print getAllChipIDs(ohboard, options.gtx, chipmask)
            for idx in range(24):
                # if (idx%8) > 3:
                print "VFAT%02d: 0x%08x"%(idx,getChipID(ohboard,options.gtx,idx))
                # pass
                pass
            pass
        time.sleep(1)
        pass
    pass
msg = "Trying to do a block read on all VFATs chipID0"
gemlogger.debug(msg)

chipids = getAllChipIDs(ohboard, options.gtx, chipmask,options.debug)
msg = chipids
gemlogger.debug(msg)

if options.biasAll:
    biasAllVFATs(ohboard, options.gtx, chipmask)
    pass
if options.sleepAll:
    for chip in range(24):
        msg = "sleeping chip %d"%(chip)
        gemlogger.info(msg)
        setRunMode(ohboard, options.gtx, chip, False)
        pass
    pass
for chip in chips:
    msg = "enabling chip %d"%(chip)
    gemlogger.info(msg)
    setRunMode(ohboard, options.gtx, chip, True)
    pass

if options.testsingle:
    print "Testing single VFAT transactions"
    for vfat in range(24):
        # if (vfat%2 == 0):
        setRunMode(ohboard, options.gtx, chip=vfat, enable=False,debug=True)
        # setRunMode(ohboard, options.gtx, chip=vfat, enable=True,debug=True)
        # else:
        #     setRunMode(ohboard, options.gtx, chip=vfat, enable=False,debug=True)
        #     pass
        pass
    pass

controlRegs = nesteddict()
for control in range(4):
    controls.append(readAllVFATs(ohboard, options.gtx, "ContReg%d"%(control), 0xf0000000, options.debug))
    controlRegs["ctrl%d"%control] = dict(map(lambda chip: (chip, controls[control][chip]&0xff), range(0,24)))
    pass
displayChipInfo(ohboard, options.gtx, chipids)

print "%6s  %6s  %02s  %02s  %02s  %02s"%("chip", "ID", "ctrl0", "ctrl1", "ctrl2", "ctrl3")
for chip in chipids.keys():
    if (int(chip)%8==0):
        print "-------------GEB Column %d-----------------"%(int(chip)/8)
        pass
    print "%s%6s%s  %s0x%04x%s   0x%02x   0x%02x   0x%02x   0x%02x"%(colors.GREEN,chip,colors.ENDC,
                                                                     colors.CYAN,chipids[chip],colors.ENDC,
                                                                     controlRegs["ctrl0"][chip],
                                                                     controlRegs["ctrl1"][chip],
                                                                     controlRegs["ctrl2"][chip],
                                                                     controlRegs["ctrl3"][chip])
    pass

print
print "--=======================================--"
print

print "--===== Last BX Latency Counter =========--"
print
counts = []
# writeAllVFATs(ohboard, options.gtx, "VThreshold1", 100)
for i in range(24):
    baseNode = "GEM_AMC.OH.OH%d.COUNTERS"%(options.gtx)
    writeRegister(ohboard,"%s.VFAT%d_LAT_BX.RESET"%(baseNode,i),0x1)
    # time.sleep(0.1*(test+1))
for i in range(24):
    baseNode = "GEM_AMC.OH.OH%d.COUNTERS"%(options.gtx)
    counts.append(readRegister(ohboard,"%s.VFAT%d_LAT_BX"%(baseNode,i)))
    # print ('  '.join(map(str,map(lambda vfat: (vfat, "%d"%counts[vfat]), range(0,24)))))
for i in range(24):
    print "VFAT%02d  %d"%(i,counts[i])
