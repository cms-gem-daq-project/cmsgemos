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
parser.add_option("-o", "--links", type="string", dest="activeLinks", action='append',
		  help="pair of connected optical links", metavar="activeLinks", default=[])
(options, args) = parser.parse_args()

links = {}
for link in options.activeLinks:
	pair = map(int, link.split(","))
	links[pair[0]] = pair[1]
print "links", links
#exit(1)
#links[activeLinks[0]

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

uTCAslot = 15
if options.slot:
	uTCAslot = 160+options.slot
print options.slot, uTCAslot
ipaddr        = '192.168.0.%d'%(uTCAslot)
address_table = "file://${GEM_ADDRESS_TABLE_PATH}/optohybrid_address_table.xml"
uri           = "chtcp-2.0://localhost:10203?target=%s:50001"%(ipaddr)
optohybrid    = uhal.getDevice( "optohybrid" , uri, address_table )
address_table = "file://${GEM_ADDRESS_TABLE_PATH}/glib_address_table.xml"
glib          = uhal.getDevice( "glib" , uri, address_table )
address_table = "file://${GEM_ADDRESS_TABLE_PATH}/geb_vfat_address_table.xml"
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

#for link in links.keys():
#	print "column%d:"%(link)
#	column1data = [0]
#	data1 = 0
#	print "FIFO%d I2CRecCnt = 0x%x"%(link,readRegister(glib,"GLIB.GLIB_LINKS.LINK%d.OPTICAL_LINKS.RecI2CRequests"%(link)))
#	print "FIFO%d depth     = %d"%(link,readRegister(  glib,"GLIB.GLIB_LINKS.LINK%d.TRK_FIFO.DEPTH"%(              link)))
#	column1data[0] = readRegister(glib,"GLIB.GEB.TRK_DATA.COL%d.DATA_RDY"%(link))
#	
#	#count N_L1As and send 5 if none are around
#	 
#	if not readRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.COUNTERS.L1A.Total"%(links[link])):
#		print "no L1As found, sending L1A"
#		writeRegister(optohybrid,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.L1A"%(links[link]),0x1)
#	for hit in range(6):
#		if hit == 0:
#			while ((column1data[0]&0xF000F000) != 0xA000C000):
#				print "trying to find first word"
#				column1data[0] = readRegister(glib,"GLIB.TRK_DATA.COL%d.DATA.%d"%(link,hit+1))
#		else:
#			column1data.append(readRegister(glib,"GLIB.TRK_DATA.COL%d.DATA.%d"%(link,hit+1)))
#			
#		# column1data.append(readRegister(glib,"trackingdata1"))
#		data1 = (data1<<(32*hit))+column1data[hit]
#	print "0x%x"%(data1)
#
#	myData = VFAT2TrackingData(column1data)
#	myData.printReport()
#
#exit(1)
#print
#print "column2:"
#column2data = [0]
#data2 = 0
#for link in (links.keys()):
#	print "OH link%d fw ver:0x%x"%(links[link],readRegister(glib,"OH_link%d_FW"%(links[link])))
#	writeRegister(glib,"OH_link%d_SendResync"%(links[link]),0x1)
#	writeRegister(glib,"OH_link%d_SendBC0"%(links[link]),   0x1)
#
#	writeRegister(glib,"OH_link%d_RstL1A"%(links[link]),   0x1)
#	writeRegister(glib,"OH_link%d_RstCal"%(links[link]),   0x1)
#	writeRegister(glib,"OH_link%d_RstResync"%(links[link]),0x1)
#	writeRegister(glib,"OH_link%d_RstBC0"%(links[link]),   0x1)
#
#if options.trgSrc in [0,1,2]:
#	for link in (links.keys()):
#		writeRegister(glib,"OH_link%d_TrgSrc"%(link),options.trgSrc)
#
#for link in (links.keys()):
#	print "FIFO%d I2CRecCnt: 0x%x"%(link,readRegister(glib,"GLIB_link%d_I2CRecCnt"(link)))
#	print "FIFO%d depth:       %d"%(link,readRegister(glib,"GLIB_link%d_FIFO_Occ"(link)))
#
#	print "OH link%d N L1A %d"%(links[link],readRegister(glib,"OH_link%d_ReadL1A"%(links[link])))
#	print "OH link%d N Cal %d"%(links[link],readRegister(glib,"OH_link%d_ReadCal"%(links[link])))
#	print "OH link%d N Rsy %d"%(links[link],readRegister(glib,"OH_link%d_ReadResync"%(links[link])))
#	print "OH link%d N BC0 %d"%(links[link],readRegister(glib,"OH_link%d_ReadBC0"%(links[link])))
#	print "OH link%d N BXC %d"%(links[link],readRegister(glib,"OH_link%d_ReadBX"%(links[link])))
#	
#	# writeRegister(glib,"OH_link%d_CDCE_SRC"%(links[link]),options.extclk)
#	# writeRegister(glib,"OH_link%d_CDCE_BKP"%(links[link]),options.fallback)
#	# writeRegister(glib,"OH_link%d_VFAT_SRC"%(links[link]),options.extclk)
#	# writeRegister(glib,"OH_link%d_VFAT_BKP"%(links[link]),options.fallback)
#	writeRegister(glib,"OH_link%d_SendResync"%(links[link]),0x1)
#	writeRegister(glib,"OH_link%d_SendBC0"%(links[link]),   0x1)
#
#	print "OH link%d VFAT Clock SRC %d"%(links[link],readRegister(glib,"OH_link%d_VFAT_SRC"(links[link])))
#	print "OH link%d VFAT Clock FLB %d"%(links[link],readRegister(glib,"OH_link%d_VFAT_BKP"(links[link])))
#	print "OH link%d CDCE Clock SRC %d"%(links[link],readRegister(glib,"OH_link%d_CDCE_SRC"(links[link])))
#	print "OH link%d CDCE Clock FLB %d"%(links[link],readRegister(glib,"OH_link%d_CDCE_BKP"(links[link])))
#
#time.sleep(5)
#
#for link in (links.keys()):
#	writeRegister(glib,"GLIB_link%d_FIFO_Flush"%(link),0x1)

