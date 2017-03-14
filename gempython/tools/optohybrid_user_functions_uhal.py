import sys, time, signal
sys.path.append('${GEM_PYTHON_PATH}')

from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_uhal import *

gemlogger = getGEMLogger(logclassname="optohybrid_user_functions")

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

def getOHObject(slot,shelf,link,debug=False):
    connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"
    manager         = uhal.ConnectionManager(connection_file )
    ohboard         = manager.getDevice( "gem.shelf%02d.amc%02d.optohybrid%02d"%(shelf,slot,link) )
    if checkOHBoard(ohboard):
        return ohboard
    else:
        raise Exception

def checkOHBoard(device,gtx=0,debug=False):
    # TO BE IMPLEMENTED
    return True

def getFirmwareVersionRaw(device,gtx=0,debug=False):
    """
    Returns the raw OH firmware date
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwver = readRegister(device,"%s.STATUS.FW"%(baseNode),debug)
    return fwver

def getFirmwareVersion(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwver = readRegister(device,"%s.STATUS.FW.VERSION"%(baseNode),debug)
    ver = "%x.%x.%x.%x"%(0xff&(fwver>>24),0xff&(fwver>>16),0xff&(fwver>>8),0xff&fwver)
    return ver
    #return date

def getFirmwareDate(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwdate = readRegister(device,"%s.STATUS.FW.DATE"%(baseNode),debug)
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
    vfatVals  = broadcastRead(device,gtx,"ChipID0")
    bmask = 0x0
    if (vfatVals):
        for i,val in enumerate(vfatVals):
            msg = "%d: value = 0x%08x"%(i,val)
            gemlogger.debug(msg)
            pass
        pass

    return bmask

def broadcastWrite(device,gtx,register,value,mask=0xff000000,debug=False):
    """
    Perform a broadcast I2C write on the VFATs specified by mask
    Will return when operation has completed
    """
    baseNode = "GEM_AMC.OH.OH%d.GEB.Broadcast"%(gtx)
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
    Will return data when operation has completed
    """
    baseNode = "GEM_AMC.OH.OH%d.GEB.Broadcast"%(gtx)
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

