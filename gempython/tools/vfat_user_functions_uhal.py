import sys, time, signal

from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_uhal import *
from gempython.tools.optohybrid_user_functions_uhal import *
from gempython.utils.gemlogger import colors,colormsg

import logging
vfatlogger = logging.getLogger(__name__)

def setVFATLogLevel(level):
    vfatlogger.setLevel(level)
    pass

class VFATException(Exception):
    pass

def readVFAT(device, gtx, chip, reg, debug=False):
    baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(gtx,chip)
    vfatVal = readRegister(device,"%s.%s"%(baseNode,reg),debug)
    # do check on status
    if ((vfatVal >> 26) & 0x1) :
        msg = "%s: error on VFAT transaction (chip %d, %s)"%(device,chip, reg)
        vfatlogger.warning(colormsg(msg,logging.WARNING))
        raise VFATException(colormsg(msg,logging.FATAL))
    elif ((vfatVal >> 25) & 0x0):
        msg = "%s: invalid VFAT transaction (chip %d, %s)"%(device,chip, reg)
        vfatlogger.warning(colormsg(msg,logging.WARNING))
        raise VFATException(colormsg(msg,logging.FATAL))
    elif ((vfatVal >> 24) & 0x0):
        msg = "%s: wrong type of VFAT transaction (chip %d, %s)"%(device,chip, reg)
        vfatlogger.warning(colormsg(msg,logging.WARNING))
        raise VFATException(colormsg(msg,logging.FATAL))
    else :
        return vfatVal

def readAllVFATs(device, gtx, reg, mask=0x0, debug=False):
    vfatVals = broadcastRead(device,gtx,reg,mask,debug)
    if vfatVals:
        msg = "%s: readAllVFATs"%(device)
        for i,val in enumerate(vfatVals):
            msg+= "%d: value = 0x%08x\n"%(i,val)
            pass
        vfatlogger.debug(colormsg(msg,logging.DEBUG))
        pass
    ## do check on status, maybe only do the check in the calling code
    #if ((vfatVals >> 26) & 0x1) :
    #    msg = "error on block VFAT transaction (%s)"%(reg)
    #    vfatlogger.debug(colormsg(msg,logging.DEBUG))
    #    return -1
    #elif ((vfatVals >> 25) & 0x0):
    #    msg = "invalid block VFAT transaction (%s)"%(reg)
    #    vfatlogger.debug(colormsg(msg,logging.DEBUG))
    #    return -1
    #elif ((vfatVals >> 24) & 0x0):
    #    msg = "wrong type of block VFAT transaction (%s)"%(reg)
    #    vfatlogger.debug(colormsg(msg,logging.DEBUG))
    #    return -1
    #else :
    #    return vfatVals
    return vfatVals

def writeVFAT(device, gtx, chip, reg, value, debug=False):
    baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(gtx,chip)
    writeRegister(device,"%s.%s"%(baseNode,reg), value, debug)

def writeVFATRegisters(device, gtx, chip, regs_with_values, debug=False):
    baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(gtx,chip)
    registers = nesteddict()
    for reg in regs_with_values.keys():
        registers["%s.%s"%(baseNode,reg)] = regs_with_values[reg]
        pass
    writeRegisterList(device,registers)

def readVFATRegisters(device, gtx, chip, regs, debug=False):
    baseNode = "GEM_AMC.OH.OH%d.GEB.VFAT%d"%(gtx,chip)
    registers = []
    for reg in regs:
        registers.append("%s.%s"%(baseNode,reg))
        pass
    return readRegisterList(device,registers)

def writeAllVFATs(device, gtx, reg, value, mask=0x0, debug=False):
    broadcastWrite(device,gtx,reg,value,mask,debug)

def setRunMode(device, gtx, chip, enable, debug=False):
    regVal = readVFAT(device, gtx, chip, "CFG_RUN",debug)
    if (enable):
        msg = "%s: Enabling VFAT%02d - Current CFG_RUN value 0x%02x setting to 0x%02x"%(device,chip,regVal,((0xff&regVal)|0x1))
        vfatlogger.debug(colormsg(msg,logging.DEBUG))
        writeVFAT(device, gtx, chip, "CFG_RN", ((0xff&regVal)|0x01),debug)
    else:
        msg = "%s: Disabling VFAT%02d - Current CFG_RUN value 0x%02x setting to 0x%02x"%(device,chip,regVal,((0xff&regVal)&0xfe))
        vfatlogger.debug(colormsg(msg,logging.DEBUG))
        writeVFAT(device, gtx, chip, "CFG_RUN", ((0xff&regVal)&0xfe),debug)
        pass

    msg = "%s: VFAT%02d CFG_RUN is now 0x%02x"%(device,chip,readVFAT(device, gtx, chip, "CFG_RUN",debug))
    vfatlogger.debug(colormsg(msg,logging.DEBUG))
    return

