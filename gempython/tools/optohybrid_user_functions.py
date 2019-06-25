import time
# import sys, time, signal
# sys.path.append('${GEM_PYTHON_PATH}')

from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_uhal import *
from gempython.utils.gemlogger import colormsg
from gempython.core._cmsgemos_gempython import HwGenericAMC

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

def getOHObject(slot,link,shelf=1,ctrlhubhost=None,use_connection_file=False,debug=False):
    """
    Connect to a uhal OptoHybrid HwDevice either using a connection file, or creating the connection on the fly
    """

    hostname     = "gem-shelf{:02d}-amc{:02d}-optohybrid{:02d}".format(shelf,slot,link)

    if use_connection_file:
        connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"
        ohboard         = HwOptoHybrid(hostname, connection_file)
    else:
        addresstable = "file://${{GEM_ADDRESS_TABLE_PATH}}/uhal_gem_amc_ctp7_link{:02d}.xml".format(link)
        if ctrlhubhost:
            # for controlhub connection, non-CTP7 hardware
            port         = 50001
            ctrlhubport  = 10203
            proto = "chtcp-2.0"
            uri = "{}://{}:{}?target={}:{}".format(proto,ctrlhubhost,ctrlhubport,hostname,port)
        else:
            port  = 60002
            proto = "ipbustcp-2.0"
            uri   = "{}://{}:{}".format(proto,hostname,port)

        ohboard = HwOptoHybrid(hostname, uri, addresstable)

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
    Returns the raw OH firmware date
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwver = device.readRegister("%s.STATUS.FW"%(baseNode))
    return fwver

