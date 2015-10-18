#!/bin/env python

import sys, re, time, datetime, os

sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal
from registers_uhal import *
from glib_system_info_uhal import *
from vfat_functions_uhal import *
from rate_calculator import errorRate
from glib_user_functions_uhal import *

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="slot in uTCA crate", metavar="slot", default=15)
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("-z", "--sleep", action="store_true", dest="sleepAll",
		  help="set all chips into sleep mode", metavar="sleepAll")
parser.add_option("-b", "--bias", action="store_true", dest="biasAll",
		  help="set all chips with default bias parameters", metavar="biasAll")
parser.add_option("-e", "--enable", type="string", dest="enabledChips",
		  help="list of chips to enable, comma separated", metavar="enabledChips", default=[])
(options, args) = parser.parse_args()

links = {}
for link in options.activeLinks:
	pair = map(int, link.split(","))
	links[pair[0]] = pair[1]
print "links", links
chips = []
if options.enabledChips:
	chips = [int(n) for n in options.enabledChips.split(",")] 
	print "chips", chips

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 15
if options.slot:
	uTCAslot = 160+options.slot
print options.slot, uTCAslot
ipaddr        = '192.168.0.%d'%(uTCAslot)
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
print "OH   FW: 0x%08x"%(readRegister(optohybrid,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.FW"))
print "Trying to do a block read on all VFATs chipID0"
chipID0s = readAllVFATs(glib, 0xf0000000, "ChipID0", options.debug)
chipID1s = readAllVFATs(glib, 0xf0000000, "ChipID1", options.debug)

chipids = dict(map(lambda slotID: (slotID,(((chipID1s[slotID])&0xff)<<8)|(chipID0s[slotID]&0xff)), range(0,24)))
controls = []
chipmask = 0xff000000
controlRegs = {}
for control in range(4):
        controls.append(readAllVFATs(glib, 0xf0000000, "ContReg%d"%(control), options.debug))
        controlRegs["ctrl%d"%control] = dict(map(lambda chip: (chip, controls[control][chip]&0xff), range(0,24)))

if options.debug:
        print chipids
        print controlRegs
        
if options.biasAll:
        biasAllVFATs(optohybrid,mask)

if options.sleepAll:
        for chip in range(24):
                print "sleeping chip %d"%(chip)
                setRunMode(optohybrid, chip, False)

for chip in chips:
        print "enabling chip %d"%(chip)
        setRunMode(optohybrid, chip, True)
 
controlRegs = {}
for control in range(4):
        controls.append(readAllVFATs(glib, 0xf0000000, "ContReg%d"%(control), options.debug))
        controlRegs["ctrl%d"%control] = dict(map(lambda chip: (chip, controls[control][chip]&0xff), range(0,24)))

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