#need to set this up for proper link awareness
# also need to be aware that GLIB link and OH link are not necessarily the same
chipIDs = {}
for chip in range(0,24):
	if (0 not in links.keys()):
		if (chip < 8):
			if options.debug:
				print "GLIB link 0 missing, skipping %d"%chip
			continue
	if (1 not in links.keys()):
		if ((chip > 7) or (chip < 16)):
			if options.debug:
				print "GLIB link 1 missing, skipping %d"%chip
			continue
	if (2 not in links.keys()):
		if (chip > 15):
			if options.debug:
				print "GLIB link 2 missing, skipping %d"%chip
			continue

				       
	regVal = readVFAT(optohybrid, chip, "ContReg0")
	if (options.multVFATs):
		writeVFAT(optohybrid, chip, "ContReg0",regVal&0xFF)
	else:
		writeVFAT(optohybrid, chip, "ContReg0",regVal&0xFE)

	regValue = (0x00 | ((options.mspl)<<4))
	writeVFAT(optohybrid, chip, "ContReg2",regValue)
	writeVFAT(optohybrid, chip, "ContReg3",0x00)
	if options.invasive:
		biasVFAT(optohybrid,"slot%d"(chip))
	writeVFAT(optohybrid, chip, "Latency",options.latency)

	chipIDs[chip]  = ((readVFAT(optohybrid, chip, "ChipID1")&0xFF)<<8)
	chipIDs[chip] |= ( readVFAT(optohybrid, chip, "ChipID0")&0xFF)

vfatForControl = "slot9"
vfatForControl = options.chip

##better to lambda this probably
## or list chip ID vertically, and various register values horizontally
base = " ChipID     ::"
perchip = "0x%04x  "
perreg  = "  0x%02x  "
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
def displayChipID(regkeys):
	return map(lambda chipID: [perchip%(regkeys[chipID])], regkeys.keys())
def displayRegVal(regkeys):
	return map(lambda chip: [perreg%(readVFAT(optohybrid,chip, key)&0xFF)], regkeys.keys())
def displayChipInfo(regkeys):
	print "%s"%base, map(lambda chipID: [perchip%(regkeys[chipID])], regkeys.keys())
	for reg in registerList:
		print "%11s::"%reg, map(lambda chip: [perreg%(readVFAT(optohybrid,chip,reg)&0xFF)], regkeys.keys())
	return