def optohybridCounters(device,gtx=0,doReset=False,debug=False):
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
        reg_list = nesteddict()
        for wbcnt in ["Strobe","Ack"]:

            reg_list["%s.WB.MASTER.%s.GTX.Reset"%(   baseNode, wbcnt)] = 0x1
            reg_list["%s.WB.MASTER.%s.ExtI2C.Reset"%(baseNode, wbcnt)] = 0x1
            reg_list["%s.WB.MASTER.%s.Scan.Reset"%(  baseNode, wbcnt)] = 0x1
            reg_list["%s.WB.MASTER.%s.DAC.Reset"%(   baseNode, wbcnt)] = 0x1
            # wishbone slaves
            for i2c in range(6):
                reg_list["%s.WB.SLAVE.%s.I2C%d.Reset"%(baseNode, wbcnt, i2c)] = 0x1
            for slave in ["ExtI2C","Scan","T1","DAC","ADC","Clocking","Counters","System"]:
                reg_list["%s.WB.SLAVE.%s.%s.Reset"%(baseNode, wbcnt, slave)] = 0x1
        #CRC counters
        for vfat in range(24):
            reg_list["%s.CRC.VALID.VFAT%d.Reset"%(    baseNode, vfat)] = 0x1
            reg_list["%s.CRC.INCORRECT.VFAT%d.Reset"%(baseNode, vfat)] = 0x1

        #T1 counters
        for t1src in ["TTC", "INTERNAL","EXTERNAL","LOOPBACK","SENT"]:
            for t1 in ["L1A", "CalPulse","Resync","BC0"]:
                reg_list["%s.T1.%s.%s.Reset"%(baseNode, t1src, t1)] = 0x1

        reg_list["%s.GTX.TRK_ERR.Reset"%(     baseNode)] = 0x1
        reg_list["%s.GTX.TRG_ERR.Reset"%(     baseNode)] = 0x1
        reg_list["%s.GTX.DATA_Packets.Reset"%(baseNode)] = 0x1

        writeRegisterList(device,reg_list,debug)
        return
    else:
        counters = nesteddict()

        # counters["WB"] = {}
        # counters["WB"]["MASTER"] = {}
        # counters["WB"]["SLAVE"]  = {}
        reg_list = []
        for wbcnt in ["Strobe","Ack"]:
            # counters["WB"]["MASTER"][wbcnt] = {}
            # reg_list.append("%s.WB.MASTER.%s.GTX"%(   baseNode, wbcnt))
            # reg_list.append("%s.WB.MASTER.%s.ExtI2C"%(baseNode, wbcnt))
            # reg_list.append("%s.WB.MASTER.%s.Scan"%(  baseNode, wbcnt))
            # reg_list.append("%s.WB.MASTER.%s.DAC"%(   baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["GTX"]    = readRegister(device,"%s.WB.MASTER.%s.GTX"%(   baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["ExtI2C"] = readRegister(device,"%s.WB.MASTER.%s.ExtI2C"%(baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["Scan"]   = readRegister(device,"%s.WB.MASTER.%s.Scan"%(  baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["DAC"]    = readRegister(device,"%s.WB.MASTER.%s.DAC"%(   baseNode, wbcnt))

            # wishbone slaves
            # counters["WB"]["SLAVE"][wbcnt]  = {}
            for i2c in range(6):
                # reg_list.append("%s.WB.SLAVE.%s.I2C%d"%(baseNode, wbcnt, i2c))
                counters["WB"]["MASTER"][wbcnt]["I2C%d"%i2c] = readRegister(device,"%s.WB.SLAVE.%s.I2C%d"%(baseNode, wbcnt, i2c))
            for slave in ["ExtI2C","Scan","T1","DAC","ADC","Clocking","Counters","System"]:
                # reg_list.append("%s.WB.SLAVE.%s.%s"%(baseNode, wbcnt, slave))
                counters["WB"]["MASTER"][wbcnt][slave]       = readRegister(device,"%s.WB.SLAVE.%s.%s"%(baseNode, wbcnt, slave))

        #CRC counters
        # counters["CRC"] = {}
        # counters["CRC"]["VALID"]     = {}
        # counters["CRC"]["INCORRECT"] = {}
        for vfat in range(24):
            # reg_list.append("%s.CRC.VALID.VFAT%d"%(    baseNode, vfat))
            # reg_list.append("%s.CRC.INCORRECT.VFAT%d"%(baseNode, vfat))
            counters["CRC"]["VALID"]["VFAT%d"%vfat]     = readRegister(device,"%s.CRC.VALID.VFAT%d"%(    baseNode, vfat))
            counters["CRC"]["INCORRECT"]["VFAT%d"%vfat] = readRegister(device,"%s.CRC.INCORRECT.VFAT%d"%(baseNode, vfat))

        #T1 counters
        # counters["T1"] = {}
        for t1src in ["TTC", "INTERNAL","EXTERNAL","LOOPBACK","SENT"]:
            # counters["T1"][t1src] = {}
            for t1 in ["L1A", "CalPulse","Resync","BC0"]:
                # reg_list.append("%s.T1.%s.%s"%(baseNode, t1src, t1))
                counters["T1"][t1src][t1] = readRegister(device,"%s.T1.%s.%s"%(baseNode, t1src, t1))

        # counters["GTX"] = {}
        # reg_list.append("%s.GTX.TRK_ERR"%(baseNode))
        # reg_list.append("%s.GTX.TRG_ERR"%(baseNode))
        # reg_list.append("%s.GTX.DATA_Packets"%(baseNode))
        counters["GTX"]["TRK_ERR"]      = readRegister(device,"%s.GTX.TRK_ERR"%(baseNode))
        counters["GTX"]["TRG_ERR"]      = readRegister(device,"%s.GTX.TRG_ERR"%(baseNode))
        counters["GTX"]["DATA_Packets"] = readRegister(device,"%s.GTX.DATA_Packets"%(baseNode))

        # reg_vals = readRegisterList(device,reg_list,debug)

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
    return writeRegister(device,"GEM_AMC.OH.OH%d.CONTROL.THROTTLE"%(gtx),throttle)

