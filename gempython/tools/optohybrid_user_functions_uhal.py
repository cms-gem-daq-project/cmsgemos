import sys, time, signal
sys.path.append('${GEM_PYTHON_PATH}')

from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_uhal import *
from gempython.utils.gemlogger import colormsg

import logging
ohlogger = logging.getLogger(__name__)

def setOHLogLevel(level):
    ohlogger.setLevel(level)
    pass

class scanmode:
    THRESHTRG = 0 # Threshold scan
    THRESHCH  = 1 # Threshold scan per channel
    LATENCY   = 2 # Latency scan
    SCURVE    = 3 # s-curve scan
    THRESHTRK = 4 # Threshold scan with tracking data
    pass

class triggermode:
    pass

class triggersrc:
    DAQTTC   = 0
    INTERNAL = 1
    EXTERNAL = 2
    LOOPBACK = 3
    LOGICOR  = 4
    GBTTTC   = 5
    pass

class clocksrc:
    # as of 2.2.b.b
    GBTCLK  = 0 # FW default
    HDMICLK = 1
    pass

class dacmode:
    IPREAMPIN   = 0
    IPREAMPFEED = 1
    IPREAMPOUT  = 2
    ISHAPER     = 3
    ISHAPERFEED = 4
    ICOMP       = 5
    VTHRESHOLD1 = 6
    VTHRESHOLD2 = 7
    VCAL        = 8
    CALOUT      = 9
    pass

class OptoHybridException(Exception):
    pass

def getOHObject(slot,link,shelf=1,debug=False):
    connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"
    manager         = uhal.ConnectionManager(connection_file )
    ohboard         = manager.getDevice( "gem.shelf%02d.amc%02d.optohybrid%02d"%(shelf,slot,link) )
    if checkOHBoard(ohboard):
        msg = "%s: Success!"%(ohboard)
        ohlogger.info(colormsg(msg,logging.INFO))
        return ohboard
    else:
        msg = "%s: Failed to create OptoHybrid object"%(ohboard)
        raise OptoHybridException(colormsg(msg,logging.FATAL))

def checkOHBoard(device,gtx=0,debug=False):
    # TO BE IMPLEMENTED
    return True

def getFirmwareVersionRaw(device,gtx=0,debug=False):
    """
    Returns the raw OH firmware version
    """
    baseNode = "GEM_AMC.OH.OH%d.FPGA.CONTROL"%(gtx)
    fwver = readRegister(device,"%s.RELEASE.VERSION"%(baseNode),debug)
    return fwver

