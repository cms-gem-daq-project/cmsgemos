import sys, os, time, signal, random
sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *
from gemlogger import GEMLogger
gemlogger = GEMLogger("optohybrid_user_functions").gemlogger

def getFirmwareVersionRaw(device,gtx=0):
    """
    Returns the raw OH firmware date
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwver = readRegister(device,"%s.STATUS.FW"%(baseNode))
    return fwver

def getFirmwareVersion(device,gtx=0):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwver = readRegister(device,"%s.STATUS.FW"%(baseNode))
    date = {}
    date["d"] = fwver&0xff
    date["m"] = (fwver>>8)&0xff
    date["y"] = (fwver>>16)&0xffff
    return date

def getConnectedVFATsMask(device,gtx=0,debug=False):
    """
    Returns the broadcast I2C mask corresponding to the connected VFATs
    """
    vfatVal  = broadcastRead(device,gtx,"ChipID0")
    bmask = 0x0
    if (vfatVals):
        for i,val in enumerate(vfatVals):
            msg = "%d: value = 0x%08x"%(i,vfatVal)
            gemlogger.debug(msg)
            pass
        pass

    return bmask

def broadcastWrite(device,gtx,register,value,mask=0xff000000,debug=False):
    """
    Perform a broadcast I2C write on the VFATs specified by mask
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast"%(gtx)
    writeRegister(device,"%s.Reset"%(baseNode), 0x1,debug)
    writeRegister(device,"%s.Mask"%(baseNode), mask,debug)
    writeRegister(device,"%s.Request.%s"%(baseNode,register),value,debug)
    if debug:
        readRegister(device,"%s.Running"%(baseNode),debug)
        pass
    while (readRegister(device,"%s.Running"%(baseNode))):
        i = 1
        if (debug):
            print "broadcast request still running..."
            pass
        pass
    return

