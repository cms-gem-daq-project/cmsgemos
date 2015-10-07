import sys, os, time, signal, random
sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal
from registers_uhal import *

def readVFAT(device, chip, reg, debug=False):
    baseNode = "GLIB.OptoHybrid_0.OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    vfatVal = readRegister(device,"%s.%s"%(baseNode,reg))
    # do check on status
    if ((vfatVal >> 26) & 0x1) :
        if debug:
            print "error on VFAT transaction (chip %d, %s)"%(chip, reg)
        return -1
    elif ((vfatVal >> 25) & 0x0):
        if debug:
            print "invalid VFAT transaction (chip %d, %s)"%(chip, reg)
        return -1
    elif ((vfatVal >> 24) & 0x0):
        if debug:
            print "wrong type of VFAT transaction (chip %d, %s)"%(chip, reg)
        return -1
    else :
        return vfatVal

def readAllVFATs(device, mask, reg, debug=False):
    baseNode = "GLIB.OptoHybrid_0.OptoHybrid.GEB.Broadcast"
    writeRegister(device,"%s.Reset"%(baseNode), 0x1)
    writeRegister(device,"%s.Mask"%(baseNode), mask)
    vfatVal  = readRegister(device,"%s.Request.%s"%(baseNode,reg))
    if (debug):
        print "vfatVal = 0x%08x"%(vfatVal)
    #vfatVals = []
    #for vfat in range(24):
    #    vfatVals.append(readRegister(device,"%s.Results"%(baseNode)))
    vfatVals = readBlock(device,"%s.Results"%(baseNode),24)
    if (debug and vfatVals):
        for i,val in enumerate(vfatVals):
            print "%d: value = 0x%08x"%(i,vfatVal)
    ## do check on status
    #if ((vfatVals >> 26) & 0x1) :
    #    if debug:
    #        print "error on block VFAT transaction (%s)"%(reg)
    #    return -1
    #elif ((vfatVals >> 25) & 0x0):
    #    if debug:
    #        print "invalid block VFAT transaction (%s)"%(reg)
    #    return -1
    #elif ((vfatVals >> 24) & 0x0):
    #    if debug:
    #        print "wrong type of block VFAT transaction (%s)"%(reg)
    #    return -1
    #else :
    #    return vfatVals
    print "returning vfatVals"
    return vfatVals

def writeVFAT(device, chip, reg, value, debug=False):
    baseNode = "GLIB.OptoHybrid_0.OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    writeRegister(device,"%s.%s"%(baseNode,reg), value)

def writeAllVFATs(device, mask, reg, value, debug=False):
    baseNode = "GLIB.OptoHybrid_0.OptoHybrid.GEB.Broadcast"
    writeRegister(device,"%s.Mask"%(baseNode), mask)
    writeRegister(device,"%s.Request.%s"%(baseNode,reg), value)

def setupDefaultCRs(device,chip, sleep=False, debug=False):
    #baseNode = "GLIB.OptoHybrid_0.OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    if not sleep:
        #writeRegister(device,"%s.ContReg0"%(baseNode),  0x36)
        writeVFAT(device, chip, "ContReg0", 0x36)
    else:
        #writeRegister(device,"%s.ContReg0"%(baseNode),  0x37)
        writeVFAT(device, chip, "ContReg0", 0x37)
    #writeRegister(device,"%s.ContReg1"%(    baseNode),  0x00)
    #writeRegister(device,"%s.ContReg2"%(    baseNode),  0x30)
    #writeRegister(device,"%s.ContReg3"%(    baseNode),  0x00)
    writeVFAT(device, chip, "ContReg1", 0x00)
    writeVFAT(device, chip, "ContReg2", 0x30)
    writeVFAT(device, chip, "ContReg3", 0x00)
    return

