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
		  help="slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
		  help="GTX on the GLIB", metavar="gtx", default=0)
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("-z", "--sleep", action="store_true", dest="sleepAll",
		  help="set all chips into sleep mode", metavar="sleepAll")
parser.add_option("-b", "--bias", action="store_true", dest="biasAll",
		  help="set all chips with default bias parameters", metavar="biasAll")
parser.add_option("-e", "--enable", type="string", dest="enabledChips",
		  help="list of chips to enable, comma separated", metavar="enabledChips", default=[])
parser.add_option("--testbeam", action="store_true", dest="testbeam",
		  help="fixed IP address for testbeam", metavar="testbeam")
parser.add_option("--testi2c", type="int", dest="testi2c", default=-1,
		  help="Testing the I2C lines (select I2C line 0-5, or 6 for all", metavar="testi2c")

(options, args) = parser.parse_args()

chips = []
if options.enabledChips:
	chips = [int(n) for n in options.enabledChips.split(",")] 
	print "chips", chips

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 10
if options.slot:
	uTCAslot = 160+options.slot
print options.slot, uTCAslot
ipaddr        = '192.168.0.%d'%(uTCAslot)
if options.testbeam:
        ipaddr        = '137.138.115.185'
uri           = "chtcp-2.0://localhost:10203?target=%s:50001"%(ipaddr)

address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
optohybrid    = uhal.getDevice( "optohybrid" , uri, address_table )

address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
glib          = uhal.getDevice( "glib"       , uri, address_table )

########################################
# IP address
########################################
if options.debug:
	print
	print "--=======================================--"
	print "  Opening GLIB with IP", ipaddr
	print "--=======================================--"
	getSystemInfo(glib)
        print "The nodes within GLIB are:"
        for inode in glib.getNode("GLIB").getNodes():
                print inode, "attributes ..."
                node = glib.getNode("GLIB."+inode)
                print "Address:0x%08x"%(node.getAddress())
                # Bit-mask of node
                print "Mask:   0x%08x"%(node.getMask())
                # Mode enum - one of uhal.BlockReadWriteMode.SINGLE (default), INCREMENTAL and NON_INCREMENTAL,
                #  or HIERARCHICAL for top-level nodes nesting other nodes.
                print "Mode:", node.getMode()
                # One of uhal.NodePermission.READ, WRITE and READWRITE
                print "R/W:", node.getPermission()
                # In units of 32-bits. All single registers and FIFOs have default size of 1
                print "Size (in units of 32-bits):", node.getSize()
                # User-definable string from address table - in principle a comma separated list of values
                #print "Tags:", node.getTags()
                ## Map of user-definable, semicolon-delimited parameter=value pairs specified in the "parameters"
                ##  xml address file attribute.
                #print "Parameters:", node.getParameters()


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
## these need to be done in a link specific way, so select the first available link
print "GLIB FW: 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.FIRMWARE"))
print "OH   FW: 0x%08x"%(readRegister(optohybrid,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.FW"%(options.gtx)))
        
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
                                                print "VFAT%02d: 0x%08x"%(idx,getChipID(glib,options.gtx,idx))
                                                pass
                                        pass
                                else:
                                        if (idx%8) < 4:
                                                print "VFAT%02d: 0x%08x"%(idx,getChipID(glib,options.gtx,idx))
                                        pass
                                pass
                        pass
                else:
                        print getAllChipIDs(glib, options.gtx, chipmask)
                        for idx in range(24):
                                # if (idx%8) > 3:
                                print "VFAT%02d: 0x%08x"%(idx,getChipID(glib,options.gtx,idx))
                                # pass
                                pass
                        pass
                time.sleep(1)
                pass
        pass
print "Trying to do a block read on all VFATs chipID0"
chipids = getAllChipIDs(glib, options.gtx, chipmask)
print chipids

if options.debug:
        print chipids
        print controlRegs
        
if options.biasAll:
        biasAllVFATs(optohybrid, options.gtx, chipmask)

if options.sleepAll:
        for chip in range(24):
                print "sleeping chip %d"%(chip)
                setRunMode(optohybrid, options.gtx, chip, False)

for chip in chips:
        print "enabling chip %d"%(chip)
        setRunMode(optohybrid, options.gtx, chip, True)
 
controlRegs = {}
for control in range(4):
        controls.append(readAllVFATs(glib, options.gtx, 0xf0000000, "ContReg%d"%(control), options.debug))
        controlRegs["ctrl%d"%control] = dict(map(lambda chip: (chip, controls[control][chip]&0xff), range(0,24)))

displayChipInfo(glib, options.gtx, chipids)

print "%6s  %6s  %02s  %02s  %02s  %02s"%("chip", "ID", "ctrl0", "ctrl1", "ctrl2", "ctrl3")
for chip in chipids.keys():
	if (int(chip)%8==0):
		print "-------------GEB Column %d-----------------"%(int(chip)/8)

	print "%s%6s%s  %s0x%04x%s   0x%02x   0x%02x   0x%02x   0x%02x"%(colors.GREEN,chip,colors.ENDC,
									 colors.CYAN,chipids[chip],colors.ENDC,
                                                                         controlRegs["ctrl0"][chip],
                                                                         controlRegs["ctrl1"][chip],
                                                                         controlRegs["ctrl2"][chip],
                                                                         controlRegs["ctrl3"][chip])
	

print
print "--=======================================--"
print