displayChipInfo(chipIDs)
#print base+display(chipIDs)
#print " ChipID     ::0x%04x  0x%04x  0x%04x  0x%04x  0x%04x  0x%04x"%(chipID1,chipID2,chipID3,
#								      chipID4,chipID5,chipID6)
#
#print " CtrlReg0   ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "ContReg0")&0xFF       ,
#print " CtrlReg1   ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "ContReg1")&0xFF       ,
#print " CtrlReg2   ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "ContReg2")&0xFF       ,
#print " CtrlReg3   ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "ContReg3")&0xFF       ,
#print " Latency    ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "Latency")&0xFF         ,
#print " IPreampIn  ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "IPreampIn"%(  "slot8"))&0xFF,
#print " IPreampFeed::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "IPreampFeed"%("slot8"))&0xFF,
#print " IPreampFeed::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "IPreampOut"%( "slot8"))&0xFF,
#print " IShaper    ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "IShaper"%(    "slot8"))&0xFF,
#print " IShaperFeed::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "IShaperFeed"%("slot8"))&0xFF,
#print " IComp      ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "IComp"%(      "slot8"))&0xFF,
#print " Ch1        ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "VFATChannels.ChanReg1"%(   "slot8"))&0xFF,
#print " Ch2        ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "VFATChannels.ChanReg2"%(   "slot8"))&0xFF,
#print " Ch10       ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "VFATChannels.ChanReg10"%(  "slot8"))&0xFF,
#print " VCal       ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "VCal"%(       "slot8"))&0xFF,
#print " VT1        ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "VThreshold1"%("slot8"))&0xFF,
#print " VT2        ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "VThreshold2"%("slot8"))&0xFF,
#print " CalPhase   ::  0x%02x    0x%02x    0x%02x    0x%02x    0x%02x    0x%02x"%(
#	readVFAT(optohybrid,%d, "CalPhase"%(   "slot8"))&0xFF,
#
print "TRACKING INFO SCRIPT UNDER DEVELOPMENT"
exit(1)
if (options.bias):
	exit(1)

#print "FIFO0 depth = %d"%readRegister(glib,"GLIB_link0_FIFO_Occ")
writeRegister(glib,"SendResync",0x1)
writeRegister(glib,"SendBC0",0x1)
time.sleep(5)
print "FIFO1 depth = %d"%readRegister(glib,"GLIB_link1_FIFO_Occ")
#print "FIFO2 depth = %d"%readRegister(glib,"GLIB_link2_FIFO_Occ")
column2data[0] = readRegister(glib,"ch2trackingdata0")
#if (not column2data[0]):
#	print "no tracking data first word found, sending test patterns"
#	writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl0",0x37)
#	writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl3",0x10)
#	time.sleep(5)
#	writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl3",0x00)
#	print "FIFO1 depth = %d (first resend)"%readRegister(glib,"GLIB_link1_FIFO_Occ")
#	column2data[0] = readRegister(glib,"ch2trackingdata0")
#	print "N L1A %d"%(readRegister(glib,"ReadL1A"))
#	print "N Cal %d"%(readRegister(glib,"ReadCal"))
#	print "N Rsy %d"%(readRegister(glib,"ReadResync"))
#	print "N BC0 %d"%(readRegister(glib,"ReadBC0"))
#	print "N BXC %d"%(readRegister(glib,"ReadBX"))
#	print "column2data[0] 0x%x"%column2data[0]
#
#if (not column2data[0]):
#	print "no tracking data first word found, sending test patterns (second time)"
#	writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl0",0x37)
#	writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl3",0x10)
#	time.sleep(5)
#	writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl3",0x00)
#	print "FIFO1 depth = %d (second resend)"%readRegister(glib,"GLIB_link1_FIFO_Occ")
#	column2data[0] = readRegister(glib,"ch2trackingdata0")
#	print "N L1A %d"%(readRegister(glib,"ReadL1A"))
#	print "N Cal %d"%(readRegister(glib,"ReadCal"))
#	print "N Rsy %d"%(readRegister(glib,"ReadResync"))
#	print "N BC0 %d"%(readRegister(glib,"ReadBC0"))
#	print "N BXC %d"%(readRegister(glib,"ReadBX"))
#	print "column2data[0] 0x%x"%column2data[0]
#
if (not column2data[0]):
	print "no tracking data first word found, sending L1As"
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl0",0x37)
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl3",0x00)
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl1",0x00)
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl2",0x50)
	print "CtrlReg0::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl0")&0xFF)
	print "CtrlReg1::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl1")&0xFF)
	print "CtrlReg2::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl2")&0xFF)
	print "CtrlReg3::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl3")&0xFF)
	print "Latency ::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_lat")  &0xFF)
	writeRegister(glib,"SendResync",0x1)
	writeRegister(glib,"SendBC0",   0x1)

	for trig in range(options.nTrigs):
		writeRegister(glib,"SendL1A",0x1)
		time.sleep(0.5)
		print "FIFO1 depth = %d (send L1As %d)"%(readRegister(glib,"GLIB_link1_FIFO_Occ"),
							 trig+1)
	time.sleep(5)
	column2data[0] = readRegister(glib,"ch2trackingdata0")
	print "N L1A %d"%(readRegister(glib,"ReadL1A"))
	print "N Cal %d"%(readRegister(glib,"ReadCal"))
	print "N Rsy %d"%(readRegister(glib,"ReadResync"))
	print "N BC0 %d"%(readRegister(glib,"ReadBC0"))
	print "N BXC %d"%(readRegister(glib,"ReadBX"))
	print "FIFO1 depth = %d"%readRegister(glib,"GLIB_link1_FIFO_Occ")
	print "column2data[0] 0x%x"%column2data[0]