def setRunMode(device,chip, enable, debug=False):
    regVal = readVFAT(device, chip, "ContReg0")
    if (regVal < 0):
        return
    if (enable):
        writeVFAT(device, chip, "ContReg0", regVal|0x01)
    else:
        writeVFAT(device, chip, "ContReg0", regVal&0xFE)
    return
                        
def biasVFAT(device,chip, enable=True, debug=False):
    if (enable):
        writeVFAT(device, chip, "ContReg0",    0x37)
    else:
        #what about leaving any other settings?
        #not now, want a reproducible routine
        writeVFAT(device, chip, "ContReg0",    0x36)
    writeVFAT(device, chip, "ContReg1",    0x00)
    writeVFAT(device, chip, "ContReg2",    0x30)
    writeVFAT(device, chip, "ContReg3",    0x00)
    writeVFAT(device, chip, "Latency",       20)
    writeVFAT(device, chip, "IPreampIn",    168)
    writeVFAT(device, chip, "IPreampFeed",   80)
    writeVFAT(device, chip, "IPreampOut",   150)
    writeVFAT(device, chip, "IShaper",      150)
    writeVFAT(device, chip, "IShaperFeed",  100)
    writeVFAT(device, chip, "IComp",         90)
    writeVFAT(device, chip, "VCal",         150)
    writeVFAT(device, chip, "VThreshold1",   75)
    writeVFAT(device, chip, "VThreshold2", 0x00)
    writeVFAT(device, chip, "CalPhase",    0x05)

    for chan in range(128):
        #writeRegister(device,"%s.VFATChannels.ChanReg%d"%(baseNode,chan+1),0x40)
        #mask no channels, as this seems to affect the output data packets, not just the triggers
        # disable cal pulses to all channels
        writeVFAT(device, chip, "VFATChannels.ChanReg%d"%(chan+1),0x00)
    return

def biasAllVFATs(device, mask, enable=True, debug=False):
    if (enable):
        writeAllVFATs(device, mask, "ContReg0",    0x37)
    else:
        #what about leaving any other settings?
        #not now, want a reproducible routine
        writeAllVFATs(device, mask, "ContReg0",    0x36)
    writeAllVFATs(device, mask, "ContReg1",    0x00)
    writeAllVFATs(device, mask, "ContReg2",    0x30)
    writeAllVFATs(device, mask, "ContReg3",    0x00)
    writeAllVFATs(device, mask, "Latency",       20)
    writeAllVFATs(device, mask, "IPreampIn",    168)
    writeAllVFATs(device, mask, "IPreampFeed",   80)
    writeAllVFATs(device, mask, "IPreampOut",   150)
    writeAllVFATs(device, mask, "IShaper",      150)
    writeAllVFATs(device, mask, "IShaperFeed",  100)
    writeAllVFATs(device, mask, "IComp",         90)
    writeAllVFATs(device, mask, "VCal",         150)
    writeAllVFATs(device, mask, "VThreshold1",   75)
    writeAllVFATs(device, mask, "VThreshold2", 0x00)
    writeAllVFATs(device, mask, "CalPhase",    0x05)

    for chan in range(128):
        #writeRegister(device,"%s.VFATChannels.ChanReg%d"%(baseNode,chan+1),0x40)
        #mask no channels, as this seems to affect the output data packets, not just the triggers
        # disable cal pulses to all channels
        writeAllVFATs(device, mask, "VFATChannels.ChanReg%d"%(chan+1),0x00)
    return

def getChipID(device,chip, debug=False):
    thechipid = 0x0000
    #baseNode = "GLIB.OptoHybrid_0.OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    emptyMask = 0xFFFF
    ebmask = 0x000000ff
    thechipid  = readVFAT(device, chip, "ChipID1")
    if (thechipid < 0):
        thechipid = 0xdead
    elif (((thechipid>>16)&emptyMask) != ((0x050<<4)+(chip))):
        thechipid  = (thechipid & ebmask)<<8
        thechipid |= (readVFAT(device, chip, "ChipID0") & ebmask)
    else:
        thechipid = 0xdead
    
    return thechipid