def getTriggerThrottle(device,gtx):
    """
    Get the trigger throttling value
    """
    return readRegister(device,"GEM_AMC.OH.OH%d.CONTROL.THROTTLE"%(gtx))

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
    gemlogger.debug(msg)

    if (mode == 0):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx),t1type,debug)
        msg = "configuring the T1 controller for type 0x%x (0x%x)"%(
            t1type,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx),debug))
        gemlogger.debug(msg)
    if (mode == 1):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx),delay,debug)
        msg = "configuring the T1 controller for delay %d (%d)"%(
            delay,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx),debug))
        gemlogger.debug(msg)
    if (mode != 2):
        writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx),interval,debug)
        msg = "configuring the T1 controller for interval %d (%d)"%(
            interval,
            readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx),debug))
        gemlogger.debug(msg)

    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx),number,debug)
    msg = "configuring the T1 controller for nsignals %d (%d)"%(
        number,
        readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx),debug))
    gemlogger.debug(msg)
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
    if debug:
        msg = "resetting the T1 controller"
        gemlogger.debug(msg)
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1,debug)
    if debug:
        msg = "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
        gemlogger.debug(msg)
    configureLocalT1(device,gtx,0x0,0x0,0x0,interval,number,debug)
    msg = "current T1 status",readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))
    gemlogger.debug(msg)
    startLocalT1(device,gtx,debug)
    # if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
    #     writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    #if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
    #    msg = "status: 0x%x"%(readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    #    msg = "toggling T1Controller for sending L1A"
    # writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1,debug)
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
    msg = "current T1 status",readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))
    gemlogger.debug(msg)
    startLocalT1(device,gtx,debug)
    # if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
    #     writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def sendResync(device,gtx,interval=25,number=1,debug=False):
    """
    Send a Resync signal
    """
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
    gemlogger.debug(msg)
    configureLocalT1(device,gtx,0x0,0x2,0x0,interval,number)
    msg = "current T1 status",readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))
    gemlogger.debug(msg)
    startLocalT1(device,gtx,debug)
    # if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
    #     writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def sendBC0(device,gtx,interval=25,number=1,debug=False):
    """
    Send a BC0 signal
    """
    writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
    gemlogger.debug(msg)
    configureLocalT1(device,gtx,0x0,0x3,0x0,interval,number)
    msg = "current T1 status",readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))
    gemlogger.debug(msg)
    startLocalT1(device,gtx,debug)
    # if not readRegister(device,"GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
    #     writeRegister(device,"GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
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
    return readRegister(device,"GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx))

def getClockingInfo(device,gtx,debug=False):
    """
    Get the OptoHybrid clocking information
    """
    clocking = nesteddict()

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
    baseNode = "GEM_AMC.OH.OH%d.CONTROL.VFAT"%(gtx)
    return readRegister(device,"%s.TRK_MASK"%(baseNode))

def setVFATTrackingMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.CONTROL.VFAT"%(gtx)
    return writeRegister(device,"%s.TRK_MASK"%(baseNode),mask)

def calculateLockErrors(device,gtx,register,sampleTime):
    baseNode = "GEM_AMC.OH.OH%d.COUNTERS"%(gtx)
    errorCounts = nesteddict()

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

    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
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
    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
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
        regParams = nesteddict()
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

    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
        pass

    if (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        print "Scan is already running (0x%x), not starting a new scan"%(readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        return
    if (readRegister(device,"%s.MONITOR.ERROR"%(scanBase)) > 0):
        print "There was an error in the scan configuration, not starting a new scan"
        return

    writeRegister(device,"%s.START"%(scanBase),0x1)
    if readRegister(device,"%s.MONITOR.ERROR"%(scanBase)) or not readRegister(device,"%s.MONITOR.STATUS"%(scanBase)):
        print "Scan failed to start"#, FIFO read 0x%08x"%(readRegister(device,"%s.RESULTS"%(scanBase)))
        print "ERROR  %d"%(readRegister(device,"%s.MONITOR.ERROR"%(scanBase)))
        print "STATUS %d"%(not readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
        raw_input("enter to continue")
        pass
    if debug:
        print "After start, scan status is: 0x%08x"%(readRegister(device,"%s.MONITOR"%(scanBase)))
        pass
    return

def getScanResults(device, gtx, numpoints, debug=False):
    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
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
    scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
    while (readRegister(device,"%s.MONITOR.STATUS"%(scanBase)) > 0):
        if debug and False:
            print "Ultra scan still running (0x%x), not returning results"%(readRegister(device,"%s.MONITOR.STATUS"%(scanBase)))
            pass
        time.sleep(0.1)
        pass

    if debug:
        print "Ultra scan status (0x%08x)"%(           readRegister(device,"%s.MONITOR"%(      scanBase)))
        print "Ultra scan results available (0x%06x)"%(readRegister(device,"%s.MONITOR.READY"%(scanBase)))
        pass
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
               "TEMP_MAX", "VCCINT_MAX", "VCCAUX_MAX",
               "TEMP_MIN", "VCCINT_MIN", "VCCAUX_MIN"]
    regs = []
    for reg in regList:
        regs.append("%s.%s"%(adcBase,reg))
        pass
    res = readRegisterList(device,regs)
    print "OptoHybrid ADC sysmon"
    print "        %8s  %8s  %8s"%("Temp", "VCCINT", "VCCAUX")
    print "Current:%2.6f  %2.6f  %2.6f"%(res["%s.TEMP"%(adcBase)],
                                         res["%s.VCCINT"%(adcBase)],
                                         res["%s.VCCAUX"%(adcBase)])
    print "Max:    %2.6f  %2.6f  %2.6f"%(res["%s.TEMP_MAX"%(adcBase)],
                                         res["%s.VCCINT_MAX"%(adcBase)],
                                         res["%s.VCCAUX_MAX"%(adcBase)])
    print "Min:    %2.6f  %2.6f  %2.6f"%(res["%s.TEMP_MIN"%(adcBase)],
                                         res["%s.VCCINT_MIN"%(adcBase)],
                                         res["%s.VCCAUX_MIN"%(adcBase)])
    return

def calculateLinkErrors(device,gtx,sampleTime):
    baseNode = "GEM_AMC.OH_LINKS"
    errorCounts = nesteddict()

    for link in ("TRK","TRG"):
        writeRegister(device,"%s.CTRL.CNT_RESET"%(baseNode),0x1)
        first = readRegister(device,"%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
        time.sleep(sampleTime)
        second = readRegister(device,"%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
        errorCounts[link] = [first,second]
    return errorCounts
