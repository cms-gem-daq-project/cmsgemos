import sys, os, time, signal, random
sys.path.append('${GEM_PYTHON_PATH}')

import uhal
from registers_uhal import *

def calculateLinkErrors(isGLIB,device,sampleTime):
    baseNode = "GLIB.COUNTERS.GTX0"
    errorCounts = {}
    if not isGLIB:
        baseNode = "GLIB.OptoHybrid_0.OptoHybrid.COUNTERS.GTX"

    for link in ("TRK","TRG"):
        writeRegister(device,"%s.%s_ERR.Reset"%(baseNode,link),0x1)
        first = readRegister(device,"%s.%s_ERR"%(baseNode,link))
        time.sleep(sampleTime)
        second = readRegister(device,"%s.%s_ERR"%(baseNode,link))
        errorCounts[link] = [first,second]
    return errorCounts

def optohybridCounters(device,link=0,doReset=False):
    """
    read the optical link counters, returning a map
    if doReset is true, just send the reset command and pass
    WB:MASTER,SLAVE
    CRC:VALID,INCORRECT
    T1:TTC,INTERNAL,EXTERNAL,LOOPBACK,SENT
    GTX
    """
    baseNode = "GLIB.OptoHybrid_%d.OptoHybrid.COUNTERS"%(link)

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
                writeRegister(device,"%s.T1.%s.%s.Reset"%(baseNode, t1),0x1)

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

def readTrackingInfo(device,link):
    """
    read the tracking info from given optical link, returning a map
    """
    baseNode = "GLIB.TRK_DATA.COL%d"%(link)

    data = {}
    data["hasData"] = readRegister(device,"%s.DATA_RDY"%(baseNode))
    for word in range(1,6):
        data["data%d"%(word)] = readRegister(device,"%s.DATA.%d"%(baseNode,word))
    
    for word in data.keys():
        print "%s: 0x%08x"%(word,data[word])
    return data

def readFIFODepth(device,link):
    """
    read the tracking FIFO depth from given optical link
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(link)

    data = {}
    data["isFULL"]    = readRegister(device,"%s.ISFULL"%(baseNode))
    data["isEMPTY"]   = readRegister(device,"%s.ISEMPTY"%(baseNode))
    data["Occupancy"] = readRegister(device,"%s.DEPTH"%(baseNode))
    return data

def setTriggerSource(isGLIB,device,source):
    """
    Set the trigger source
    GLIB: 0=software, 1=backplane, 2=both 
    OH:   0=TTC,     1=FIRMWARE,  2=both 4=ALL
    """
    if isGLIB:
        #writeRegister(device,"GLIB.GLIB_LINKS.LINK%d.TrgSrc"%(link),source)
        writeRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.SOURCE",source)
    else:
        writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CONTROL.TRIGGER.SOURCE",source)
	#writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid_COM.Send.Resync"%(link),0x1)

    return

def configureLocalT1(device,mode,t1type,delay,interval,number):
    """
    Configure the T1 controller
    Mode: 0 (Single T1 signal), 1 (CalPulse followed by L1A), 2 (pattern)
    t1type only for mode 0, type of T1 signal to send, L1A, CalPulse, Resync, BC0
    delay only for mode 1, delay between CalPulse and L1A
    interval only for mode 0,1, how often to repeat signals
    number how many signals to send (0 is continuous
    """
    writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.MODE",mode)
    print "configuring the T1 controller for mode 0x%x (0x%x)"%(
        mode,
        readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.MODE"))
    if (mode == 0):
        writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.TYPE",t1type)
        print "configuring the T1 controller for type 0x%x (0x%x)"%(
            t1type,
            readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.TYPE"))
    if (mode == 1):
        writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.DELAY",delay)
        print "configuring the T1 controller for delay %d (%d)"%(
            delay,
            readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.DELAY"))
    if (mode != 2):
        writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.INTERVAL",interval)
        print "configuring the T1 controller for interval %d (%d)"%(
            interval,
            readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.INTERVAL"))

    writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.NUMBER",number)
    print "configuring the T1 controller for nsignals %d (%d)"%(
        number,
        readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.NUMBER"))
    return

def sendL1ACalPulse(device,delay,number=0):
    """
    Configure the T1 controller
    Mode: 0 (Single T1 signal), 1 (CalPulse followed by L1A), 2 (pattern)
    t1type only for mode 0, type of T1 signal to send, L1A, CalPulse, Resync, BC0
    delay only for mode 1, delay between CalPulse and L1A
    interval only for mode 0,1, how often to repeat signals
    number how many signals to send (0 is continuous
    """
    print "resetting the T1 controller"
    writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.RESET",0x1)
    print "configuring the T1 controller for mode 0x1, delay %d, interval 25, nsignals %d"%(delay,number)
    configureLocalT1(device,0x1,0x0,delay,25,number)
    #if not readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.MONITOR"):
        #writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.T1Controller.TOGGLE",0x1)
    return

def getTriggerSource(isGLIB,device,link):
    """
    Set the trigger source
    GLIB: 0=software, 1=backplane, 2=both 
    OH:   0=GLIB,     1=external,  2=both
    """
    if isGLIB:
        #readRegister(device,"GLIB.GLIB_LINKS.LINK%d.TrgSrc"%(link))
        return readRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.SOURCE"%(link))
    else:
        return readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.TRIGGER.SOURCE"%(link))
        
def setTriggerSBits(isGLIB,device,link,source):
    """
    Set the trigger sbit source
    GLIB: 0=software, 1=backplane, 2=both 
    OH:   0=GLIB,     1=external,  2=both
    """
    if isGLIB:
        #writeRegister(device,"GLIB.GLIB_LINKS.TRIGGER.TDC_SBits",source)
        writeRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(link),source)
    else:
        writeRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.TRIGGER.TDC_SBits"%(link),source)
	writeRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.FAST_COM.Send.Resync"%(link),0x1)

    return

def getTriggerSBits(isGLIB,device,link):
    """
    Set the trigger sbit source
    GLIB: 0=software, 1=backplane, 2=both 
    OH:   0=GLIB,     1=external,  2=both
    """
    if isGLIB:
        return readRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(link))
    else:
        return readRegister(device,"OptoHybrid.OptoHybrid_LINKS.LINK%d.TRIGGER.TDC_SBits"%(link))

def setReferenceClock(device,source):
    """
    Set the reference clock source on the OptoHybrid
    OH:   0=onboard,     1=GTX recovered,  2=external clock
    """
    writeRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CONTROL.CLOCK.REF_CLK",source)
    return

def getClockingInfo(device):
    """
    Get the OptoHybrid clocking information
    """
    clocking = {}

    clocking["fpgaplllock"] = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.FPGA_PLL_LOCK")
    clocking["extplllock"]  = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.EXT_PLL_LOCK" )
    clocking["cdcelock"]    = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.CDCE_LOCK"    )
    clocking["gtxreclock"]  = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.GTX_LOCK" )
    clocking["refclock"]    = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CONTROL.CLOCK.REF_CLK")
    #clocking["vfatsrc"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.VFAT.SOURCE"  )
    #clocking["cdcesrc"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.CDCE.SOURCE"  )
    #clocking["vfatbkp"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.VFAT.FALLBACK")
    #clocking["cdcebkp"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.CDCE.FALLBACK")

    return clocking