if (not column2data[0]):
	print "no tracking data first word found, sending 10 L1A+CalPulse"
	time.sleep(5)
	writeRegister(glib,"GLIB_link1_FIFO_Flush",0x1)
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl0",0x37)
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl3",0x00)
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl1",0x00)
	#writeRegister(glib,"vfat2_"+vfatForControl+"_ctrl2",0x30)
	print "CtrlReg0::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl0")&0xFF)
	print "CtrlReg1::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl1")&0xFF)
	print "CtrlReg2::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl2")&0xFF)
	print "CtrlReg3::0x%x"%(readRegister(glib,"vfat2_"+vfatForControl+"_ctrl3")&0xFF)
	writeRegister(glib,"SendResync",0x1)
	writeRegister(glib,"SendBC0",   0x1)
	
	#delay = 0x04
	
	for trig in range(10):
		writeRegister(glib,"SendL1ACal",options.delay)
		time.sleep(0.1)
		print "FIFO1 depth = %d (L1A+CalPulse %d)"%(readRegister(glib,"GLIB_link1_FIFO_Occ"),
							    trig+1)
	column2data[0] = readRegister(glib,"ch2trackingdata0")
	print "N L1A %d"%(readRegister(glib,"ReadL1A"))
	print "N Cal %d"%(readRegister(glib,"ReadCal"))
	print "N Rsy %d"%(readRegister(glib,"ReadResync"))
	print "N BC0 %d"%(readRegister(glib,"ReadBC0"))
	print "N BXC %d"%(readRegister(glib,"ReadBX"))
	#time.sleep(5)
	print "FIFO1 depth = %d"%readRegister(glib,"GLIB_link1_FIFO_Occ")
	print "column2data[0] 0x%x"%column2data[0]

