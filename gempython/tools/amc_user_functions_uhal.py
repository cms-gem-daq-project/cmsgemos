import sys, time, signal
sys.path.append('${GEM_PYTHON_PATH}')

from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_uhal import *
from gempython.utils.gemlogger import colormsg

import logging
amclogger = logging.getLogger(__name__)

def setAMCLogLevel(level):
    amclogger.setLevel(level)
    pass

class AMCException(Exception):
    pass

def getAMCObject(slot,shelf=1,debug=False):
    connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"
    manager         = uhal.ConnectionManager(connection_file )
    amc             = manager.getDevice( "gem.shelf%02d.amc%02d"%(shelf,slot) )
    if checkAMCBoard(amc):
        return amc
    else:
        raise Exception

def checkAMCBoard(device,debug=False):
    # TO BE IMPLEMENTED
    return True

def calculateLinkErrors(isGLIB,device,gtx,sampleTime):
    baseNode = "GEM_AMC.OH_LINKS"
    errorCounts = nesteddict()
    if not isGLIB:
        baseNode = "GEM_AMC.OH_LINKS"

    for link in ("TRK","TRG"):
        writeRegister(device,"%s.CTRL.CNT_RESET"%(baseNode),0x1)
        first = readRegister(device,"%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
        time.sleep(sampleTime)
        second = readRegister(device,"%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
        errorCounts[link] = [first,second]
    return errorCounts


def glibCounters(device,gtx,doReset=False):
    """
    read the optical gtx counters, returning a map
    if doReset is true, just send the reset command and pass
    IPBus:Strobe,Ack
    T1:L1A,CalPulse,Resync,BC0
    GTX:
    """
    baseNode = "GLIB.COUNTERS"

    if doReset:
        for ipbcnt in ["Strobe","Ack"]:
            writeRegister(device,"%s.IPBus.%s.OptoHybrid_%d.Reset"%(baseNode, ipbcnt, gtx),0x1)
            writeRegister(device,"%s.IPBus.%s.TRK_%d.Reset"%(       baseNode, ipbcnt, gtx),0x1)
            writeRegister(device,"%s.IPBus.%s.Counters.Reset"%(     baseNode, ipbcnt),     0x1)

        #T1 counters
        for t1 in ["L1A", "CalPulse","Resync","BC0"]:
            writeRegister(device,"%s.T1.%s.Reset"%(baseNode, t1),0x1)

        writeRegister(device,"%s.GTX%d.TRK_ERR.Reset"%(     baseNode, gtx), 0x1)
        writeRegister(device,"%s.GTX%d.TRG_ERR.Reset"%(     baseNode, gtx), 0x1)
        writeRegister(device,"%s.GTX%d.DATA_Packets.Reset"%(baseNode, gtx), 0x1)
        return
    else:
        counters = nesteddict()
        
        for ipbcnt in ["Strobe","Ack"]:
            for ipb in ["OptoHybrid","TRK"]:
                counters["IPBus"][ipbcnt][ipb] = readRegister(device,"%s.IPBus.%s.%s_%d"%(   baseNode, ipbcnt,ipb,gtx))
            counters["IPBus"][ipbcnt]["Counters"] = readRegister(device,"%s.IPBus.%s.Counters"%(baseNode, ipbcnt))


        #T1 counters
        for t1 in ["L1A", "CalPulse","Resync","BC0"]:
            counters["T1"][t1] = readRegister(device,"%s.T1.%s"%(baseNode, t1))

        counters["GTX%d"%(gtx)]["TRK_ERR"]      = readRegister(device,"%s.GTX%d.TRK_ERR"%(baseNode,gtx))
        counters["GTX%d"%(gtx)]["TRG_ERR"]      = readRegister(device,"%s.GTX%d.TRG_ERR"%(baseNode,gtx))
        counters["GTX%d"%(gtx)]["DATA_Packets"] = readRegister(device,"%s.GTX%d.DATA_Packets"%(baseNode,gtx))
        return counters


def readTrackingInfo(device,gtx,nBlocks=1):
    """
    read the tracking info from given optical gtx, returning a map
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(gtx)
    data = readBlock(device,"%s.FIFO"%(baseNode),7*nBlocks)
    
    #for word in data:
    #    msg = "%s: 0x%08x"%(word,data)
    #    amclogger.info(colormsg(msg,logging.INFO))
    return data

def flushTrackingFIFO(device,gtx):
    """
    Flush the tracking FIFO from given optical gtx
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(gtx)
    writeRegister(device,"%s.FLUSH"%(baseNode),0)
    return

def readFIFODepth(device,gtx):
    """
    read the tracking FIFO depth from given optical gtx
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(gtx)

    data = nesteddict()
    data["isFULL"]    = readRegister(device,"%s.ISFULL"%(baseNode))
    data["isEMPTY"]   = readRegister(device,"%s.ISEMPTY"%(baseNode))
    data["Occupancy"] = readRegister(device,"%s.DEPTH"%(baseNode))
    return data
        
def setTriggerSBits(isGLIB,device,gtx,source):
    """
    Set the trigger sbit source
    """
    if isGLIB:
        writeRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(gtx),source)
    else:
        writeRegister(device,"GEM_AMC.OH.OH%d.TRIGGER.TDC.SBits"%(gtx),source)
        sendResync(device,1,1)
    return

def getTriggerSBits(isGLIB,device,gtx):
    """
    Get the trigger sbit source
    """
    if isGLIB:
        return readRegister(device,"GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(gtx))
    else:
        return readRegister(device,"GEM_AMC.OH.OH%d.TRIGGER.TDC.SBits"%(gtx))

### DAQ Link functionality
def resetDAQLink(device):
    """
    Reset the DAQ link
    """
    
    writeRegister(device,"GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x1)
    writeRegister(device,"GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x0)
    pass

def enableDAQLinkMask(linkno, linkEnableMask=0x0):
    linkEnableMask |= (0x1<<linkno)
    return linkEnableMask

def enableDAQLink(device, linkEnableMask=0x1, doReset=False):
    msg = "%s: Reset daq_enable: %i"%(device,1)
    amclogger.info(colormsg(msg,logging.INFO))
    if (doReset):
        resetDAQLink(device)
        
    writeRegister(device, "GEM_AMC.DAQ.CONTROL.DAQ_ENABLE",        0x1)
    writeRegister(device, "GEM_AMC.DAQ.CONTROL.TTS_OVERRIDE",      0x8)
    writeRegister(device, "GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", linkEnableMask)
    writeRegister(device, "GEM_AMC.DAQ.CONTROL.DAV_TIMEOUT",       0x30D40)
    for olink in range(NGTX):
        # in 160MHz clock cycles, so multiply by 4 to get in terms of BX
        # 0xc35 -> 781 BX
        writeRegister(device,"GEM_AMC.DAQ.OH%d.CONTROL.EOE_TIMEOUT"%(olink),0x30D4)

def blockL1A(device):
    writeRegister(device, "GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x1)

def enableL1A(device):
    writeRegister(device, "GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x0)