def broadcastRead(device,gtx,register,mask=0xff000000,debug=False):
    """
    Perform a broadcast I2C read on the VFATs specified by mask
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast"%(gtx)
    writeRegister(device,"%s.Reset"%(baseNode), 0x1,debug)
    writeRegister(device,"%s.Mask"%(baseNode), mask,debug)
    readRegister(device,"%s.Request.%s"%(baseNode,register),debug)
    if debug:
        readRegister(device,"%s.Running"%(baseNode),debug)
        pass
    while (readRegister(device,"%s.Running"%(baseNode))):
        i = 1
        if (debug):
            print "broadcast request still running..."
            pass
        pass
    return readBlock(device,"%s.Results"%(baseNode),24)

def optohybridCounters(device,gtx=0,doReset=False):
    """
    read the optical link counters, returning a map
    if doReset is true, just send the reset command and pass
    WB:MASTER,SLAVE
    CRC:VALID,INCORRECT
    T1:TTC,INTERNAL,EXTERNAL,LOOPBACK,SENT
    GTX
    """
    baseNode = "GEM_AMC.OH.OH%d.COUNTERS"%(gtx)

    if doReset:
        for wbcnt in ["Strobe","Ack"]:
            writeRegister(device,"%s.WB.MASTER.%s.GTX.Reset"%(   baseNode, wbcnt),0x1)
            writeRegister(device,"%s.WB.MASTER.%s.ExtI2C.Reset"%(baseNode, wbcnt),0x1)
            writeRegister(device,"%s.WB.MASTER.%s.Scan.Reset"%(  baseNode, wbcnt),0x1)
            writeRegister(device,"%s.WB.MASTER.%s.DAC.Reset"%(   baseNode, wbcnt),0x1)
            # wishbone slaves
            for i2c in range(6):
                writeRegister(device,"%s.WB.SLAVE.%s.I2C%d.Reset"%(baseNode, wbcnt, i2c),0x1)
            for slave in ["ExtI2C","Scan","T1","DAC","ADC","Clocking","Counters","System"]:
                writeRegister(device,"%s.WB.SLAVE.%s.%s.Reset"%(baseNode, wbcnt, slave),0x1)
        #CRC counters
        for vfat in range(24):
            writeRegister(device,"%s.CRC.VALID.VFAT%d.Reset"%(    baseNode, vfat),0x1)
            writeRegister(device,"%s.CRC.INCORRECT.VFAT%d.Reset"%(baseNode, vfat),0x1)

        #T1 counters
        for t1src in ["TTC", "INTERNAL","EXTERNAL","LOOPBACK","SENT"]:
            for t1 in ["L1A", "CalPulse","Resync","BC0"]:
                writeRegister(device,"%s.T1.%s.%s.Reset"%(baseNode, t1src, t1),0x1)

        writeRegister(device,"%s.GTX.TRK_ERR.Reset"%(     baseNode), 0x1)
        writeRegister(device,"%s.GTX.TRG_ERR.Reset"%(     baseNode), 0x1)
        writeRegister(device,"%s.GTX.DATA_Packets.Reset"%(baseNode), 0x1)
        return
    else:
        counters = {}

        counters["WB"] = {}
        counters["WB"]["MASTER"] = {}
        counters["WB"]["SLAVE"]  = {}
        for wbcnt in ["Strobe","Ack"]:
            counters["WB"]["MASTER"][wbcnt] = {}
            counters["WB"]["MASTER"][wbcnt]["GTX"] = readRegister(device,"%s.WB.MASTER.%s.GTX"%(   baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["ExtI2C"] = readRegister(device,"%s.WB.MASTER.%s.ExtI2C"%(baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["Scan"] = readRegister(device,"%s.WB.MASTER.%s.Scan"%(  baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["DAC"] = readRegister(device,"%s.WB.MASTER.%s.DAC"%(   baseNode, wbcnt))

            # wishbone slaves
            counters["WB"]["SLAVE"][wbcnt]  = {}
            for i2c in range(6):
                counters["WB"]["MASTER"][wbcnt]["I2C%d"%i2c] = readRegister(device,"%s.WB.SLAVE.%s.I2C%d"%(baseNode, wbcnt, i2c))
            for slave in ["ExtI2C","Scan","T1","DAC","ADC","Clocking","Counters","System"]:
                counters["WB"]["MASTER"][wbcnt][slave] = readRegister(device,"%s.WB.SLAVE.%s.%s"%(baseNode, wbcnt, slave))

        #CRC counters
        counters["CRC"] = {}
        counters["CRC"]["VALID"]     = {}
        counters["CRC"]["INCORRECT"] = {}
        for vfat in range(24):
            counters["CRC"]["VALID"]["VFAT%d"%vfat]     = readRegister(device,"%s.CRC.VALID.VFAT%d"%(    baseNode, vfat))
            counters["CRC"]["INCORRECT"]["VFAT%d"%vfat] = readRegister(device,"%s.CRC.INCORRECT.VFAT%d"%(baseNode, vfat))

        #T1 counters
        counters["T1"] = {}
        for t1src in ["TTC", "INTERNAL","EXTERNAL","LOOPBACK","SENT"]:
            counters["T1"][t1src] = {}
            for t1 in ["L1A", "CalPulse","Resync","BC0"]:
                counters["T1"][t1src][t1] = readRegister(device,"%s.T1.%s.%s"%(baseNode, t1src, t1))

        counters["GTX"] = {}
        counters["GTX"]["TRK_ERR"] = readRegister(device,"%s.GTX.TRK_ERR"%(baseNode))
        counters["GTX"]["TRG_ERR"] = readRegister(device,"%s.GTX.TRG_ERR"%(baseNode))
        counters["GTX"]["DATA_Packets"] = readRegister(device,"%s.GTX.DATA_Packets"%(baseNode))
        return counters

def setTriggerSource(device,gtx,source):
    """
    Set the trigger source
    OH:   0=TTC, 1=FIRMWARE, 2=EXTERNAL, 3=LOOPBACK
    """
    return writeRegister(device,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(gtx),source)

def getTriggerSource(device,gtx):
    """
    Get the trigger source
    OH:   0=TTC, 1=FIRMWARE, 2=EXTERNAL, 3=LOOPBACK
    """
    return readRegister(device,"GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(gtx))

def setTriggerThrottle(device,gtx,throttle):
    """
    Set the trigger throttle
    """
    return writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.THROTTLE"%(gtx),throttle)

def getTriggerThrottle(device,gtx):
    """
    Get the trigger throttling value
    """
    return readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.THROTTLE"%(gtx))

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
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MODE"%(gtx),mode)
    msg = "configuring the T1 controller for mode 0x%x (0x%x)"%(
        mode,
        readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MODE"%(gtx)))
    gemlogger.debug(msg)

    if (mode == 0):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx),t1type)
        msg = "configuring the T1 controller for type 0x%x (0x%x)"%(
            t1type,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx)))
        gemlogger.debug(msg)
    if (mode == 1):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx),delay)
        msg = "configuring the T1 controller for delay %d (%d)"%(
            delay,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx)))
        gemlogger.debug(msg)
    if (mode != 2):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx),interval)
        msg = "configuring the T1 controller for interval %d (%d)"%(
            interval,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx)))
        gemlogger.debug(msg)

    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx),number)
    msg = "configuring the T1 controller for nsignals %d (%d)"%(
        number,
        readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx)))
    gemlogger.debug(msg)
    return

def resetLocalT1(device,gtx,debug=False):
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.RESET"%(gtx),0x1)
    return

def getLocalT1Status(device,gtx,debug=False):
    return readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx))

def startLocalT1(device,gtx,debug=False):
    if not readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TOGGLE"%(gtx),0x1)
    return

def stopLocalT1(device,gtx,debug=False):
    if readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TOGGLE"%(gtx),0x1)
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
    if debug:
        msg = "resetting the T1 controller"
        gemlogger.debug(msg)
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    if debug:
        msg = "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
        gemlogger.debug(msg)
    configureLocalT1(device,gtx,0x0,0x0,0x0,interval,number)
    #if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
    #    msg = "status: 0x%x"%(readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    #    msg = "toggling T1Controller for sending L1A"
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    #msg = "status: 0x%x"%(readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
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
    msg = "resetting the T1 controller"
    gemlogger.debug(msg)
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "configuring the T1 controller for mode 0x1, delay %d, interval %d, nsignals %d"%(delay,interval,number)
    gemlogger.debug(msg)
    configureLocalT1(device,gtx,0x1,0x0,delay,interval,number)
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def sendResync(device,gtx,interval=25,number=1,debug=False):
    """
    Send a Resync signal
    """
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
    gemlogger.debug(msg)
    configureLocalT1(device,gtx,0x0,0x2,0x0,interval,number)
    msg = "current T1 status"%(gtx),readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))
    gemlogger.debug(msg)
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def sendBC0(device,gtx,interval=25,number=1,debug=False):
    """
    Send a BC0 signal
    """
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
    gemlogger.debug(msg)
    configureLocalT1(device,gtx,0x0,0x3,0x0,interval,number)
    msg = "current T1 status"%(gtx),readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))
    gemlogger.debug(msg)
    if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def setReferenceClock(device,gtx,source,debug=False):
    """
    Set the reference clock source on the OptoHybrid
    OH:   0=onboard,     1=GTX recovered,  2=external clock
    V2A only
    """
    # writeRegister(device,"GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx),source)
    return

def getReferenceClock(device,gtx,debug=False):
    """
    Get the reference clock source on the OptoHybrid
    OH:   0=onboard,     1=GTX recovered,  2=external clock
    V2A only
    """
    return readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.CLOCK.REF_CLK"%(gtx))

def getClockingInfo(device,gtx,debug=False):
    """
    Get the OptoHybrid clocking information
    """
    clocking = {}

    # v2b only
    clocking["qplllock"]        = readRegister(device,"GEM_AMC.OH.OH%d.STATUS.QPLL_LOCK" %(gtx))
    clocking["qpllfpgaplllock"] = readRegister(device,"GEM_AMC.OH.OH%d.STATUS.QPLL_FPGA_PLL_LOCK"%(gtx))

    #v2a only
    clocking["fpgaplllock"] = readRegister(device,"GEM_AMC.OH.OH%d.STATUS.FPGA_PLL_LOCK"%(gtx))
    clocking["extplllock"]  = readRegister(device,"GEM_AMC.OH.OH%d.STATUS.EXT_PLL_LOCK" %(gtx))
    clocking["cdcelock"]    = readRegister(device,"GEM_AMC.OH.OH%d.STATUS.CDCE_LOCK"    %(gtx))
    clocking["gtxreclock"]  = readRegister(device,"GEM_AMC.OH.OH%d.STATUS.GTX_LOCK" %(gtx))
    clocking["refclock"]    = readRegister(device,"GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx))

    return clocking

def getVFATsBitMask(device,gtx=0,debug=False):
    """
    Returns the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.CONTROL"%(gtx)
    return readRegister(device,"%s.VFAT.SBIT_MASK"%(baseNode))

def setVFATsBitMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.CONTROL"%(gtx)
    return writeRegister(device,"%s.VFAT.SBIT_MASK"%(baseNode),mask)

def getVFATTrackingMask(device,gtx=0,debug=False):
    """
    Returns the VFAT s-bit mask
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.VFAT"%(gtx)
    return readRegister(device,"%s.TRK_MASK"%(baseNode))

def setVFATTrackingMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.VFAT"%(gtx)
    return writeRegister(device,"%s.TRK_MASK"%(baseNode),mask)

def calculateLockErrors(device,gtx,register,sampleTime):
    baseNode = "GEM_AMC.OH.OH%d.COUNTERS"%(gtx)
    errorCounts = {}

    #for link in ("QPLL_LOCK","QPLL_FPGA_PLL_LOCK"):
    writeRegister(device,"%s.%s_LOCK.Reset"%(baseNode,register),0x1)
    first = readRegister(device,"%s.%s_LOCK"%(baseNode,register))
    time.sleep(sampleTime)
    second = readRegister(device,"%s.%s_LOCK"%(baseNode,register))
    errorCounts = [first,second]
    return errorCounts

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
    """

    scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.ULTRA"%(gtx)
        pass

    if (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        print "Scan is already running (0x%x), not starting a new scan"%(readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        return

    if debug:
        print scanBase
        print "FW scan mode       : %d"%(mode)
        print "FW scan min        : %d"%(scanmin)
        print "FW scan max        : %d"%(scanmax)
        if useUltra:
            print "Ultra FW scan mask : 0x%08x"%(vfat)
        else:
            print "FW scan VFAT       : %d"%(vfat)
            pass
        print "FW scan channel    : %d"%(channel)
        print "FW scan step size  : %d"%(stepsize)
        print "FW scan n_triggers : %d"%(numtrigs)
        pass

    writeRegister(device,"%s.RESET"%(scanBase),0x1)
    regList = {
        "%s.MODE"%(scanBase):  mode,
        "%s.MIN"%(scanBase):   scanmin,
        "%s.MAX"%(scanBase):   scanmax,
        "%s.CHAN"%(scanBase):  channel,
        "%s.STEP"%(scanBase):  stepsize,
        "%s.NTRIGS"%(scanBase):numtrigs
     }
    if useUltra:
        regList["%s.MASK"%(scanBase)] = vfat
    else:
        regList["%s.CHIP"%(scanBase)] = vfat
        pass

    writeRegisterList(device,regList)
    return

def printScanConfiguration(device,gtx,useUltra=False,debug=False):
    """
    """
    scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.ULTRA"%(gtx)
        pass

    print scanBase
    regList = [
        "%s.MODE"%(scanBase),
        "%s.MIN"%(scanBase),
        "%s.MAX"%(scanBase),
        "%s.CHAN"%(scanBase),
        "%s.STEP"%(scanBase),
        "%s.NTRIGS"%(scanBase),
        "%s.MONITOR"%(scanBase),
     ]
    if useUltra:
        regList.append("%s.MASK"%(scanBase))
    else:
        regList.append("%s.CHIP"%(scanBase))
        pass

    if debug:
        regParams = {}
        for reg in regList:
            regParams[reg] = {
                "Path":       "%s"%(device.getNode(reg).getPath()),
                "Address":    "0x%x"%(device.getNode(reg).getAddress()),
                "Mask":       "0x%x"%(device.getNode(reg).getMask()),
                "Permission": "%s"%(device.getNode(reg).getPermission()),
                "Mode":       "%s"%(device.getNode(reg).getMode()),
                "Size":       "%s"%(device.getNode(reg).getSize())
                }
            print regParams[reg]
            pass
        pass
    regVals = readRegisterList(device,regList)
    print "FW scan mode       : %d"%(regVals["%s.MODE"%(scanBase)])
    print "FW scan min        : %d"%(regVals["%s.MIN"%(scanBase)])
    print "FW scan max        : %d"%(regVals["%s.MAX"%(scanBase)])
    if useUltra:
        print "Ultra FW scan mask : 0x%08x"%(regVals["%s.MASK"%(scanBase)])
    else:
        print "FW scan VFAT       : %d"%(regVals["%s.CHIP"%(scanBase)])
        pass
    print "FW scan channel    : %d"%(regVals["%s.CHAN"%(scanBase)])
    print "FW scan step size  : %d"%(regVals["%s.STEP"%(scanBase)])
    print "FW scan n_triggers : %d"%(regVals["%s.NTRIGS"%(scanBase)])
    print "FW scan status     : 0x%08x"%(readRegister(device,"%s.MONITOR"%(scanBase)))

    return

def startScanModule(device, gtx, useUltra=False,debug=False):
    """
    """

    scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.ULTRA"%(gtx)
        pass

    if (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        print "Scan is already running (0x%x), not starting a new scan"%(readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        return
    if (readRegister(device,"%s.MONITOR.ERROR"%(scanBase)) > 0):
        print "There was an error in the scan configuration, not starting a new scan"
        return

    writeRegister(device,"%s.START"%(scanBase),0x1)
    if readRegister(device,"%s.MONITOR.ERROR"%(scanBase)) or not (readRegister(device,"%s.MONITOR.STATUS"%(scanBase))):
        print "Scan failed to start, FIFO read 0x%08x"%(readRegister(device,"%s.RESULTS"%(scanBase)))
        pass
    if debug:
        print "After start, scan status is: 0x%08x"%(readRegister(device,"%s.MONITOR"%(scanBase)))
        pass
    return

def getScanResults(device, gtx, numpoints, debug=False):
    scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.THLAT"%(gtx)
    while (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        if debug and False:
            print "Scan still running (0x%x), not returning results"%(readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
            pass
        time.sleep(0.1)
        pass

    if debug:
        print "Scan status (0x%08x)"%(readRegister(device,"%s.MONITOR"%(scanBase)))
        print "Scan results available (0x%06x)"%(readRegister(device,"%s.MONITOR.READY"%(scanBase)))
    # results = []
    # results.append(readRegister(device,"%s.RESULTS"%(scanBase)))
    # print "0x%08x"%(results[0])
    # results.append(readBlock(device,"%s.RESULTS"%(scanBase),numpoints-1))
    # print results
    results = readBlock(device,"%s.RESULTS"%(scanBase),numpoints)
    return results

def getUltraScanResults(device, gtx, numpoints, debug=False):
    scanBase = "GLIB.OptoHybrid_%d.OptoHybrid.ScanController.ULTRA"%(gtx)
    while (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        if debug and False:
            print "Scan still running (0x%x), not returning results"%(readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
            pass
        time.sleep(0.1)
        pass

    if debug:
        print "Scan status (0x%08x)"%(readRegister(device,"%s.MONITOR"%(scanBase)))
        print "Scan results available (0x%06x)"%(readRegister(device,"%s.MONITOR.READY"%(scanBase)))
        pass
    results = []

    for chip in range(24):
        results.append(readBlock(device,"%s.RESULTS.VFAT%d"%(scanBase,chip),numpoints))

    return results
