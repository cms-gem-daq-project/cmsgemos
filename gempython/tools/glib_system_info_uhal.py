import sys, os, time, signal, random
sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *

def getBoardID(glib,debug=False):
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.BOARD_ID.CHAR1",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.BOARD_ID.CHAR2",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.BOARD_ID.CHAR3",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.BOARD_ID.CHAR4",
        ]
    brd_result = readRegisterList(glib,reg_list,debug)
    board_id = ''.join([chr(brd_result[reg_list[0]]),
                        chr(brd_result[reg_list[1]]),
                        chr(brd_result[reg_list[2]]),
                        chr(brd_result[reg_list[3]])])
    return board_id

def getSystemID(glib,debug=False):
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.SYSTEM_ID.CHAR1",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.SYSTEM_ID.CHAR2",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.SYSTEM_ID.CHAR3",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.SYSTEM_ID.CHAR4",
        ]
    sys_result = readRegisterList(glib,reg_list,debug)
    system_id = ''.join([chr(sys_result[reg_list[0]]),
                         chr(sys_result[reg_list[1]]),
                         chr(sys_result[reg_list[2]]),
                         chr(sys_result[reg_list[3]])])
    return system_id

def getSystemFWRaw(glib,debug=False):
    return readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.FIRMWARE")

def getSystemFWVer(glib,debug=False):
    fwformat = "%x"
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.FIRMWARE.MAJOR",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.FIRMWARE.MINOR",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.FIRMWARE.BUILD",
        ]
    fw_result = readRegisterList(glib,reg_list,debug)
    fwver = '.'.join([fwformat%(fw_result[reg_list[0]]),
                      fwformat%(fw_result[reg_list[1]]),
                      fwformat%(fw_result[reg_list[2]])])
    return fwver

def getSystemFWDate(glib,debug=False):
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.FIRMWARE.YY",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.FIRMWARE.MM",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.FIRMWARE.DD",
        ]
    fw_result = readRegisterList(glib,reg_list,debug)
    fwdate = '/'.join([str(fw_result[reg_list[2]]),
                       str(fw_result[reg_list[1]]),
                       str(2000+fw_result[reg_list[0]])])
    return fwdate

def getSystemMACAddress(glib,debug=False):
    macformat = "%02x"
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.MAC.B5",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.MAC.B4",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.MAC.B3",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.MAC.B2",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.MAC.B1",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.MAC.B0",
        ]
    mac_result = readRegisterList(glib,reg_list,debug)
    mac_addr = ':'.join([macformat%mac_result[reg_list[5]],
                         macformat%mac_result[reg_list[4]],
                         macformat%mac_result[reg_list[3]],
                         macformat%mac_result[reg_list[2]],
                         macformat%mac_result[reg_list[1]],
                         macformat%mac_result[reg_list[0]]])
    return mac_addr

def getSystemIPAddress(glib,debug=False):
    ipformat = "%d"
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.IP_INFO.B3",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.IP_INFO.B2",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.IP_INFO.B1",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.IP_INFO.B0",
        ]
    ip_result = readRegisterList(glib,reg_list,debug)
    ip_addr = '.'.join([ipformat%ip_result[reg_list[3]],
                        ipformat%ip_result[reg_list[2]],
                        ipformat%ip_result[reg_list[1]],
                        ipformat%ip_result[reg_list[0]]])
    return ip_addr

def getSystemHWAddress(glib,debug=False):
    hwformat = "%02x"
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.HW_ID.B6",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.HW_ID.B5",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.HW_ID.B4",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.HW_ID.B3",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.HW_ID.B2",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.HW_ID.B1",
        ]
    hw_result = readRegisterList(glib,reg_list,debug)
    hw_addr = ':'.join([hwformat%hw_result[reg_list[5]],
                        hwformat%hw_result[reg_list[4]],
                        hwformat%hw_result[reg_list[3]],
                        hwformat%hw_result[reg_list[2]],
                        hwformat%hw_result[reg_list[1]],
                        hwformat%hw_result[reg_list[0]]])
    return hw_addr

def getSFPInfo(glib,sfp,debug=False):
    reg_list = [
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP1.Mod_abs",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP1.RxLOS",
        "GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP1.TxFault",
        ]
    return "-> sfp%d:  %4d     %3d    %5d"

def getAllSFPInfo(glib,debug=False):
    reg_list = []
    for sfp in range(4):
        reg_list.append("GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP%d.Mod_abs"%(sfp+1))
        reg_list.append("GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP%d.RxLOS"%(  sfp+1))
        reg_list.append("GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP%d.TxFault"%(sfp+1))
        pass

    sfp_result = readRegisterList(glib,reg_list,debug)
    sfpformat = "-> sfp%d:  %4d     %3d    %5d"
    sfp_status = "\n".join(
        "-> sfp    absent   rxlos   txfault",
        map(lambda sfp: sfpformat%(sfp,
                                   reg_result[reg_list[(sfp*3)+0]],
                                   reg_result[reg_list[(sfp*3)+1]],
                                   reg_result[reg_list[(sfp*3)+2]],
                                   ),range(4))

        )
    return sfp_status

