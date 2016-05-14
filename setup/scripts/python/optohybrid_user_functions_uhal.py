import sys, os, time, signal, random
sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *

def getFirmwareVersion(device,gtx=0):
    """
    Returns the OH firmware date as a map (day, month, year)
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid"%(gtx)
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
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.GEB.Broadcast"%(gtx)
    writeRegister(device,"%s.Reset"%(baseNode), 0x1)
    writeRegister(device,"%s.Mask"%(baseNode), 0x0)
    vfatVal  = readRegister(device,"%s.Request.ChipID0"%(baseNode))
    if (debug):
        print "vfatVal = 0x%08x"%(vfatVal)
    vfatVals = readBlock(device,"%s.Results"%(baseNode),24)
    bmask = 0x0
    if (debug and vfatVals):
        for i,val in enumerate(vfatVals):
            print "%d: value = 0x%08x"%(i,vfatVal)
    
    return bmask

def optohybridCounters(device,gtx=0,doReset=False):
    """
    read the optical link counters, returning a map
    if doReset is true, just send the reset command and pass
    WB:MASTER,SLAVE
    CRC:VALID,INCORRECT
    T1:TTC,INTERNAL,EXTERNAL,LOOPBACK,SENT
    GTX
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS"%(gtx)

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
    return writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.TRIGGER.SOURCE"%(gtx),source)

def getTriggerSource(device,gtx):
    """
    Get the trigger source
    OH:   0=TTC, 1=FIRMWARE, 2=EXTERNAL, 3=LOOPBACK
    """
    return readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.TRIGGER.SOURCE"%(gtx))

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
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MODE"%(gtx),mode)
    if debug:
        print "configuring the T1 controller for mode 0x%x (0x%x)"%(
            mode,
            readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MODE"%(gtx)))
    if (mode == 0):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TYPE"%(gtx),t1type)
        if debug:
            print "configuring the T1 controller for type 0x%x (0x%x)"%(
                t1type,
                readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TYPE"%(gtx)))
    if (mode == 1):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.DELAY"%(gtx),delay)
        if debug:
            print "configuring the T1 controller for delay %d (%d)"%(
                delay,
                readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.DELAY"%(gtx)))
    if (mode != 2):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.INTERVAL"%(gtx),interval)
        if debug:
            print "configuring the T1 controller for interval %d (%d)"%(
                interval,
                readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.INTERVAL"%(gtx)))

    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.NUMBER"%(gtx),number)
    if debug:
        print "configuring the T1 controller for nsignals %d (%d)"%(
            number,
            readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.NUMBER"%(gtx)))
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
        print "resetting the T1 controller"
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.RESET"%(gtx),0x1)
    if debug:
        print "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
    configureLocalT1(device,gtx,0x0,0x0,0x0,interval,number)
    #if not readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)):
    #    print "status: 0x%x"%(readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)))
    #    print "toggling T1Controller for sending L1A"
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TOGGLE"%(gtx),0x1)
    #print "status: 0x%x"%(readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)))
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
    if debug:
        print "resetting the T1 controller"
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.RESET"%(gtx),0x1)
    if debug:
        print "configuring the T1 controller for mode 0x1, delay %d, interval %d, nsignals %d"%(delay,interval,number)
    configureLocalT1(device,gtx,0x1,0x0,delay,interval,number)
    if not readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TOGGLE"%(gtx),0x1)
    return

def sendResync(device,gtx,interval=25,number=1,debug=False):
    """
    Send a Resync signal
    """
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.RESET"%(gtx),0x1)
    if debug:
        print "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
    configureLocalT1(device,gtx,0x0,0x2,0x0,interval,number)
    if debug:
        print "current T1 status"%(gtx),readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx))
    if not readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TOGGLE"%(gtx),0x1)
    return

def sendBC0(device,gtx,interval=25,number=1,debug=False):
    """
    Send a BC0 signal
    """
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.RESET"%(gtx),0x1)
    if debug:
        print "configuring the T1 controller for mode 0x0, interval %d, nsignals %d"%(interval,number)
    configureLocalT1(device,gtx,0x0,0x3,0x0,interval,number)
    if debug:
        print "current T1 status"%(gtx),readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx))
    if not readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.MONITOR"%(gtx)):
        writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.T1Controller.TOGGLE"%(gtx),0x1)
    return

def setReferenceClock(device,gtx,source,debug=False):
    """
    Set the reference clock source on the OptoHybrid
    OH:   0=onboard,     1=GTX recovered,  2=external clock
    V2A only
    """
    writeRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.CLOCK.REF_CLK"%(gtx),source)
    return

def getClockingInfo(device,gtx,debug=False):
    """
    Get the OptoHybrid clocking information
    """
    clocking = {}

    # v2b only
    clocking["qplllock"]        = readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.QPLL_LOCK" %(gtx))
    clocking["qpllfpgaplllock"] = readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.QPLL_FPGA_PLL_LOCK"%(gtx))
    
    #v2a only
    clocking["fpgaplllock"] = readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.FPGA_PLL_LOCK"%(gtx))
    clocking["extplllock"]  = readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.EXT_PLL_LOCK" %(gtx))
    clocking["cdcelock"]    = readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.CDCE_LOCK"    %(gtx))
    clocking["gtxreclock"]  = readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.STATUS.GTX_LOCK" %(gtx))
    clocking["refclock"]    = readRegister(device,"GLIB.OptoHybrid_%d.OptoHybrid.CONTROL.CLOCK.REF_CLK"%(gtx))

    return clocking

def getVFATsBitMask(device,gtx=0,debug=False):
    """
    Returns the VFAT s-bit mask
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL"%(gtx)
    return readRegister(device,"%s.SBIT_MASK"%(baseNode))

def setVFATsBitMask(device,gtx=0,mask=0x000000,debug=False):
    """
    Set the VFAT s-bit mask
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.CONTROL"%(gtx)
    return writeRegister(device,"%s.SBIT_MASK"%(baseNode),mask)

def calculateLockErrors(device,gtx,register,sampleTime):
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS"%(gtx)
    errorCounts = {}

    #for link in ("QPLL_LOCK","QPLL_FPGA_PLL_LOCK"):
    writeRegister(device,"%s.%s_LOCK.Reset"%(baseNode,register),0x1)
    first = readRegister(device,"%s.%s_LOCK"%(baseNode,register))
    time.sleep(sampleTime)
    second = readRegister(device,"%s.%s_LOCK"%(baseNode,register))
    errorCounts = [first,second]
    return errorCounts


