from gempython.tools.optohybrid_user_functions_xhal import *
from gempython.tools.hw_constants import gemVariants
from gempython.utils.gemlogger import colormsg

import logging

class HwVFAT(object):
    def __init__(self, cardName, link, debug=False, gemType="ge11", detType="short"):
        """
        Initialize the HW board an open an RPC connection
        """
        # Debug flag
        self.debug = debug

        # Logger
        self.vfatlogger = logging.getLogger(__name__)

        if gemType not in gemVariants.keys():
            raise OHTypeException("HwVFAT: gemType '{0}' not in the list of known gemVariants: {1}".format(gemType,gemVariants.keys()),os.EX_USAGE)

        if detType not in gemVariants[gemType]:
            raise OHTypeException("HwVFAT: detType '{0}' not in the list of known detector types for gemType {1}; list of known detector types: {2}".format(detType, gemType, gemVariants[gemType]), os.EX_USAGE)
        
        # Optohybrid
        self.parentOH = HwOptoHybrid(cardName, link, debug, gemType, detType)

        # Define VFAT3 DAC Monitoring
        self.confDacMonitor = self.parentOH.parentAMC.lib.configureVFAT3DacMonitor
        self.confDacMonitor.argTypes = [ c_uint, c_uint, c_uint ]
        self.confDacMonitor.restype = c_uint

        self.readADCs = self.parentOH.parentAMC.lib.readVFAT3ADC
        self.readADCs.argTypes = [ c_uint, POINTER(c_uint32), c_uint, c_uint ]
        self.readADCs.restype = c_uint

        # Define VFAT3 Configuration
        self.confVFAT3s = self.parentOH.parentAMC.lib.configureVFAT3s
        self.confVFAT3s.argTypes = [ c_uint, c_uint ]
        self.confVFAT3s.restype = c_uint

        # Get all channel regs
        self.getChannelRegistersVFAT3 = self.parentOH.parentAMC.lib.getChannelRegistersVFAT3
        self.getChannelRegistersVFAT3.argTypes = [ c_uint, c_uint, POINTER(c_uint32) ]
        self.getChannelRegistersVFAT3.restype = c_uint

        # Get VFAT ChipID
        self.getVFAT3ChipIDs = self.parentOH.parentAMC.lib.getVFAT3ChipIDs
        self.getVFAT3ChipIDs.argTypes = [ POINTER(c_uint32), c_uint, c_uint, c_bool ]
        self.getVFAT3ChipIDs.restype = c_uint

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
        
        # User specified values - rely on the user to load self.paramsDefVals
        for key in self.paramsDefVals.keys():
            self.writeAllVFATs(key,self.paramsDefVals[key],mask)

        return

    def configureDACMonitor(self, dacSelect, mask=0x0):
        """
        Configure the DAC Monitoring to monitor the register defined by dacSelect on all unmasked VFATs.

        dacSelect - An integer defining the monitored register.  See VFAT3 Manual GLB_CFG_CTR_4 for details.
        mask - VFAT mask
        """

        return self.confDacMonitor(self.parentOH.link, mask, dacSelect)

    def getAllChipIDs(self, mask=0x0, rawID=False):
        """
        Returns the chipIDs for all VFATs not in mask.

        mask - vfatMask, up to a 24 bit number, 1 in the N^th bit means skip this VFAT
        rawID - If true returns the rawID and does not apply the Reed-Muller decoding
        """

        chipIDData = (c_uint32 * 24)()
        
        rpcResp = self.getVFAT3ChipIDs(chipIDData, self.parentOH.link, mask, rawID)
        if rpcResp != 0:
            raise Exception("RPC response was non-zero, failed to get chipID data for OH{0}".format(self.parentOH.link))
        
        return chipIDData

    def getAllChannelRegisters(self, mask=0x0):
        chanRegData = (c_uint32 * 3072)()

        rpcResp = self.getChannelRegistersVFAT3(self.parentOH.link, mask, chanRegData)
        if rpcResp != 0:
            raise Exception("RPC response was non-zero, failed to get channel data for OH{0}".format(self.parentOH.link))
        return chanRegData

    def readAllADCs(self, adcData, useExtRefADC=False, mask=0x0):
        """
        Reads the ADC value from all unmasked VFATs

        adcData - Array of type c_uint32 of size 24
        useExtRefADC - True (False) use the externally (internally) referenced ADC
        mask - VFAT Mask
        """

        return self.readADCs(self.parentOH.link, adcData, useExtRefADC, mask)

    def readAllVFATs(self, reg, mask=0x0):
        vfatVals = self.parentOH.broadcastRead(reg,mask)
        if vfatVals:
            msg = "Link %i: readAllVFATs"%(self.parentOH.link)
            for i,val in enumerate(vfatVals):
                msg+= "%d: value = 0x%08x\n"%(i,val)
            self.vfatlogger.debug(colormsg(msg,logging.DEBUG))
        return vfatVals

    def readVFAT(self, chip, reg, debug=False):
        baseNode = ""
        if self.parentOH.parentAMC.fwVersion > 2:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(self.parentOH.link,chip)
        else:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFATS.VFAT%d"%(self.parentOH.link,chip)
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
            chanReg = (pulse<<15)+(mask<<14)+(trimZCCPol<<13)+(trimZCC<<7)+(trimARMPol<<6)+trimARM
            self.writeVFAT(chip, "VFAT_CHANNELS.CHANNEL%d"%(chan),chanReg)
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
        if pulse is None:
            pulse = (c_uint32 * 3072)()
        if trimARM is None:
            trimARM = (c_uint32 * 3072)()
        if trimARMPol is None:
            trimARMPol = (c_uint32 * 3072)()
        if trimZCC is None:
            trimZCC = (c_uint32 * 3072)()
        if trimZCCPol is None:
            trimZCCPol = (c_uint32 * 3072)()

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
        else:
            if (enable):
                self.writeAllVFATs("ContReg0", 0x37, mask)
            else:
                self.writeAllVFATs("ContReg0", 0x36, mask)
        return

    def stopCalPulses(self, mask=0x0, chanMin=0, chanMax=127):
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
        return

    def setVFATCalPhaseAll(self, mask=0x0, phase=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_CAL_PHI", phase, mask)
        else:
            self.writeAllVFATs("CalPhase", phase, mask)
        return
    
    def setVFATLatency(self, chip, lat, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeVFATRegisters(chip,{"CFG_LATENCY": lat})
        else:
            self.writeVFATRegisters(chip,{"Latency": lat})
        return

    def setVFATLatencyAll(self, mask=0x0, lat=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_LATENCY",lat,mask)
        else:
            self.writeAllVFATs("Latency",lat,mask)

        return
    
    def setVFATMSPLAll(self, mask=0x0, mspl=4, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_PULSE_STRETCH",mspl,mask)
        else:
            self.writeAllVFATs("ContReg2",((mspl-1)<<4),mask)

        return

    def setVFATThreshold(self, chip, vt1, vt2=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeVFATRegisters(chip,{"CFG_THR_ARM_DAC": vt1})
        else:
            self.writeVFATRegisters(chip,{"VThreshold1": vt1,"VThreshold2": vt2})
        return

    def setVFATThresholdAll(self, mask=0x0, vt1=100, vt2=0, debug=False):
        if self.parentOH.parentAMC.fwVersion > 2:
            self.writeAllVFATs("CFG_THR_ARM_DAC",vt1,mask)
        else:
            self.writeAllVFATs("VThreshold1",vt1,mask)
            self.writeAllVFATs("VThreshold2",vt2,mask)

        return
    
    def writeAllVFATs(self, reg, value, mask=0x0):
        return self.parentOH.broadcastWrite(reg,value,mask)
    
    def writeVFAT(self, chip, reg, value, debug=False):
        baseNode = ""
        if self.parentOH.parentAMC.fwVersion > 2:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(self.parentOH.link,chip)
        else:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFATS.VFAT%d"%(self.parentOH.link,chip)
        self.parentOH.parentAMC.writeRegister("%s.%s"%(baseNode,reg), value)

    def writeVFATRegisters(self, chip, regs_with_values, debug=False):
        baseNode = ""
        if self.parentOH.parentAMC.fwVersion > 2:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(self.parentOH.link,chip)
        else:
            baseNode = "GEM_AMC.OH.OH%d.GEB.VFATS.VFAT%d"%(self.parentOH.link,chip)
        registers = {}
        for reg in regs_with_values.keys():
            registers["%s.%s"%(baseNode,reg)] = regs_with_values[reg]
        self.parentOH.parentAMC.writeRegisterList(registers)