def getFirmwareVersion(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d.FPGA.CONTROL"%(gtx)
    fwver = readRegister(device,"%s.RELEASE.VERSION"%(baseNode),debug)
    ver = "{3:x}.{2:x}.{1:x}.{0:x}".format(0xff&(fwver>>24),0xff&(fwver>>16),0xff&(fwver>>8),0xff&fwver)
    return ver

def getFirmwareDate(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d.FPGA.CONTROL"%(gtx)
    fwdate = readRegister(device,"%s.RELEASE.DATE"%(baseNode),debug)
    date = nesteddict()
    date["d"] = fwdate&0xff
    date["m"] = (fwdate>>8)&0xff
    date["y"] = (fwdate>>16)&0xffff
    return date

def getFirmwareDateString(device,gtx=0,old=False,debug=False):
    date = getFirmwareDate(device,gtx,debug)
    if old:
        date = getFirmwareDateOld(device,gtx,debug)
    return "%02x/%02x/%04x"%(date["d"],date["m"],date["y"])

def getFirmwareDateOld(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwdate = readRegister(device,"%s.STATUS.FW_DATE"%(baseNode),debug)
    date = nesteddict()
    date["d"] = fwdate&0xff
    date["m"] = (fwdate>>8)&0xff
    date["y"] = (fwdate>>16)&0xffff
    return date

def getConnectedVFATsMask(device,gtx=0,debug=False):
    """
    Returns the broadcast I2C mask corresponding to the connected VFATs
    """
    level = reglogger.level
    setRegisterLogLevel(logging.FATAL)
    vfatVals  = broadcastRead(device,gtx,"HW_CHIP_ID")
    bmask = 0xff000000
    if (vfatVals):
        for i,val in enumerate(vfatVals):
            msg = "%d: value = 0x%08x"%(i,val)
            ohlogger.debug(colormsg(msg,logging.DEBUG))
            if not val:
                bmask |= (0x1<<i)
            pass
        pass

    setRegisterLogLevel(level)
    return bmask

def broadcastWrite(device,gtx,register,value,mask=0xff000000,debug=False):
    """
    Perform a broadcast I2C write on the VFATs specified by mask
    Will return when operation has completed
    FIXME for V3 this is changed to a list write
    """
    baseNode = "GEM_AMC.OH.OH%d.GEB"%(gtx)
    regsToWrite = {}
    for v in range(24):
        if not ((mask >> v) & 0x1):
            regsToWrite["{:s}.VFAT{:d}.{:s}".format(baseNode,v,register)] = value
    # writeRegisterList(device, regsToWrite, debug)
    for r in regsToWrite:
        regVals.append(writeRegister(device, r, regsToWrite[r], debug))
    pass

def broadcastRead(device,gtx,register,mask=0xff000000,debug=False):
    """
    Perform a broadcast I2C read on the VFATs specified by mask
    Will return data when operation has completed
    FIXME for V3 this is changed to a list read
    """
    baseNode = "GEM_AMC.OH.OH%d.GEB"%(gtx)
    regsToRead = []
    for v in range(24):
        # if not ((mask >> v) & 0x1):
        regsToRead.append("{:s}.VFAT{:d}.{:s}".format(baseNode,v,register))

    # regVals = readRegisterList(device, regsToRead, debug)
    regVals = []
    for r in regsToRead:
        try:
            regVals.append(readRegister(device, r, debug))
        except Exception as e:
            regVals.append(None)
    return regVals

def optohybridCounters(device,gtx=0,doReset=False,debug=False):
    """
    read the optical link counters, returning a map
    if doReset is true, just send the reset command and pass
    SEM:CRITICAL,CORRECTION
    CRC:VALID,INCORRECT
    LOCAL:BX0,BXN
    TTC:L1A,BX0,BXN
    """
    baseNode = "GEM_AMC.OH.OH%d.FPGA"%(gtx)

    if doReset:
        reg_list = nesteddict()
        return
    else:
        counters = nesteddict()
        return counters

def setTriggerSource(device,gtx,source,debug=False):
    """
    Set the trigger source
    OH:   0 = TTC over GTX
          1 = FIRMWARE
          2 = EXTERNAL
          3 = LOOPBACK
          4 = LOGICAL OR
          5 = TTC over GBT
    """
    return writeRegister(device,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(gtx),source,debug)

def getTriggerSource(device,gtx,debug=False):
    """
    Get the trigger source
    OH:   0 = TTC over GTX
          1 = FIRMWARE
          2 = EXTERNAL
          3 = LOOPBACK
          4 = LOGICAL OR
          5 = TTC over GBT
    """
    return readRegister(device,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(gtx),debug)

def setTriggerThrottle(device,gtx,throttle):
    """
    Set the trigger throttle
    """
    return writeRegister(device,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.THROTTLE"%(gtx),throttle)

def getTriggerThrottle(device,gtx):
    """
    Get the trigger throttling value
    """
    return readRegister(device,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.THROTTLE"%(gtx))

def configureLocalT1(device, gtx, mode, t1type, delay, interval, number, debug=False):
    """
    Configure the T1 controller
    mode: 0 (Single T1 signal),
          1 (CalPulse followed by L1A),
          2 (pattern)
    t1type (only for mode 0, type of T1 signal to send):
          0 L1A
          1 CalPulse
          2 Resync
          3 BC0
    delay (only for mode 1), delay between CalPulse and L1A
    interval (only for mode 0,1), how often to repeat signals
    number how many signals to send (0 is continuous)
    """
    ## may be necessary, seems to help
    resetLocalT1(device,gtx,debug)

    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MODE"%(gtx),mode,debug)
    msg = "configuring the T1 controller for mode 0x%x (0x%x)"%(
        mode,
        readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MODE"%(gtx),debug))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    if (mode == 0):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx),t1type,debug)
        msg = "configuring the T1 controller for type 0x%x (0x%x)"%(
            t1type,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx),debug))
        ohlogger.debug(colormsg(msg,logging.DEBUG))
    if (mode == 1):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx),delay,debug)
        msg = "configuring the T1 controller for delay %d (%d)"%(
            delay,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx),debug))
        ohlogger.debug(colormsg(msg,logging.DEBUG))
    if (mode != 2):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx),interval,debug)
        msg = "configuring the T1 controller for interval %d (%d)"%(
            interval,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx),debug))
        ohlogger.debug(colormsg(msg,logging.DEBUG))

    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx),number,debug)
    msg = "configuring the T1 controller for nsignals %d (%d)"%(
        number,
        readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx),debug))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    return

