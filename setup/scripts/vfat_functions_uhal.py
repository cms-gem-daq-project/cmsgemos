import sys, os, time, signal, random
sys.path.append('/opt/gemdaq/firmware/testing/src')

import uhal
from registers_uhal import *

def biasVFAT(device,chip):
    baseNode = "OptoHybrid.GEB.VFATS.VFAT%d"%(chip)
    writeRegister(device,"%s.Latency"%(    baseNode),  15)
    writeRegister(device,"%s.IPreampIn"%(  baseNode), 168)
    writeRegister(device,"%s.IPreampFeed"%(baseNode), 150)
    writeRegister(device,"%s.IPreampOut"%( baseNode),  80)
    writeRegister(device,"%s.IShaper"%(    baseNode), 150)
    writeRegister(device,"%s.IShaperFeed"%(baseNode), 100)
    writeRegister(device,"%s.IComp"%(      baseNode),  75)
    writeRegister(device,"%s.VCal"%(       baseNode), 100)
    writeRegister(device,"%s.VThreshold1"%(baseNode),  40)
    writeRegister(device,"%s.VThreshold2"%(baseNode),   0)
    writeRegister(device,"%s.CalPhase"%(   baseNode),   0)

    for chan in range(128):
        writeRegister(device,"%s.VFATChannels.ChanReg%d"%(baseNode,chan+1),0x40)
        
