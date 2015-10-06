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

def linkCounters(isGLIB,device,link,doReset=False):
    """
    read the optical link counters, returning a map
    if doReset is true, just send the reset command and pass
    """
    baseNode = "GLIB.GLIB_LINKS.LINK%d"%(link)
    if not isGLIB:
        baseNode = "OptoHybrid.OptoHybrid_LINKS.LINK%d"%(link)

    if doReset:
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.LinkErr"%(       baseNode),0x1)
        if isGLIB:
            print "emptying FIFO: %s.TRK_FIFO.FLUSH"%(baseNode)
            writeRegister(device,"%s.TRK_FIFO.FLUSH"%(                 baseNode),0x1)
        else:
            writeRegister(device,"%s.COUNTERS.RESETS.L1A.Total"%(     baseNode),0x1)
            writeRegister(device,"%s.COUNTERS.RESETS.CalPulse.Total"%(baseNode),0x1)
            writeRegister(device,"%s.COUNTERS.RESETS.Resync"%(        baseNode),0x1)
            writeRegister(device,"%s.COUNTERS.RESETS.BC0"%(           baseNode),0x1)

        writeRegister(device,"%s.OPTICAL_LINKS.Resets.RecI2CRequests"%(baseNode),0x1)
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.SntI2CRequests"%(baseNode),0x1)
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.RecRegRequests"%(baseNode),0x1)
        writeRegister(device,"%s.OPTICAL_LINKS.Resets.SntRegRequests"%(baseNode),0x1)
        return
    else:
        counters = {}
        counters["LinkErrors"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.LinkErr"%(baseNode))
        if isGLIB:
            counters["TRK_FIFO_Depth"] = readRegister(device,"%s.TRK_FIFO.DEPTH"%(       baseNode))
            print "FIFO: %s.TRK_FIFO.DEPTH = %d (0x%x)"%(baseNode, counters["TRK_FIFO_Depth"], counters["TRK_FIFO_Depth"])
        else:                                                                            
            counters["L1A"]     = readRegister(device,"%s.COUNTERS.L1A.Total"%(      baseNode))
            counters["Cal"]     = readRegister(device,"%s.COUNTERS.CalPulse.Total"%( baseNode))
            counters["Resync"]  = readRegister(device,"%s.COUNTERS.Resync"%(         baseNode))
            counters["BC0"]     = readRegister(device,"%s.COUNTERS.BC0"%(            baseNode))
            counters["BXCount"] = readRegister(device,"%s.COUNTERS.BXCount"%(        baseNode))

        counters["RecI2CRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.RecI2CRequests"%(baseNode))
        counters["SntI2CRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.SntI2CRequests"%(baseNode))
        counters["RecRegRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.RecRegRequests"%(baseNode))
        counters["SntRegRequests"] = readRegister(device,"%s.OPTICAL_LINKS.Counter.SntRegRequests"%(baseNode))
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

def getClockingInfo(device):
    """
    Get the OptoHybrid clocking information
    """
    clocking = {}

    clocking["fpgaplllock"] = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.FPGA_PLL_LOCK")
    clocking["extplllock"]  = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.EXT_PLL_LOCK" )
    clocking["cdcelock"]    = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.CDCE_LOCK"    )
    clocking["gtxreclock"]  = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.STATUS.GTX_LOCK" )
    #clocking["vfatsrc"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.VFAT.SOURCE"  )
    #clocking["cdcesrc"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.CDCE.SOURCE"  )
    #clocking["vfatbkp"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.VFAT.FALLBACK")
    #clocking["cdcebkp"]     = readRegister(device,"GLIB.OptoHybrid_0.OptoHybrid.CLOCKING.CDCE.FALLBACK")

    return clocking