packet = 0
while (column2data[0]):
	print "FIFO1 depth = %d (readout)"%readRegister(glib,"GLIB_link1_FIFO_Occ")
	for hit in range(7):
		#if hit == 0:
		#	#while ((column2data[0]&0xF000F000) != 0xA000C000):
		#	#print "trying to find first word (0x%x)"%(column2data[0])
		#	column2data[0] = readRegister(glib,"ch2trackingdata%d"%(hit+1))
		#else:
		column2data.append(readRegister(glib,"ch2trackingdata%d"%(hit+1)))
		
	#for hit,word in enumerate(column2data):
	#	print "%d  0x%x"%(hit,word)

	check1 = bin((column2data[6]&0xF0000000)>>28)
	check2 = bin((column2data[6]&0x0000F000)>>12)
	check3 = bin((column2data[5]&0xF0000000)>>28)
	
	bc     = hex((0x0fff0000 & column2data[6]) >> 16)
	ec     = hex((0x00000ff0 & column2data[6]) >> 4)
	chipid = hex((0x0fff0000 & column2data[5]) >> 16)
	data1  = bin(((0x0000ffff & column2data[5]) << 16) | ((0xffff0000 & column2data[4]) >> 16))
	data2  = bin(((0x0000ffff & column2data[4]) << 16) | ((0xffff0000 & column2data[3]) >> 16))
	data3  = bin(((0x0000ffff & column2data[3]) << 16) | ((0xffff0000 & column2data[2]) >> 16))
	data4  = bin(((0x0000ffff & column2data[2]) << 16) | ((0xffff0000 & column2data[1]) >> 16))
	crc    = hex(0x0000ffff & column2data[1])
	bx     = hex(column2data[7])

	#data2 = (data2<<(32*hit))+column2data[hit]
	#print hit, "0x%08x"%(column2data[hit])

	if (packet < 20):
		# print "0x%x"%(data2)
		# myData = VFAT2TrackingData(column2data)
		#print "N L1A %d"%(readRegister(glib,"ReadL1A"))
		#print "N Cal %d"%(readRegister(glib,"ReadCal"))
		#print "N Rsy %d"%(readRegister(glib,"ReadResync"))
		#print "N BC0 %d"%(readRegister(glib,"ReadBC0"))
		#print "N BXC %d"%(readRegister(glib,"ReadBX"))
		## myData.printReport()
		#
		#print "packet6msb = %s"%(bin((0xffff0000&column2data[6])>>16))
		#print "packet6lsb = %s"%(bin((0x0000ffff&column2data[6])))
		#print "packet5msb = %s"%(bin((0xffff0000&column2data[5])>>16))
		#print "packet5lsb = %s"%(bin((0x0000ffff&column2data[5])))
		#print "packet4msb = %s"%(bin((0xffff0000&column2data[4])>>16))
		#print "packet4lsb = %s"%(bin((0x0000ffff&column2data[4])))
		#print "packet3msb = %s"%(bin((0xffff0000&column2data[3])>>16))
		#print "packet3lsb = %s"%(bin((0x0000ffff&column2data[3])))
		#print "packet2msb = %s"%(bin((0xffff0000&column2data[2])>>16))
		#print "packet2lsb = %s"%(bin((0x0000ffff&column2data[2])))
		#print "packet1msb = %s"%(bin((0xffff0000&column2data[1])>>16))
		#print "packet1lsb = %s"%(bin((0x0000ffff&column2data[1])))
		#
		print "check1 = %s"%(check1)
		print "check2 = %s"%(check2)
		print "check3 = %s"%(check3)
		print "bc     = %s"%(bc    )
		print "ec     = %s"%(ec    )
		print "chipid = %s"%(chipid)
		#print "data1  = %s"%(data1 )
		#print "data2  = %s"%(data2 )
		#print "data3  = %s"%(data3 )
		#print "data4  = %s"%(data4 )
		#print "crc    = %s"%(crc   )
		print "bx     = %s"%(bx    )
	#print "chipid = %s"%(chipid)
	column2data    = []
	column2data.append(readRegister(glib,"ch2trackingdata0"))
	packet += 1
	

##print
##print "column3:"
##column3data = [3]
##data3 = 0
##print "FIFO2 I2CRecCnt = 0x%x"%readRegister(glib,"GLIB_link2_I2CRecCnt")
##print "FIFO2 depth = %d"%readRegister(glib,"GLIB_link2_FIFO_Occ")
##column3data[0] = readRegister(glib,"ch3trackingdata0")
##for hit in range(6):
##	if hit == 0:
##		while ((column3data[0]&0xF000F000) != 0xA000C000):
##			print "trying to find first word"
##			column3data[0] = readRegister(glib,"ch3trackingdata%d"%(hit+1))
##	else:
##		column3data.append(readRegister(glib,"ch3trackingdata%d"%(hit+1)))
####for hit in range(6):
####	if hit == 0:
####		while ((column3data[0]&0xF000F000) != 0xA000C000):
####			print "trying to find first word"
####			column3data[0] = readRegister(glib,"trackingdata3")
####	else:
####		column3data.append(readRegister(glib,"trackingdata3"))
####		
####	#column3data.append(readRegister(glib,"trackingdata3"))
##	data3 = (data3<<(32*hit))+column3data[hit]
##print "0x%x"%(data3)
##
##myData = VFAT2TrackingData(column3data)
##myData.printReport()
print
print "--=======================================--"
print