def resetLocalT1(device,gtx,debug=False):
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    return

def getLocalT1Status(device,gtx,debug=False):
    return readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))

def startLocalT1(device,gtx,debug=False):
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def stopLocalT1(device,gtx,debug=False):
    if readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def sendL1A(device,gtx,interval=25,number=0,debug=False):
    """
    Configure the T1 controller
    Mode: 0 (Single T1 signal), 1 (CalPulse followed by L1A), 2 (pattern)
    t1type only for mode 0, type of T1 signal to send, L1A, CalPulse, Resync, BC0
    delay only for mode 1, delay between CalPulse and L1A
    interval only for mode 0,1, how often to repeat signals
    number how many signals to send (0 is continuous
    """
    msg = "%s: resetting the T1 controller"%(device)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1,debug)
    msg = "%s: configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(device,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x0,0x0,0x0,interval,number,debug)
    msg = "%s: current T1 status 0x%x"%(device,readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    return

def sendL1ACalPulse(device,gtx,delay,interval=25,number=0,debug=False):
    """
    Configure the T1 controller
    Mode: 0 (Single T1 signal), 1 (CalPulse followed by L1A), 2 (pattern)
    t1type only for mode 0, type of T1 signal to send, L1A, CalPulse, Resync, BC0
    delay only for mode 1, delay between CalPulse and L1A
    interval only for mode 0,1, how often to repeat signals
    number how many signals to send (0 is continuous
    """
    msg = "%s: resetting the T1 controller"%(device)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "%s: configuring the T1 controller for mode 0x1, delay %d, interval %d, nsignals %d"%(device,delay,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x1,0x0,delay,interval,number)
    msg = "%s: current T1 status 0x%x"%(device,readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    return

def sendResync(device,gtx,interval=25,number=1,debug=False):
    """
    Send a Resync signal
    """
    msg = "%s: resetting the T1 controller"%(device)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "%s: configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(device,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x0,0x2,0x0,interval,number)
    msg = "%s: current T1 status 0x%x"%(device,readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    return

def sendBC0(device,gtx,interval=25,number=1,debug=False):
    """
    Send a BC0 signal
    """
    msg = "%s: resetting the T1 controller"%(device)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "%s: configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(device,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x0,0x3,0x0,interval,number)
    msg = "%s: current T1 status 0x%x"%(device,readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    return

def setReferenceClock(device,gtx,source,debug=False):
    """
    Set the reference clock source on the OptoHybrid
    V2A:   0=onboard, 1=GTX recovered,  2=external clock
    V2B:   0=GBT,     1=CCB HDMI

    (Documentation above suspect, current OH FW has GBT and CCB switched,
    but as a hack)
    """
    writeRegister(device,"GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx),source)
    return

def getReferenceClock(device,gtx,debug=False):
    """
    Get the reference clock source on the OptoHybrid
    V2A:   0=onboard, 1=GTX recovered,  2=external clock
    V2B:   0=GBT,     1=CCB HDMI
    """
    return readRegister(device,"GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx))

def getClockingInfo(device,gtx,debug=False):
    """
    Get the OptoHybrid clocking information
    """
    clocking = nesteddict()

    clocking["gbtmmcmlock"]      = readRegister(device,"GEM_AMC.OH.OH%d.FPGA.CLOCKING.GBT_MMCM_LOCKED" %(gtx))
    clocking["gbtmmcmlockcnt"]   = readRegister(device,"GEM_AMC.OH.OH%d.FPGA.CLOCKING.GBT_MMCM_UNLOCKED_CNT" %(gtx))
    clocking["logicmmcmlock"]    = readRegister(device,"GEM_AMC.OH.OH%d.FPGA.CLOCKING.LOGIC_MMCM_LOCKED"%(gtx))
    clocking["logicmmcmlockcnt"] = readRegister(device,"GEM_AMC.OH.OH%d.FPGA.CLOCKING.LOGIC_MMCM_UNLOCKED_CNT"%(gtx))

    return clocking

def getVFATsBitMask(device,gtx=0,debug=False):
    """
    Returns the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.FPGA.TRIG"%(gtx)
    return readRegister(device,"%s.CTRL.VFAT_MASK"%(baseNode))

def setVFATsBitMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.FPGA.TRIG"%(gtx)
    return writeRegister(device,"%s.CTRL.VFAT_MASK"%(baseNode),mask)

def getVFATTrackingMask(device,gtx=0,debug=False):
    """
    Returns the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH_LINKS.OH%d.VFAT"%(gtx)
    return readRegister(device,"%s_MASK"%(baseNode))

def setVFATTrackingMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH_LINKS.OH%d.VFAT"%(gtx)
    return writeRegister(device,"%s_MASK"%(baseNode),mask)

def configureScanModule(device, gtx, mode, vfat, channel=0,
                        scanmin=0x0, scanmax=0xff,
                        stepsize=0x1, numtrigs=1000,
                        useUltra=False,debug=False):
    """
    Configure the firmware scan controller
    mode: 0 Threshold scan
          1 Threshold scan per channel
          2 Latency scan
          3 s-curve scan
          4 Threshold scan with tracking data
    vfat: for single VFAT scan, specify the VFAT number
          for ULTRA scan, specify the VFAT mask
    """

    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
        pass

    if (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Scan is already running (0x%x), not starting a new scan"%(device,
                                                                             readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        return

    msg = "%s: %s\n"%(device,scanBase)
    msg+= "FW scan mode       : %d\n"%(mode)
    msg+= "FW scan min        : %d\n"%(scanmin)
    msg+= "FW scan max        : %d\n"%(scanmax)
    if useUltra:
        msg+= "Ultra FW scan mask : 0x%08x"%(vfat)
    else:
        msg+= "FW scan VFAT       : %d\n"%(vfat)
        pass
    msg+= "FW scan channel    : %d\n"%(channel)
    msg+= "FW scan step size  : %d\n"%(stepsize)
    msg+= "FW scan n_triggers : %d\n"%(numtrigs)
    ohlogger.info(colormsg(msg,logging.DEBUG))

    writeRegister(device,"%s.RESET"%(scanBase),0x1)
    regList = {
        "%s.CONF.MODE"%(scanBase):  mode,
        "%s.CONF.MIN"%(scanBase):   scanmin,
        "%s.CONF.MAX"%(scanBase):   scanmax,
        "%s.CONF.CHAN"%(scanBase):  channel,
        "%s.CONF.STEP"%(scanBase):  stepsize,
        "%s.CONF.NTRIGS"%(scanBase):numtrigs
     }
    if useUltra:
        regList["%s.CONF.MASK"%(scanBase)] = vfat
    else:
        regList["%s.CONF.CHIP"%(scanBase)] = vfat
        pass

    writeRegisterList(device,regList)

    #regList = dict.fromkeys(["GEM_AMC.OH.OH%d.COUNTERS.CRC.INCORRECT.VFAT%d.Reset"%(gtx,i) for i in range(24)],1)
    #writeRegisterList(device,regList)
    # for some reason the code above doesn't work and triggers ipbus transaction errors... The code below works!
    for i in range(24):
        writeRegister(device,"GEM_AMC.OH.OH%d.COUNTERS.CRC.INCORRECT.VFAT%d.Reset"%(gtx,i), 1)
        writeRegister(device,"GEM_AMC.OH.OH%d.COUNTERS.CRC.VALID.VFAT%d.Reset"%(gtx,i), 1)

    return

def printScanConfiguration(device,gtx,useUltra=False,debug=False):
    """
    """
    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
        pass

    print scanBase
    regList = [
        "%s.CONF.MODE"%(  scanBase),
        "%s.CONF.MIN"%(   scanBase),
        "%s.CONF.MAX"%(   scanBase),
        "%s.CONF.CHAN"%(  scanBase),
        "%s.CONF.STEP"%(  scanBase),
        "%s.CONF.NTRIGS"%(scanBase),
        "%s.MONITOR"%(    scanBase),
     ]
    if useUltra:
        regList.append("%s.CONF.MASK"%(scanBase))
    else:
        regList.append("%s.CONF.CHIP"%(scanBase))
        pass

    if debug:
        regParams = nesteddict()
        for reg in regList:
            regParams[reg] = {
                "Path":       "%s"%(  device.getNode(reg).getPath()),
                "Address":    "0x%x"%(device.getNode(reg).getAddress()),
                "Mask":       "0x%x"%(device.getNode(reg).getMask()),
                "Permission": "%s"%(  device.getNode(reg).getPermission()),
                "Mode":       "%s"%(  device.getNode(reg).getMode()),
                "Size":       "%s"%(  device.getNode(reg).getSize())
                }
            print regParams[reg]
            pass
        pass
    regVals = readRegisterList(device,regList)
    print "FW scan mode       : %d"%(regVals["%s.CONF.MODE"%(scanBase)])
    print "FW scan min        : %d"%(regVals["%s.CONF.MIN"%(scanBase)])
    print "FW scan max        : %d"%(regVals["%s.CONF.MAX"%(scanBase)])
    if useUltra:
        print "Ultra FW scan mask : 0x%08x"%(regVals["%s.CONF.MASK"%(scanBase)])
    else:
        print "FW scan VFAT       : %d"%(regVals["%s.CONF.CHIP"%(scanBase)])
        pass
    print "FW scan channel    : %d"%(regVals["%s.CONF.CHAN"%(scanBase)])
    print "FW scan step size  : %d"%(regVals["%s.CONF.STEP"%(scanBase)])
    print "FW scan n_triggers : %d"%(regVals["%s.CONF.NTRIGS"%(scanBase)])
    print "FW scan status     : 0x%08x"%(readRegister(device,"%s.MONITOR"%(scanBase)))

    return

def startScanModule(device, gtx, useUltra=False,debug=False):
    """
    """

    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
        pass

    if (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Scan is already running (0x%x), not starting a new scan"%(device,
                                                                             readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        return
    if (readRegister(device,"%s.MONITOR.ERROR"%(scanBase)) > 0):
        msg = "%s: There was an error in the scan configuration, not starting a new scan"%(device)
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))

    writeRegister(device,"%s.START"%(scanBase),0x1)
    if readRegister(device,"%s.MONITOR.ERROR"%(scanBase)) or not readRegister(device,"%s.MONITOR.STATUS"%(scanBase)):
        msg = "%s: Scan failed to start"%(device)
        msg += "ERROR  %d"%(readRegister(device,"%s.MONITOR.ERROR"%(scanBase)))
        msg += "STATUS %d"%(not readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    msg = "%s: After start, scan status is: 0x%08x"%(device,readRegister(device,"%s.MONITOR"%(scanBase)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    return

def getScanResults(device, gtx, numpoints, debug=False):
    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    while (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Scan still running (0x%x), not returning results"%(device,readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        ohlogger.debug(colormsg(msg,logging.DEBUG))
        time.sleep(0.1)
        pass

    msg = "%s::getScanResults\n"%(device)
    msg+= "Scan status (0x%08x)\n"%(readRegister(device,"%s.MONITOR"%(scanBase)))
    msg+= "Scan results available (0x%06x)"%(readRegister(device,"%s.MONITOR.READY"%(scanBase)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    results = readBlock(device,"%s.RESULTS"%(scanBase),numpoints)

    return results

def getUltraScanResults(device, gtx, numpoints, debug=False):
    scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
    ohnL1A_0 = getL1ACount(device,gtx)
    ohnL1A = getL1ACount(device,gtx)
    numtrigs = readRegister(device,"%s.CONF.NTRIGS"%(scanBase))
    if (readRegister(device,"%s.CONF.MODE"%(scanBase))==2):
        isLatency = True
        print "At link %s: %d/%d L1As processed, %d%% done" %(gtx, getL1ACount(device,gtx)-ohnL1A_0, numpoints*numtrigs, (getL1ACount(device,gtx)-ohnL1A_0)*100./(numpoints*numtrigs))
    else:
        isLatency = False
    while (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Ultra scan still running (0x%x), not returning results"%(device,
                                                                            readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        ohlogger.debug(colormsg(msg,logging.DEBUG))
        if (isLatency):
            if ((getL1ACount(device,gtx)-ohnL1A) > numtrigs):
                print "At link %s: %d/%d L1As processed, %d%% done" %(gtx, getL1ACount(device,gtx)-ohnL1A_0, numpoints*numtrigs, (getL1ACount(device,gtx)-ohnL1A_0)*100./(numpoints*numtrigs))
                ohnL1A = getL1ACount(device,gtx)
        else:
            pass
        time.sleep(0.1)
        pass

    msg = "%s::getScanResults\n"%(device)
    msg = "Ultra scan status (0x%08x)\n"%(         readRegister(device,"%s.MONITOR"%(      scanBase)))
    msg+= "Ultra scan results available (0x%06x)"%(readRegister(device,"%s.MONITOR.READY"%(scanBase)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    results = []

    for chip in range(24):
        results.append(readBlock(device,"%s.RESULTS.VFAT%d"%(scanBase,chip),numpoints))

    return results

def getADCValue(device, gtx, adc, debug=False):
    adcBase = "GEM_AMC.OH.OH%d.ADC"%(gtx)
    adcinfo = readRegister(device,"%s.%s"%(adcBase,adc))
    return adcinfo

def getADCTemperature(device, gtx, debug=False):
    return getADCValue(device,gtx,"TEMP",debug)

def getADCVCCInt(device, gtx, debug=False):
    return getADCValue(device,gtx,"VCCINT",debug)

def getADCVCCAux(device, gtx, debug=False):
    return getADCValue(device,gtx,"VCCAUX",debug)

def getVFATADCVoltage(device, gtx, column, debug=False):
    return getADCValue(device,gtx,"VCCINT",debug)

def getVFATADCCurrent(device, gtx, column, debug=False):
    return getADCValue(device,gtx,"VCCAUX",debug)

def printSysmonInfo(device, gtx, debug=False):
    adcBase = "GEM_AMC.OH.OH%d.ADC"%(gtx)

    regList = ["TEMP", "VCCINT", "VCCAUX",
               "TEMP.MAX", "VCCINT.MAX", "VCCAUX.MAX",
               "TEMP.MIN", "VCCINT.MIN", "VCCAUX.MIN"]
    regs = []
    for reg in regList:
        regs.append("%s.%s"%(adcBase,reg))
        pass
    res = readRegisterList(device,regs)

    def TEMP_CONV(daccode):
        return daccode*0.49-273.15
    def VCC_CONV(daccode):
        return daccode*2.93/1000

    print "OptoHybrid ADC sysmon"
    print "         %7s  %8s  %8s"%("Temp", "VCCINT", "VCCAUX")
    print "Current: %3.2f C  %1.4f V  %1.4f V"%(TEMP_CONV((res["%s.TEMP"%(adcBase)     ]>>6)&0x3ff),
                                                VCC_CONV((res["%s.VCCINT"%(adcBase)    ]>>6)&0x3ff),
                                                VCC_CONV((res["%s.VCCAUX"%(adcBase)    ]>>6)&0x3ff))
    print "Max:     %3.2f C  %1.4f V  %1.4f V"%(TEMP_CONV((res["%s.TEMP.MAX"%(adcBase) ]>>6)&0x3ff),
                                                VCC_CONV((res["%s.VCCINT.MAX"%(adcBase)]>>6)&0x3ff),
                                                VCC_CONV((res["%s.VCCAUX.MAX"%(adcBase)]>>6)&0x3ff))
    print "Min:     %3.2f C  %1.4f V  %1.4f V"%(TEMP_CONV((res["%s.TEMP.MIN"%(adcBase) ]>>6)&0x3ff),
                                                VCC_CONV((res["%s.VCCINT.MIN"%(adcBase)]>>6)&0x3ff),
                                                VCC_CONV((res["%s.VCCAUX.MIN"%(adcBase)]>>6)&0x3ff))
    return

def calculateLinkErrors(device,gtx,sampleTime):
    baseNode = "GEM_AMC.OH_LINKS"
    errorCounts = nesteddict()

    # writeRegister(device,"%s.CTRL.CNT_RESET"%(baseNode),0x1)
    first  = readRegister(device,"%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
    time.sleep(sampleTime)
    second = readRegister(device,"%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
    errorCounts["LINK"] = [first,second]

    baseNode = "GEM_AMC.OH"
    for link in ("GBT","GTX"):
        writeRegister(device,"%s.OH%d.COUNTERS.%s_LINK.TRK_ERR.Reset"%(baseNode,gtx,link),0x1)
        first = readRegister(device,"%s.OH%d.COUNTERS.%s_LINK.TRK_ERR"%(baseNode,gtx,link))
        time.sleep(sampleTime)
        second = readRegister(device,"%s.OH%d.COUNTERS.%s_LINK.TRK_ERR"%(baseNode,gtx,link))
        errorCounts[link] = [first,second]
    return errorCounts

def getL1ACount(device,link):
    return readRegister(device, "GEM_AMC.OH.OH%s.COUNTERS.T1.SENT.L1A"%(link))
