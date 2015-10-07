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
parser.add_option("-o", "--links", type="string", dest="activeLinks", action='append',
		  help="pair of connected optical links", metavar="activeLinks", default=[])
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
controlRegs = {}
chipids = []
controlReg0 = []
controlReg1 = []
controlReg2 = []
controlReg3 = []
emptyMask = 0xFFFF
thechipid = 0x0000
## these need to be done in a link specific way, so select the first available link
print "GLIB FW: 0x%08x"%(readRegister(glib,"GLIB.SYSTEM.FIRMWARE"))
print "OH   FW: 0x%08x"%(readRegister(optohybrid,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.FW"))
print "Trying to do a block read on all VFATs chipID0"
print readAllVFATs(glib, 0xf0000000, "ChipID0", options.debug)
print readAllVFATs(glib, 0xf0000000, "ChipID1", options.debug)

for control in range(4):
        controlRegs["ctrl%d"%(control)] = []

for chip in range(0,24):
	baseNode = "GLIB.OptoHybrid_0.OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
	if (0 not in links.keys()):
		if (chip < 8):
			#print "link 0 and chip %d check %d"%(chip,chip < 8)
			if options.debug:
				print "GLIB link 0 missing, skipping %d"%chip
			continue
	if (1 not in links.keys()):
		if ((chip > 7) and (chip < 16)):
			#print "link 1 and chip %d check %d,%d"%(chip,chip > 7, chip < 16)
			if options.debug:
				print "GLIB link 1 missing, skipping %d"%chip
			continue
	if (2 not in links.keys()):
		if (chip > 15):
			#print "link 2 and chip %d check %d,%d"%(chip,chip > 15, chip < 24)
			if options.debug:
				print "GLIB link 2 missing, skipping %d"%chip
			continue

	thechipid  = readRegister(optohybrid,"%s.ChipID1"%(baseNode))
	if options.debug:
		print "chip%d chipid1 0x%08x"%(chip,thechipid)

        #chipids.append(("slot%d"%(chip),thechipid))
        chipids.append(("slot%d"%(chip),getChipID(optohybrid, chip)))
        for control in range(4):
                tmp = readVFAT(optohybrid, chip, "ContReg%d"%(control))
                if (tmp>0):
                        controlRegs["ctrl%d"%(control)].append(("slot%d"%(chip),tmp&ebmask))
                else:
                        controlRegs["ctrl%d"%(control)].append(("slot%d"%(chip),0x00))
                        
        if (thechipid == 0xdead):
                if options.debug:
                        print "bad chipID, not trying to write to chip %d"%(chip)
        else:
                # # nice to only send commands to chips that appear as not 0xdead
                if options.biasAll:
                        #bias and enable
                        biasVFAT(optohybrid,chip,True)
                        #bias only
                        biasVFAT(optohybrid,chip,False)
                if options.sleepAll:
                        print "sleeping chip %d"%(chip)
                        setRunMode(optohybrid, chip, False)
                        #regVal = readVFAT(optohybrid, chip, "ContReg0")
                        #if (regVal > 0):
                        #        writeVFAT(optohybrid, chip, "ContReg0", regVal&0xFE)
                if ((options.enabledChips) and (chip in chips)):
                        print "enabling chip %d"%(chip)
                        setRunMode(optohybrid, chip, True)
                        #regVal = readVFAT(optohybrid, chip, "ContReg0")
                        #if (regVal > 0):
                        #        writeVFAT(optohybrid, chip, "ContReg0", regVal|0x01)

 
print "%6s  %6s  %02s  %02s  %02s  %02s"%("chip", "ID", "ctrl0", "ctrl1", "ctrl2", "ctrl3")
for i,chip in enumerate(chipids):
	if (int(chip[0][4:])%8==0):
		print "-------------GEB Column %d-----------------"%(int(chip[0][4:])/8)

	print "%s%6s%s  %s0x%04x%s   0x%02x   0x%02x   0x%02x   0x%02x"%(colors.GREEN,chip[0],colors.ENDC,
									 colors.CYAN,chip[1],colors.ENDC,
                                                                         controlRegs["ctrl0"][i][1],
                                                                         controlRegs["ctrl1"][i][1],
                                                                         controlRegs["ctrl2"][i][1],
                                                                         controlRegs["ctrl3"][i][1])
	

print
print "--=======================================--"
print

