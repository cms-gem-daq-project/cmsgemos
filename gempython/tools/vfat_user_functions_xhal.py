import os, signal, sys, time
sys.path.append('${GEM_PYTHON_PATH}')

from gempython.utils.nesteddict import nesteddict
from gempython.tools.optohybrid_user_functions_xhal import *
from gempython.utils.gemlogger import colors,colormsg

import logging

class HwVFAT:
    def __init__(self, slot, link, shelf=1, debug=False):
        """
        Initialize the HW board an open an RPC connection
        """
        # Debug flag
        self.debug = debug
        
        # Logger
        self.vfatlogger = logging.getLogger(__name__)

        # Optohybrid
        self.parentOH = HwOptoHybrid(slot, link, shelf, debug)
        
        # Define VFAT3 Configuration
        self.confVFAT3s = self.parentOH.parentAMC.lib.configureVFAT3s
        self.confVFAT3s.argTypes = [ c_uint, c_uint ]
        self.confVFAT3s.restype = c_uint
    
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
                pass
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
            #gemlogger.debug(msg)
            return -1
        elif ((vfatVal >> 25) & 0x0):
            msg = "invalid VFAT transaction (chip %d, %s)"%(chip, reg)
            #gemlogger.debug(msg)
            return -1
        elif ((vfatVal >> 24) & 0x0):
            msg = "wrong type of VFAT transaction (chip %d, %s)"%(chip, reg)
            #gemlogger.debug(msg)
            return -1
        else:
            return vfatVal

    def setChannelRegister(self, chip, chan, mask=0x0, pulse=0x0, trimARM=0x0, trimZCC=0x0, debug=False):
        # Invalid channel check
        if (chan not in range(0,128)):
            print "Invalid VFAT channel specified %d"%(chan)
            return

        # Write registers
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.ARM_TRIM_AMPLITUDE"%(chan+1), trimARM)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.CALPULSE_ENABLE"%(chan+1), pulse)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.MASK"%(chan+1), mask)
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d.ZCC_TRIM_AMPLITUDE"%(chan+1), trimZCC)
        else:
            chanReg = ((pulse&0x1) << 6)|((mask&0x1) << 5)|(trim&0x1f)
            self.writeVFAT(chip, "VFATChannels.ChanReg%d"%(chan+1),chanReg)
        return

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

    def setVFATLatency(self, chip, lat, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            writeVFATRegisters(chip,{"CFG_LATENCY": lat})
        else:
            writeVFATRegisters(chip,{"Latency": lat})
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
