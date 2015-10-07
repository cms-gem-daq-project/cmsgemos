#!/bin/env python

import sys, os, time
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
		  help="slot in uTCA crate, default is 15", metavar="slot", default=15)
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print debugging information, default is false", metavar="debug")
parser.add_option("-v", "--multiple", action="store_true", dest="multVFATs",
		  help="use multiple vfats, default is false", metavar="multVFATs")
parser.add_option("-t", "--delay", type="int", dest="delay",
		  help="delay between cal pulse and l1a", metavar="delay", default=0x4)
parser.add_option("-l", "--latency", type="int", dest="latency",
		  help="latency between l1a and packet send", metavar="latency", default=12)
parser.add_option("-m", "--mspl", type="int", dest="mspl",
		  help="length of the monostable pulse", metavar="mspl", default=0x1)
parser.add_option("-n", "--nTrigs", type="int", dest="nTrigs",
		  help="number of L1A signals to send", metavar="nTrigs", default=25)
parser.add_option("-c", "--chip", type="string", dest="chip",
		  help="vfat to control", metavar="chip", default="slot13")
parser.add_option("-f", "--fallback", type="int", dest="fallback",
		  help="allow clock fallback on both VFAT and CDCE (1) or not (0)", metavar="fallback", default=1)
parser.add_option("-k", "--extclk", type="int", dest="extclk",
		  help="use external clock source (1) or not (0)", metavar="extclk", default=0)
parser.add_option("-i", "--invasive", action="store_true", dest="invasive",
		  help="set default parameters onto the chip, default is false", metavar="invasive")
parser.add_option("-b", "--bias", action="store_true", dest="bias",
		  help="set default parameters onto all chips and then quit, default is false", metavar="bias")
parser.add_option("-e", "--extTrig", type="int", dest="trgSrc",
		  help="change trigger source", metavar="trgSrc")
(options, args) = parser.parse_args()

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

vfats         = uhal.getDevice( "vfats" , uri, address_table )

########################################
# IP address
########################################
print "--=======================================--"
print "  Opening GLIB with IP", ipaddr
print "--=======================================--"
########################################
if options.debug:
	uhal.setLogLevelTo( uhal.LogLevel.DEBUG )
getBasicSystemInfo(glib)

print "--=======================================--"
print "Fake tracking data:"
print
from trackingUnpacker import VFAT2TrackingData

#mask = 0xffabab00
mask = 0x00000000
if options.bias:
        biasAllVFATs(optohybrid, mask)
        writeAllVFATs(optohybrid, mask, "Latency", options.latency)
        regValue = (0x00 | ((options.mspl)<<4))
        writeAllVFATs(optohybrid, mask, "ContReg2",regValue)
#need to set this up for proper link awareness
# also need to be aware that GLIB link and OH link are not necessarily the same
chipIDs = {}
chipIDs["chip"] = {}
chipIDs["slot"] = {}


for chip in range(0,8):
	chipIDs["chip"][chip]  = ((readVFAT(optohybrid, chip, "ChipID1")&0xFF)<<8)
	chipIDs["chip"][chip] |= ( readVFAT(optohybrid, chip, "ChipID0")&0xFF)
        chipIDs["slot"][chip] = chip
        
slotbase = "GEB  SlotID::"
base     = "     ChipID::"
perslot  = "0x%x"
perchip  = "0x%04x"
perreg   = "0x%02x"
registerList = [
	"ContReg0",
	"ContReg1",
	"ContReg2",
	"ContReg3",
	"Latency",
	"IPreampIn",
	"IPreampFeed",
	"IPreampOut",
	"IShaper",
	"IShaperFeed",
	"IComp",
	"VCal",
	"VThreshold1",
	"VThreshold2",
	"CalPhase",
        ]

def displayChipInfo(regkeys):
        slotmap = map(lambda slotID: perslot%(regkeys["slot"][slotID]), regkeys["slot"].keys())
	print "%s   %s"%(slotbase,'    '.join(map(str, slotmap)))
	chipmap = map(lambda chipID: perchip%(regkeys["chip"][chipID]), regkeys["chip"].keys())
	print "%s%s"%(base, ' '.join(map(str, chipmap)))
	for reg in registerList:
                regmap = map(lambda chip: perreg%(readVFAT(optohybrid,chip,reg)&0xFF), regkeys["chip"].keys())
		#print "%11s::"%reg, regmap
		print "%11s::  %s"%(reg, '   '.join(map(str, regmap)))
	return
displayChipInfo(chipIDs)
print "TRACKING INFO SCRIPT UNDER DEVELOPMENT"

#writeRegister(glib,"SendResync",0x1)
#writeRegister(glib,"SendBC0",0x1)
#time.sleep(5)
fifoInfo = readFIFODepth(glib,0)
print "FIFO:  %8s  %7s  %10s"%("isEmpty",  "isFull", "depth")
print "       %8s  %7s  %10s"%("0x%x"%(fifoInfo["isEMPTY"]),
                               "0x%x"%(fifoInfo["isFULL"]),
                               "0x%x"%(fifoInfo["Occupancy"]))
packet = 0
nBlocks = int(fifoInfo["Occupancy"])/7
for block in range(nBlocks):
        fifoInfo = readFIFODepth(glib,0)
        print "FIFO:  isEmpty  isFull     depth"
        print "          0x%x   0x%x       0x%x"%(fifoInfo["isEMPTY"],fifoInfo["isFULL"],fifoInfo["Occupancy"])
        trackingPacket = readBlock(glib,"GLIB.TRK_DATA.OptoHybrid_0.FIFO",7)
        print trackingPacket
        #exit(1)
	check1 = bin((trackingPacket[5]&0xF0000000)>>28)
	check2 = bin((trackingPacket[5]&0x0000F000)>>12)
	check3 = bin((trackingPacket[4]&0xF0000000)>>28)
	
	bc     = hex((0x0fff0000 & trackingPacket[5]) >> 16)
	ec     = hex((0x00000ff0 & trackingPacket[5]) >> 4)
	chipid = hex((0x0fff0000 & trackingPacket[4]) >> 16)
	data1  = bin(((0x0000ffff & trackingPacket[4]) << 16) | ((0xffff0000 & trackingPacket[3]) >> 16))
	data2  = bin(((0x0000ffff & trackingPacket[3]) << 16) | ((0xffff0000 & trackingPacket[2]) >> 16))
	data3  = bin(((0x0000ffff & trackingPacket[2]) << 16) | ((0xffff0000 & trackingPacket[1]) >> 16))
	data4  = bin(((0x0000ffff & trackingPacket[1]) << 16) | ((0xffff0000 & trackingPacket[0]) >> 16))
	crc    = hex(0x0000ffff & trackingPacket[0])
	bx     = hex(trackingPacket[6])

        print "check1 = %s"%(check1)
        print "check2 = %s"%(check2)
        print "check3 = %s"%(check3)
        print "bc     = %s"%(bc    )
        print "ec     = %s"%(ec    )
        print "chipid = %s"%(chipid)
        # print "data1  = %s"%(data1 )
        # print "data2  = %s"%(data2 )
        # print "data3  = %s"%(data3 )
        # print "data4  = %s"%(data4 )
        # print "crc    = %s"%(crc   )
        print "bx     = %s"%(bx    )
	print "chipid = %s"%(chipid)
	

print
print "--=======================================--"
print