def decodeChipID(chipID):
    from subprocess import check_output
    dec = "0x{:08x}"
    try:
        retVal = check_output(["rmdecode","2","5",dec.format(chipID)])
        return int(retVal,16)
    except Exception as e:
        print("Unable to decode 0x{:08x}".format(chipID),e)
        return chipID

def getChipID(device, gtx, chip, debug=False):
    return(decodeChipID(readVFAT(device, gtx, chip, "HW_CHIP_ID")))

def getAllChipIDs(device, gtx, mask=0xff000000, debug=False):
    """Returns a map of slot number to chip ID, for chips enabled in the mask
    Currently does not only return the unmasked values, but all values
    To be fixed in a future version"""
    chipIDs = readAllVFATs(device, gtx, "HW_CHIP_ID", mask, debug)

    return dict(map(lambda slotID: (slotID, decodeChipID(chipIDs[slotID])),
                    range(0,24)))

def displayChipInfo(device, gtx, regkeys, mask=0xff000000, verbose=False, debug=False):
    """Takes as an argument a map of slot number to chip IDs and prints
    out all the information for the selected chips, would like for 0xdead
    chips to be red, but don't have time to really do this now """
    slotbase = "GEB{:d} SlotID".format(gtx)
    base     =  "     ChipID"
    perslot  = "{:6d}"
    perchip  = "{:6d}"
    perreg   = "0x{:04x}"
    perset   = "{:6d}"
    registerList = {
        perset: [
            #"CFG_{:d}".format(x) for x in range(17)
            "CFG_PULSE_STRETCH",
            "CFG_FP_FE",
            "CFG_RES_PRE",
            "CFG_CAP_PRE",
            "CFG_PT",
            "CFG_EN_HYST",
            "CFG_SEL_POL",
            "CFG_SEL_COMP_MODE",
            "CFG_IREF",
            "CFG_THR_ARM_DAC",
            "CFG_HYST",
            "CFG_LATENCY",
            "CFG_CAL_SEL_POL",
            "CFG_CAL_DAC",
            "CFG_CAL_MODE",
            "CFG_CAL_FS",
            "CFG_CAL_DUR",
            "CFG_BIAS_CFD_DAC_2",
            "CFG_BIAS_CFD_DAC_1",
            "CFG_BIAS_PRE_I_BSF",
            "CFG_BIAS_PRE_I_BIT",
            "CFG_BIAS_PRE_I_BLCC",
            "CFG_BIAS_PRE_VREF",
            "CFG_BIAS_SH_I_BFCAS",
            "CFG_BIAS_SH_I_BDIFF",
            "CFG_BIAS_SH_I_BFAMP",
            "CFG_BIAS_SD_I_BDIFF",
            "CFG_BIAS_SD_I_BSF",
            "CFG_BIAS_SD_I_BFCAS",
            "CFG_RUN",
        ],
        perreg: []
    }
    disabled = [
        "CFG_SYNC_LEVEL_MODE",
        "CFG_SELF_TRIGGER_MODE",
        "CFG_DDR_TRIGGER_MODE",
        "CFG_SPZS_SUMMARY_ONLY",
        "CFG_SPZS_MAX_PARTITIONS",
        "CFG_SPZS_ENABLE",
        "CFG_SZP_ENABLE",
        "CFG_SZD_ENABLE",
        "CFG_TIME_TAG",
        "CFG_EC_BYTES",
        "CFG_BC_BYTES",
        "CFG_FORCE_TH",
        "CFG_CAL_EXT",
    ]
    verboseRegs = [
        "CFG_FORCE_EN_ZCC",
        "CFG_VREF_ADC",
        "CFG_MON_GAIN",
        "CFG_MONITOR_SELECT",
        "CFG_THR_ZCC_DAC",
        "CFG_CAL_PHI",
    ]

    if verbose:
        registerList[perset] += verboseRegs

    MAX_HEADER = len(max(registerList[perset]+registerList[perreg],key=len))
    msghdr = "{{:{}s}}::".format(MAX_HEADER)

    slotmap = map(lambda slotID: perslot.format(slotID), regkeys.keys())
    msg = "{:s}{:s}{:s}{:s}".format(msghdr.format(slotbase),colors.GREEN,' '.join(map(str, slotmap)),colors.ENDC)

    print(msg)
    chipmap = map(lambda chipID: perchip.format(regkeys[chipID]), regkeys.keys())
    msg = "{:s}{:s}{:s}{:s}".format(msghdr.format(base),colors.CYAN,' '.join(map(str, chipmap)),colors.ENDC)

    print(msg)
    for key in registerList:
        for reg in registerList[key]:
            regValues = readAllVFATs(device, gtx, reg, mask, debug)
            regmap = map(lambda chip: key.format(chip&0xffff), regValues)
            msg = "{:s}{:s}".format(msghdr.format(reg), ' '.join(map(str, regmap)))
            print(msg)
            pass
        pass
    return