def getFirmwareVersion(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwver = device.readRegister("%s.STATUS.FW.VERSION"%(baseNode))
    ver = "%x.%x.%x.%x"%(0xff&(fwver>>24),0xff&(fwver>>16),0xff&(fwver>>8),0xff&fwver)
    return ver
    #return date

def getFirmwareDate(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwdate = device.readRegister("%s.STATUS.FW.DATE"%(baseNode))
    date = nesteddict()
    date["d"] = fwdate&0xff
    date["m"] = (fwdate>>8)&0xff
    date["y"] = (fwdate>>16)&0xffff
    return date

def getFirmwareDateString(device,gtx=0,old=False,debug=False):
    date = getFirmwareDate(device,gtx)
    if old:
        date = getFirmwareDateOld(device,gtx)
    return "%02x/%02x/%04x"%(date["d"],date["m"],date["y"])

def getFirmwareDateOld(device,gtx=0,debug=False):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GEM_AMC.OH.OH%d"%(gtx)
    fwdate = device.readRegister("%s.STATUS.FW_DATE"%(baseNode))
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
            ohlogger.debug(colormsg(msg,logging.DEBUG))
            pass
        pass

    return bmask

def broadcastWrite(device,gtx,register,value,mask=0xff000000,debug=False):
    """
    Perform a broadcast I2C write on the VFATs specified by mask
    Will return when operation has completed
    """
    baseNode = "GEM_AMC.OH.OH%d.GEB.Broadcast"%(gtx)
    device.writeRegister("%s.Reset"%(baseNode), 0x1)
    device.writeRegister("%s.Mask"%(baseNode), mask)
    device.writeRegister("%s.Request.%s"%(baseNode,register),value)

    msg ="%s: broadcast read request status 0x%x"%(device,device.readRegister("%s.Running"%(baseNode)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    while (device.readRegister("%s.Running"%(baseNode))):
        msg ="%s: broadcast read request still running..."%(device)
        ohlogger.debug(colormsg(msg,logging.DEBUG))
        time.sleep(0.1)
        pass

def broadcastRead(device,gtx,register,mask=0xff000000,debug=False):
    """
    Perform a broadcast I2C read on the VFATs specified by mask
    Will return data when operation has completed
    """
    baseNode = "GEM_AMC.OH.OH%d.GEB.Broadcast"%(gtx)
    device.writeRegister("%s.Reset"%(baseNode), 0x1)
    device.writeRegister("%s.Mask"%(baseNode), mask)
    device.readRegister("%s.Request.%s"%(baseNode,register))

    msg = "%s: broadcast write request status 0x%x"%(device,device.readRegister("%s.Running"%(baseNode)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    while (device.readRegister("%s.Running"%(baseNode))):
        msg ="%s: broadcast write request still running..."%(device)
        ohlogger.debug(colormsg(msg,logging.DEBUG))
        time.sleep(0.1)
        pass

    # bitcount = bits not set in mask
    bitcount = 24
    return readBlock(device,"%s.Results"%(baseNode),bitcount)

def optohybridCounters(device,gtx=0,doReset=False,debug=False):
    """
    read the optical link counters, returning a map
    if doReset is true, just send the reset command and pass
    WB:MASTER,SLAVE
    CRC:VALID,INCORRECT
    T1:GTX_TTC,GBT_TTC,INTERNAL,EXTERNAL,LOOPBACK,SENT
    GTX
    GBT
    """
    baseNode = "GEM_AMC.OH.OH%d.COUNTERS"%(gtx)

    if doReset:
        reg_list = nesteddict()
        for wbcnt in ["Strobe","Ack"]:

            reg_list["%s.WB.MASTER.%s.GTX.Reset"%(   baseNode, wbcnt)] = 0x1
            reg_list["%s.WB.MASTER.%s.GBT.Reset"%(   baseNode, wbcnt)] = 0x1
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
        for t1src in ["GTX_TTC", "GBT_TTC", "INTERNAL","EXTERNAL","LOOPBACK","SENT"]:
            for t1 in ["L1A", "CalPulse","Resync","BC0"]:
                reg_list["%s.T1.%s.%s.Reset"%(baseNode, t1src, t1)] = 0x1

        reg_list["%s.GTX_LINK.TRK_ERR.Reset"%(     baseNode)] = 0x1
        # reg_list["%s.GTX_LINK.TRG_ERR.Reset"%(   baseNode)] = 0x1
        reg_list["%s.GTX_LINK.DATA_Packets.Reset"%(baseNode)] = 0x1
        reg_list["%s.GBT_LINK.TRK_ERR.Reset"%(     baseNode)] = 0x1
        reg_list["%s.GBT_LINK.DATA_Packets.Reset"%(baseNode)] = 0x1

        writeRegisterList(device,reg_list,debug)
        return
    else:
        counters = nesteddict()

        reg_list = []
        for wbcnt in ["Strobe","Ack"]:
            # reg_list.append("%s.WB.MASTER.%s.GTX"%(   baseNode, wbcnt))
            # reg_list.append("%s.WB.MASTER.%s.GBT"%(   baseNode, wbcnt))
            # reg_list.append("%s.WB.MASTER.%s.ExtI2C"%(baseNode, wbcnt))
            # reg_list.append("%s.WB.MASTER.%s.Scan"%(  baseNode, wbcnt))
            # reg_list.append("%s.WB.MASTER.%s.DAC"%(   baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["GTX"]    = device.readRegister("%s.WB.MASTER.%s.GTX"%(   baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["GBT"]    = device.readRegister("%s.WB.MASTER.%s.GBT"%(   baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["ExtI2C"] = device.readRegister("%s.WB.MASTER.%s.ExtI2C"%(baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["Scan"]   = device.readRegister("%s.WB.MASTER.%s.Scan"%(  baseNode, wbcnt))
            counters["WB"]["MASTER"][wbcnt]["DAC"]    = device.readRegister("%s.WB.MASTER.%s.DAC"%(   baseNode, wbcnt))

            # wishbone slaves
            for i2c in range(6):
                # reg_list.append("%s.WB.SLAVE.%s.I2C%d"%(baseNode, wbcnt, i2c))
                counters["WB"]["MASTER"][wbcnt]["I2C%d"%i2c] = device.readRegister("%s.WB.SLAVE.%s.I2C%d"%(baseNode, wbcnt, i2c))
            for slave in ["ExtI2C","Scan","T1","DAC","ADC","Clocking","Counters","System"]:
                # reg_list.append("%s.WB.SLAVE.%s.%s"%(baseNode, wbcnt, slave))
                counters["WB"]["MASTER"][wbcnt][slave]       = device.readRegister("%s.WB.SLAVE.%s.%s"%(baseNode, wbcnt, slave))

        #CRC counters
        for vfat in range(24):
            # reg_list.append("%s.CRC.VALID.VFAT%d"%(    baseNode, vfat))
            # reg_list.append("%s.CRC.INCORRECT.VFAT%d"%(baseNode, vfat))
            counters["CRC"]["VALID"]["VFAT%d"%vfat]     = device.readRegister("%s.CRC.VALID.VFAT%d"%(    baseNode, vfat))
            counters["CRC"]["INCORRECT"]["VFAT%d"%vfat] = device.readRegister("%s.CRC.INCORRECT.VFAT%d"%(baseNode, vfat))

        #T1 counters
        for t1src in ["GTX_TTC", "GBT_TTC", "INTERNAL","EXTERNAL","LOOPBACK","SENT"]:
            for t1 in ["L1A", "CalPulse","Resync","BC0"]:
                # reg_list.append("%s.T1.%s.%s"%(baseNode, t1src, t1))
                counters["T1"][t1src][t1] = device.readRegister("%s.T1.%s.%s"%(baseNode, t1src, t1))

        # reg_list.append("%s.GTX.TRK_ERR"%(baseNode))
        # reg_list.append("%s.GTX.TRG_ERR"%(baseNode))
        # reg_list.append("%s.GTX.DATA_Packets"%(baseNode))
        counters["GTX"]["TRK_ERR"]      = device.readRegister("%s.GTX_LINK.TRK_ERR"%(baseNode))
        counters["GTX"]["DATA_Packets"] = device.readRegister("%s.GTX_LINK.DATA_Packets"%(baseNode))
        counters["GBT"]["TRK_ERR"]      = device.readRegister("%s.GBT_LINK.TRK_ERR"%(baseNode))
        counters["GBT"]["DATA_Packets"] = device.readRegister("%s.GBT_LINK.DATA_Packets"%(baseNode))

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
    return device.writeRegister("GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(gtx),source)

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
    return device.readRegister("GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(gtx))

def setTriggerThrottle(device,gtx,throttle):
    """
    Set the trigger throttle
    """
    return device.writeRegister("GEM_AMC.OH.OH%d.CONTROL.TRIGGER.THROTTLE"%(gtx),throttle)

def getTriggerThrottle(device,gtx):
    """
    Get the trigger throttling value
    """
    return device.readRegister("GEM_AMC.OH.OH%d.CONTROL.TRIGGER.THROTTLE"%(gtx))

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

    device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.MODE"%(gtx),mode)
    msg = "configuring the T1 controller for mode 0x%x (0x%x)"%(
        mode,
        device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MODE"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    if (mode == 0):
        device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx),t1type)
        msg = "configuring the T1 controller for type 0x%x (0x%x)"%(
            t1type,
            device.readRegister("GEM_AMC.OH.OH%d.T1Controller.TYPE"%(gtx)))
        ohlogger.debug(colormsg(msg,logging.DEBUG))
    if (mode == 1):
        device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx),delay)
        msg = "configuring the T1 controller for delay %d (%d)"%(
            delay,
            device.readRegister("GEM_AMC.OH.OH%d.T1Controller.DELAY"%(gtx)))
        ohlogger.debug(colormsg(msg,logging.DEBUG))
    if (mode != 2):
        device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx),interval)
        msg = "configuring the T1 controller for interval %d (%d)"%(
            interval,
            device.readRegister("GEM_AMC.OH.OH%d.T1Controller.INTERVAL"%(gtx)))
        ohlogger.debug(colormsg(msg,logging.DEBUG))

    device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx),number)
    msg = "configuring the T1 controller for nsignals %d (%d)"%(
        number,
        device.readRegister("GEM_AMC.OH.OH%d.T1Controller.NUMBER"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    return

def resetLocalT1(device,gtx,debug=False):
    device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    return

def getLocalT1Status(device,gtx,debug=False):
    return device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx))

def startLocalT1(device,gtx,debug=False):
    if not device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
    return

def stopLocalT1(device,gtx,debug=False):
    if device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.TOGGLE"%(gtx),0x1)
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
    device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "%s: configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(device,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x0,0x0,0x0,interval,number,debug)
    msg = "%s: current T1 status 0x%x"%(device,device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
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
    device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "%s: configuring the T1 controller for mode 0x1, delay %d, interval %d, nsignals %d"%(device,delay,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x1,0x0,delay,interval,number)
    msg = "%s: current T1 status 0x%x"%(device,device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    return

def sendResync(device,gtx,interval=25,number=1,debug=False):
    """
    Send a Resync signal
    """
    msg = "%s: resetting the T1 controller"%(device)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "%s: configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(device,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x0,0x2,0x0,interval,number)
    msg = "%s: current T1 status 0x%x"%(device,device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    return

def sendBC0(device,gtx,interval=25,number=1,debug=False):
    """
    Send a BC0 signal
    """
    msg = "%s: resetting the T1 controller"%(device)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    device.writeRegister("GEM_AMC.OH.OH%d.T1Controller.RESET"%(gtx),0x1)
    msg = "%s: configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(device,interval,number)
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    configureLocalT1(device,gtx,0x0,0x3,0x0,interval,number)
    msg = "%s: current T1 status 0x%x"%(device,device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    startLocalT1(device,gtx,debug)
    if not device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)):
        msg = "%s: T1Controller failed to start: status: 0x%x"%(device,
                                                                device.readRegister("GEM_AMC.OH.OH%d.T1Controller.MONITOR"%(gtx)))
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
    device.writeRegister("GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx),source)
    return

def getReferenceClock(device,gtx,debug=False):
    """
    Get the reference clock source on the OptoHybrid
    V2A:   0=onboard, 1=GTX recovered,  2=external clock
    V2B:   0=GBT,     1=CCB HDMI
    """
    return device.readRegister("GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx))

def getClockingInfo(device,gtx,debug=False):
    """
    Get the OptoHybrid clocking information
    """
    clocking = nesteddict()

    # v2b only
    clocking["qplllock"]        = device.readRegister("GEM_AMC.OH.OH%d.STATUS.QPLL_LOCK" %(gtx))
    clocking["qpllfpgaplllock"] = device.readRegister("GEM_AMC.OH.OH%d.STATUS.QPLL_FPGA_PLL_LOCK"%(gtx))

    #v2a only
    clocking["fpgaplllock"] = device.readRegister("GEM_AMC.OH.OH%d.STATUS.FPGA_PLL_LOCK"%(gtx))
    clocking["extplllock"]  = device.readRegister("GEM_AMC.OH.OH%d.STATUS.EXT_PLL_LOCK" %(gtx))
    clocking["cdcelock"]    = device.readRegister("GEM_AMC.OH.OH%d.STATUS.CDCE_LOCK"    %(gtx))
    clocking["gtxreclock"]  = device.readRegister("GEM_AMC.OH.OH%d.STATUS.GTX_LOCK" %(gtx))
    clocking["refclock"]    = device.readRegister("GEM_AMC.OH.OH%d.CONTROL.CLOCK.REF_CLK"%(gtx))

    return clocking

def getVFATsBitMask(device,gtx=0,debug=False):
    """
    Returns the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.CONTROL"%(gtx)
    return device.readRegister("%s.VFAT.SBIT_MASK"%(baseNode))

def setVFATsBitMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.CONTROL"%(gtx)
    return device.writeRegister("%s.VFAT.SBIT_MASK"%(baseNode),mask)

def getVFATTrackingMask(device,gtx=0,debug=False):
    """
    Returns the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.CONTROL.VFAT"%(gtx)
    return device.readRegister("%s.TRK_MASK"%(baseNode))

def setVFATTrackingMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GEM_AMC.OH.OH%d.CONTROL.VFAT"%(gtx)
    return device.writeRegister("%s.TRK_MASK"%(baseNode),mask)

def calculateLockErrors(device,gtx,register,sampleTime):
    baseNode = "GEM_AMC.OH.OH%d.COUNTERS"%(gtx)
    errorCounts = nesteddict()

    #for link in ("QPLL_LOCK","QPLL_FPGA_PLL_LOCK"):
    device.writeRegister("%s.%s_LOCK.Reset"%(baseNode,register),0x1)
    first = device.readRegister("%s.%s_LOCK"%(baseNode,register))
    time.sleep(sampleTime)
    second = device.readRegister("%s.%s_LOCK"%(baseNode,register))
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
    vfat: for single VFAT scan, specify the VFAT number
          for ULTRA scan, specify the VFAT mask
    """

    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
        pass

    if (device.readRegister("%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Scan is already running (0x%x), not starting a new scan"%(device,
                                                                             device.readRegister("%s.MONITOR.STATUS"%(scanBase)))
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

    device.writeRegister("%s.RESET"%(scanBase),0x1)
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
        device.writeRegister("GEM_AMC.OH.OH%d.COUNTERS.CRC.INCORRECT.VFAT%d.Reset"%(gtx,i), 1)
        device.writeRegister("GEM_AMC.OH.OH%d.COUNTERS.CRC.VALID.VFAT%d.Reset"%(gtx,i), 1)

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
    print "FW scan status     : 0x%08x"%(device.readRegister("%s.MONITOR"%(scanBase)))

    return

def startScanModule(device, gtx, useUltra=False,debug=False):
    """
    """

    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    if useUltra:
        scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
        pass

    if (device.readRegister("%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Scan is already running (0x%x), not starting a new scan"%(device,
                                                                             device.readRegister("%s.MONITOR.STATUS"%(scanBase)))
        ohlogger.warning(colormsg(msg,logging.WARNING))
        return
    if (device.readRegister("%s.MONITOR.ERROR"%(scanBase)) > 0):
        msg = "%s: There was an error in the scan configuration, not starting a new scan"%(device)
        ohlogger.warning(colormsg(msg,logging.WARNING))
        raise OptoHybridException(colormsg(msg,logging.FATAL))

    device.writeRegister("%s.START"%(scanBase),0x1)
    if device.readRegister("%s.MONITOR.ERROR"%(scanBase)) or not device.readRegister("%s.MONITOR.STATUS"%(scanBase)):
        msg = "%s: Scan failed to start"%(device)
        msg += "ERROR  %d"%(device.readRegister("%s.MONITOR.ERROR"%(scanBase)))
        msg += "STATUS %d"%(not device.readRegister("%s.MONITOR.STATUS"%(scanBase)))
        raise OptoHybridException(colormsg(msg,logging.FATAL))
    msg = "%s: After start, scan status is: 0x%08x"%(device,device.readRegister("%s.MONITOR"%(scanBase)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))
    return

def getScanResults(device, gtx, numpoints, debug=False):
    scanBase = "GEM_AMC.OH.OH%d.ScanController.THLAT"%(gtx)
    while (device.readRegister("%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Scan still running (0x%x), not returning results"%(device,device.readRegister("%s.MONITOR.STATUS"%(scanBase)))
        ohlogger.debug(colormsg(msg,logging.DEBUG))
        time.sleep(0.1)
        pass

    msg = "%s::getScanResults\n"%(device)
    msg+= "Scan status (0x%08x)\n"%(device.readRegister("%s.MONITOR"%(scanBase)))
    msg+= "Scan results available (0x%06x)"%(device.readRegister("%s.MONITOR.READY"%(scanBase)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    results = readBlock(device,"%s.RESULTS"%(scanBase),numpoints)

    return results

def getUltraScanResults(device, gtx, numpoints, debug=False):
    scanBase = "GEM_AMC.OH.OH%d.ScanController.ULTRA"%(gtx)
    ohnL1A_0 = getL1ACount(device,gtx)
    ohnL1A = getL1ACount(device,gtx)
    numtrigs = device.readRegister("%s.CONF.NTRIGS"%(scanBase))
    if (device.readRegister("%s.CONF.MODE"%(scanBase))==2):
        isLatency = True
        print "At link %s: %d/%d L1As processed, %d%% done" %(gtx, getL1ACount(device,gtx)-ohnL1A_0, numpoints*numtrigs, (getL1ACount(device,gtx)-ohnL1A_0)*100./(numpoints*numtrigs))
    else:
        isLatency = False    
    while (device.readRegister("%s.MONITOR.STATUS"%(scanBase)) > 0):
        msg = "%s: Ultra scan still running (0x%x), not returning results"%(device,
                                                                            device.readRegister("%s.MONITOR.STATUS"%(scanBase)))
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
    msg = "Ultra scan status (0x%08x)\n"%(         device.readRegister("%s.MONITOR"%(      scanBase)))
    msg+= "Ultra scan results available (0x%06x)"%(device.readRegister("%s.MONITOR.READY"%(scanBase)))
    ohlogger.debug(colormsg(msg,logging.DEBUG))

    results = []

    for chip in range(24):
        results.append(readBlock(device,"%s.RESULTS.VFAT%d"%(scanBase,chip),numpoints))

    return results

def getADCValue(device, gtx, adc, debug=False):
    adcBase = "GEM_AMC.OH.OH%d.ADC"%(gtx)
    adcinfo = device.readRegister("%s.%s"%(adcBase,adc))
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

    # device.writeRegister("%s.CTRL.CNT_RESET"%(baseNode),0x1)
    first  = device.readRegister("%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
    time.sleep(sampleTime)
    second = device.readRegister("%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
    errorCounts["LINK"] = [first,second]

    baseNode = "GEM_AMC.OH"
    for link in ("GBT","GTX"):
        device.writeRegister("%s.OH%d.COUNTERS.%s_LINK.TRK_ERR.Reset"%(baseNode,gtx,link),0x1)
        first = device.readRegister("%s.OH%d.COUNTERS.%s_LINK.TRK_ERR"%(baseNode,gtx,link))
        time.sleep(sampleTime)
        second = device.readRegister("%s.OH%d.COUNTERS.%s_LINK.TRK_ERR"%(baseNode,gtx,link))
        errorCounts[link] = [first,second]
    return errorCounts
                                                                                      
def getL1ACount(device,link):
    return device.readRegister( "GEM_AMC.OH.OH%s.COUNTERS.T1.SENT.L1A"%(link))
