import sys, os, time, signal, random
sys.path.append('${BUILD_HOME}/gemdaq-testing/setup/scripts')

import uhal
from registers_uhal import *

def readVFAT(device, chip, reg, debug=False):
    baseNode = "OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
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

def writeVFAT(device, chip, reg, value, debug=False):
    baseNode = "OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    writeRegister(device,"%s.%s"%(baseNode,reg), value)

def setupDefaultCRs(device,chip, sleep=False, debug=False):
    #baseNode = "OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
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

def biasVFAT(device,chip, debug=False):
    writeVFAT(device, chip, "ContReg0",    0x37)
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

def getChipID(device,chip, debug=False):
    thechipid = 0x0000
    #baseNode = "OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
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