def getBasicSystemInfo(glib,debug=False):

    #chipsLog.setLevel(logging.DEBUG)    # Verbose logging (see packets being sent and received)
    print
    print "--=======================================--"
    print "-> BOARD SYSTEM INFORMATION"
    print "--=======================================--"
    print
    print "-> board type  : %s%s%s"%(colors.CYAN,getBoardID(glib),colors.ENDC)
    print "-> system type : %s%s%s"%(colors.GREEN,getSystemID(glib),colors.ENDC)
    print "-> version nbr : %s%s%s"%(colors.MAGENTA,getSystemFWVer(glib),colors.ENDC)
    print "-> sys fw date : %s%s%s"%(colors.YELLOW,getSystemFWDate(glib),colors.ENDC)
    print "-> ip_addr     : %s"%(getSystemIPAddress(glib))
    print "-> hw_addr     : %s"%(getSystemHWAddress(glib))
    amc_slot = readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.V6_CPLD") & 0x0f

    print "-> CPLD bus state : 0x%02x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.V6_CPLD"))
    print
    if ((amc_slot>0) and (amc_slot<13)):
        print "-> amc slot #        : %d"%(amc_slot)
    else:
        print "-> amc slot #        : %d [not in crate]"%(amc_slot)
    print "-> mac address (ipb) : %s"%(getSystemMACAddress(glib))
    print "-> mac IP source     : 0x%x"%readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.MAC.IP_SOURCE")
    pass

def getExtendedSystemInfo(glib):
    print
    print "-> -----------------"
    print "-> BOARD STATUS     "
    print "-> -----------------"
    #print "-> sfp    absent   rxlos   txfault"
    print getAllSFPInfo(glib)
    print "-> ethphy interrupt  :", readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.GBE_INT")

    print "-> fmc presence     fmc1    fmc2"
    print "->                  %4d     %3d"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.FMC1_PRESENT"),
                                             readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.FMC2_PRESENT"))
    print "-> fpga reset state  :", readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.FPGA_RESET")
    print "-> cdce locked       :", readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.CDCE_LOCK")
    print
    getXpointInfo(glib)
    getSFPMonStatus(glib)

def getXpointInfo(glib):
    print "-> --------------------"
    print "-> XPOINT SWITCH STATUS"
    print "-> --------------------"
    print
    print "-> xpoint2 : S10 0x%x S11 0x%x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT2.S10"),
                                            readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT2.S11"))
    print "-> xpoint1 : S10 0x%x S11 0x%x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S10"),
                                            readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S11"))

    print "             S20 0x%x S11 0x%x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S20"),
                                            readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S21"))

    print "             S30 0x%x S31 0x%x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S30"),
                                            readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S31"))

    print "             S40 0x%x S41 0x%x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S40"),
                                            readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.CLK_CTRL.XPOINT1.S41"))

    print

def getSFPMonStatus(glib):
    print "-> -----------------"
    print "-> SFP MON STATUS   "
    print "-> -----------------"
    print
    print "-> sfp_phase_mon_done  0x%x"%(  readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP_PHASE_MON.DONE" ))
    print "-> sfp_phase_mon_ok    0x%x"%(  readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.SFP_PHASE_MON.OK"   ))
    print "-> sfp_phase_mon_ctrl  0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.SFP_PHASE_MON.CTRL"        ))
    print "-> sfp_phase_mon_lower 0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.SFP_PHASE_MON.CTRL.LOWER"  ))
    print "-> sfp_phase_mon_upper 0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.SFP_PHASE_MON.CTRL.UPPER"  ))
    print "-> sfp_phase_mon_stats 0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.SFP_PHASE_MON.STATS"       ))
    print
    print "-> -----------------"
    print "-> FMC MON STATUS   "
    print "-> -----------------"
    print
    print "-> fmc1_phase_mon_done  0x%x"%(  readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.FMC1_PHASE_MON.DONE" ))
    print "-> fmc1_phase_mon_ok    0x%x"%(  readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.STATUS.FMC1_PHASE_MON.OK"   ))
    print "-> fmc1_phase_mon_ctrl  0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.FMC1_PHASE_MON.CTRL"         ))
    print "-> fmc1_phase_mon_lower 0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.FMC1_PHASE_MON.CTRL.LOWER"   ))
    print "-> fmc1_phase_mon_upper 0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.FMC1_PHASE_MON.CTRL.UPPER"   ))
    print "-> fmc1_phase_mon_stats 0x%08x"%(readRegister(glib,"GEM_AMC.GLIB_SYSTEM.SYSTEM.FMC1_PHASE_MON.STATS"        ))
    print
    pass

def getSystemInfo(glib, full=False):
    getBasicSystemInfo(glib)
    if (full):
        getExtendedSystemInfo(glib)
