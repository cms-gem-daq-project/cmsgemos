import os, sys, time

from gempython.tools.optohybrid_user_functions_xhal import *
from gempython.utils.gemlogger import colormsg

import logging

class HwVFAT(object):
    def __init__(self, cardName, link, debug=False):
        """
        Initialize the HW board an open an RPC connection
        """
        # Debug flag
        self.debug = debug
         
        # Logger
        self.vfatlogger = logging.getLogger(__name__)

        # Optohybrid
        self.parentOH = HwOptoHybrid(cardName, link, debug)
        
        # Define VFAT3 Configuration
        self.confVFAT3s = self.parentOH.parentAMC.lib.configureVFAT3s
        self.confVFAT3s.argTypes = [ c_uint, c_uint ]
        self.confVFAT3s.restype = c_uint

        # Turn off calpulses
        self.stopCalPulses2AllChannels = self.parentOH.parentAMC.lib.stopCalPulse2AllChannels
        self.stopCalPulses2AllChannels.argTypes = [ c_uint, c_uint, c_uint, c_uint ]
        self.stopCalPulses2AllChannels.restype = c_uint

        # Write all channel regs
        self.setChannelRegistersVFAT3 = self.parentOH.parentAMC.lib.setChannelRegistersVFAT3
        self.setChannelRegistersVFAT3.argTypes = [ c_uint, c_uint, POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32) ]
        self.setChannelRegistersVFAT3.restype = c_uint

        # Set default parameters
        self.paramsDefVals = {}
        if self.parentOH.parentAMC.fwVersion < 3:
            self.paramsDefVals = {
                "ContReg1":    0x00,
                "ContReg2":    0x30,
                "ContReg3":    0x00,
                "Latency":      169,
                "IPreampIn":    144,
                "IPreampFeed":   69,
                "IPreampOut":   129,
                "IShaper":      130,
                "IShaperFeed":   87,
                "IComp":        101,
                "VCal":           0,
                # ["VThreshold1":   25,
                "VThreshold2": 0x00,
                "CalPhase":    0x00
                }

        return
    
    def biasAllVFATs(self, mask=0x0, enable=False):
        # HW Dependent Configuration
        if self.parentOH.parentAMC.fwVersion > 2:
            # Baseline config
            self.confVFAT3s(self.parentOH.link,mask)

            # Run mode
            if(enable):
                self.writeAllVFATs("CFG_RUN",0x1,mask)
            else:
                self.writeAllVFATs("CFG_RUN",0x0,mask)
        else:
            # Run Mode
            if (enable):
                self.writeAllVFATs("ContReg0",    0x37, mask=mask)
            else:
                #what about leaving any other settings?
                #not now, want a reproducible routine
                self.writeAllVFATs("ContReg0",    0x36, mask=mask)
                pass
        
        # User specified values - rely on the user to load self.paramsDefVals
        for key in self.paramsDefVals.keys():
            self.writeAllVFATs(key,self.paramsDefVals[key],mask)

        return

    def readAllVFATs(self, reg, mask=0x0):
        vfatVals = self.parentOH.broadcastRead(reg,mask)
        if vfatVals:
            msg = "Link %i: readAllVFATs"%(self.parentOH.link)
            for i,val in enumerate(vfatVals):
                msg+= "%d: value = 0x%08x\n"%(i,val)
                pass
            self.vfatlogger.debug(colormsg(msg,logging.DEBUG))
            pass
        return vfatVals

    def readVFAT(self, chip, reg, debug=False):
        baseNode = ""
        if self.parentOH.parentAMC.fwVersion > 2:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(self.parentOH.link,chip)
        else:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFATS.VFAT%d"%(self.parentOH.link,chip)
            pass
        vfatVal = self.parentOH.parentAMC.readRegister("%s.%s"%(baseNode,reg))
        # do check on status
        if ((vfatVal >> 26) & 0x1) :
            msg = "error on VFAT transaction (chip %d, %s)"%(chip, reg)
            self.vfatlogger.debug(colormsg(msg,logging.DEBUG))
            return -1
        elif ((vfatVal >> 25) & 0x0):
            msg = "invalid VFAT transaction (chip %d, %s)"%(chip, reg)
            self.vfatlogger.debug(colormsg(msg,logging.DEBUG))
            return -1
        elif ((vfatVal >> 24) & 0x0):
            msg = "wrong type of VFAT transaction (chip %d, %s)"%(chip, reg)
            self.vfatlogger.debug(colormsg(msg,logging.DEBUG))
            return -1
        else:
            return vfatVal

    def setChannelRegister(self, chip, chan, mask=0x0, pulse=0x0, trimARM=0x0, trimARMPol=0x0, trimZCC=0x0, trimZCCPol=0x0, debug=False):
        """
        chip - VFAT to write
        chan - channel on vfat
        mask - channel mask
        pulse - cal pulse enabled
        trimARM - v2b (v3) electronics trimDAC (arm comparator trim)
        trimARMPol - v3 electroncis only, polarity of the trimDAC for the arming comparator
        trimZCC - v3 electronics only, zero crossing comparator trim
        trimZCCPol - as trimARMPol but for the zero crossing comparator
        """

        # Invalid channel check
        if (chan not in range(0,128)):
            print "Invalid VFAT channel specified %d"%(chan)
            return

        # Write registers
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.ARM_TRIM_AMPLITUDE"%(chan), trimARM)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.ARM_TRIM_POLARITY"%(chan), trimARMPol)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.CALPULSE_ENABLE"%(chan), pulse)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.MASK"%(chan), mask)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.ZCC_TRIM_AMPLITUDE"%(chan), trimZCC)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.ZCC_TRIM_POLARITY"%(chan), trimZCCPol)
        else:
            chanReg = ((pulse&0x1) << 6)|((mask&0x1) << 5)|(trimARM&0x1f)
            self.writeVFAT(chip, "VFATChannels.ChanReg%d"%(chan),chanReg)
        return

    def setSpecificChannelAllRegisters(self, chan, chMask=0x0, pulse=0x0, trimARM=0x0, trimARMPol=0x0, trimZCC=0x0, trimZCCPol=0x0, vfatMask=0x0, debug=False):
        for vfat in range(0,self.parentOH.nVFATs):
            if (vfatMask >> vfat) & 0x1: continue
            self.setChannelRegister(vfat, chan, chMask, pulse, trimARM, trimARMPol, trimZCC, trimZCCPol, debug)
        return

    def setAllChannelRegisters(self, chMask=None, pulse=None, trimARM=None, trimARMPol=None, trimZCC=None, trimZCCPol=None, vfatMask=0x0, debug=False):
        """
        Sets all channel registers for all VFAT3s on the detector

        chMask - array with size 3072, storing channel MASK values, index goes as: idx = vfatN*128 + chan
        pulse - as chMask but for CALPULSE_ENABLE values
        trimARM - as chMask but for ARM_TRIM_AMPLITUDE values
        trimARMPol - as chMask but for ARM_TRIM_POLARITY values
        trimZCC - as chMask but for ZCC_TRIM_AMPLITUDE values
        trimZCCPol - as chMask but for ZCC_TRIM_POLARITY values
        vfatMask - 24 bit number indicating vfats to mask (n^th bit==1 means n^th VFAT ignored)
        debug - print debug information
        """

        if chMask is None:
            chMask = (c_uint32 * 3072)()
            pass
        if pulse is None:
            pulse = (c_uint32 * 3072)()
            pass
        if trimARM is None:
            trimARM = (c_uint32 * 3072)()
            pass
        if trimARMPol is None:
            trimARMPol = (c_uint32 * 3072)()
            pass
        if trimZCC is None:
            trimZCC = (c_uint32 * 3072)()
            pass
        if trimZCCPol is None:
            trimZCCPol = (c_uint32 * 3072)()
            pass

        return self.setChannelRegistersVFAT3(self.parentOH.link, vfatMask, pulse, chMask, trimARM, trimARMPol, trimZCC, trimZCCPol)

    def setDebug(self, debug):
        self.debug = debug
        self.parentOH.setDebug(debug)
        return
    
    def setRunModeAll(self, mask=0x0, enable=True, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            if (enable):
                self.writeAllVFATs("CFG_RUN",0x1,mask)
            else:
                self.writeAllVFATs("CFG_RUN",0x0,mask)
                pass
        else:
            if (enable):
                self.writeAllVFATs("ContReg0", 0x37, mask)
            else:
                self.writeAllVFATs("ContReg0", 0x36, mask)
                pass
        return

    def stopCalPulses(self, mask=0x0, chanMin=0, chanMax=128):
        """
        Turns the cal pulse off for [chanMin, chanMax] for all vfats not in mask
        v2b electronics only
        """

        return self.stopCalPulses2AllChannels(self.parentOH.link, mask, chanMin, chanMax)

    def setVFATCalHeight(self, chip, height, currentPulse=True, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            if currentPulse: #Current pulse, high CFG_CAL_DAC is a high injected charge amount
                self.writeVFATRegisters(chip,{"CFG_CAL_DAC": (height)})
            else: # Voltage pulse, low CFG_CAL_DAC is a high injected charge amount
                self.writeVFATRegisters(chip,{"CFG_CAL_DAC": (256 - height)})
        else:
            self.writeVFATRegisters(chip,{"VCal": height})
        return

    def setVFATCalHeightAll(self, mask=0x0, height=256, currentPulse=True, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            if currentPulse: #Current pulse, high CFG_CAL_DAC is a high injected charge amount
                self.writeAllVFATs("CFG_CAL_DAC", (height), mask)
            else: # Voltage pulse, low CFG_CAL_DAC is a high injected charge amount
                self.writeAllVFATs("CFG_CAL_DAC", (256 - height), mask)
        else:
            self.writeAllVFATs("VCal", height, mask)
        return

    def setVFATCalPhase(self, chip, phase, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeVFATRegisters(chip,{"CFG_CAL_PHI": phase})
        else:
            self.writeVFATRegisters(chip,{"CalPhase": phase})
            pass
        return

    def setVFATCalPhaseAll(self, mask=0x0, phase=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_CAL_PHI", phase, mask)
        else:
            self.writeAllVFATs("CalPhase", phase, mask)
            pass
        return
    
    def setVFATLatency(self, chip, lat, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeVFATRegisters(chip,{"CFG_LATENCY": lat})
        else:
            self.writeVFATRegisters(chip,{"Latency": lat})
            pass
        return

    def setVFATLatencyAll(self, mask=0x0, lat=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_LATENCY",lat,mask)
        else:
            self.writeAllVFATs("Latency",lat,mask)
            pass

        return
    
    def setVFATMSPLAll(self, mask=0x0, mspl=4, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_PULSE_STRETCH",mspl,mask)
        else:
            self.writeAllVFATs("ContReg2",((mspl-1)<<4),mask)
            pass

        return

    def setVFATThreshold(self, chip, vt1, vt2=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeVFATRegisters(chip,{"CFG_THR_ARM_DAC": vt1})
        else:
            self.writeVFATRegisters(chip,{"VThreshold1": vt1,"VThreshold2": vt2})
            pass
        return

    def setVFATThresholdAll(self, mask=0x0, vt1=100, vt2=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_THR_ARM_DAC",vt1,mask)
        else:
            self.writeAllVFATs("VThreshold1",vt1,mask)
            self.writeAllVFATs("VThreshold2",vt2,mask)
            pass

        return
    
    def writeAllVFATs(self, reg, value, mask=0x0):
        return self.parentOH.broadcastWrite(reg,value,mask)
    
    def writeVFAT(self, chip, reg, value, debug=False):
        baseNode = ""
        if self.parentOH.parentAMC.fwVersion > 2:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(self.parentOH.link,chip)
        else:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFATS.VFAT%d"%(self.parentOH.link,chip)
            pass
        self.parentOH.parentAMC.writeRegister("%s.%s"%(baseNode,reg), value)

    def writeVFATRegisters(self, chip, regs_with_values, debug=False):
        baseNode = ""
        if self.parentOH.parentAMC.fwVersion > 2:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(self.parentOH.link,chip)
        else:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFATS.VFAT%d"%(self.parentOH.link,chip)
            pass
        registers = {}
        for reg in regs_with_values.keys():
            registers["%s.%s"%(baseNode,reg)] = regs_with_values[reg]
            pass
        self.parentOH.parentAMC.writeRegisterList(registers)
