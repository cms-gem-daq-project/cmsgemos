## TODO: REWORK based on cmsgemos_gempython boosted
# only a few helpers should remain, the rest are imported from the c++ library
import time
# import sys, time, signal
# sys.path.append('${GEM_PYTHON_PATH}')

from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_uhal import *
from gempython.utils.gemlogger import colormsg,colors
from gempython.core._cmsgemos_gempython import HwGenericAMC

import logging
amclogger = logging.getLogger(__name__)

def setAMCLogLevel(level):
    amclogger.setLevel(level)
    pass

class AMCException(Exception):
    pass

def getAMCObject(slot,shelf=1,ctrlhubhost=None,use_connection_file=False,debug=False):
    """
    Connect to a HwGenericAMC device either using a connection file, or creating the connection on the fly
    """

    hostname     = "gem-shelf{:02d}-amc{:02d}".format(shelf,slot)

    if use_connection_file:
        connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"
        amc             = HwGenericAMC(hostname, connection_file)
    else:
        addresstable = "file://${GEM_ADDRESS_TABLE_PATH}/uhal_gem_amc_ctp7_amc.xml"
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

        amc = HwGenericAMC(hostname, uri, addresstable)

    if checkAMCBoard(amc):
        msg = "%s: Success!"%(amc)
        amclogger.info(colormsg(msg,logging.INFO))
        return amc
    else:
        msg = "%s: Failed to create AMC object"%(amc)
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
        device.writeRegister("%s.CTRL.CNT_RESET"%(baseNode),0x1)
        first = device.readRegister("%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
        time.sleep(sampleTime)
        second = device.readRegister("%s.OH%d.TRACK_LINK_ERROR_CNT"%(baseNode,gtx))
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
            device.writeRegister("%s.IPBus.%s.OptoHybrid_%d.Reset"%(baseNode, ipbcnt, gtx),0x1)
            device.writeRegister("%s.IPBus.%s.TRK_%d.Reset"%(       baseNode, ipbcnt, gtx),0x1)
            device.writeRegister("%s.IPBus.%s.Counters.Reset"%(     baseNode, ipbcnt),     0x1)

        #T1 counters
        for t1 in ["L1A", "CalPulse","Resync","BC0"]:
            device.writeRegister("%s.T1.%s.Reset"%(baseNode, t1),0x1)

        device.writeRegister("%s.GTX%d.TRK_ERR.Reset"%(     baseNode, gtx), 0x1)
        device.writeRegister("%s.GTX%d.TRG_ERR.Reset"%(     baseNode, gtx), 0x1)
        device.writeRegister("%s.GTX%d.DATA_Packets.Reset"%(baseNode, gtx), 0x1)
        return
    else:
        counters = nesteddict()
        
        for ipbcnt in ["Strobe","Ack"]:
            for ipb in ["OptoHybrid","TRK"]:
                counters["IPBus"][ipbcnt][ipb] = device.readRegister("%s.IPBus.%s.%s_%d"%(   baseNode, ipbcnt,ipb,gtx))
            counters["IPBus"][ipbcnt]["Counters"] = device.readRegister("%s.IPBus.%s.Counters"%(baseNode, ipbcnt))


        #T1 counters
        for t1 in ["L1A", "CalPulse","Resync","BC0"]:
            counters["T1"][t1] = device.readRegister("%s.T1.%s"%(baseNode, t1))

        counters["GTX%d"%(gtx)]["TRK_ERR"]      = device.readRegister("%s.GTX%d.TRK_ERR"%(baseNode,gtx))
        counters["GTX%d"%(gtx)]["TRG_ERR"]      = device.readRegister("%s.GTX%d.TRG_ERR"%(baseNode,gtx))
        counters["GTX%d"%(gtx)]["DATA_Packets"] = device.readRegister("%s.GTX%d.DATA_Packets"%(baseNode,gtx))
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
    device.writeRegister("%s.FLUSH"%(baseNode),0)
    return

def readFIFODepth(device,gtx):
    """
    read the tracking FIFO depth from given optical gtx
    """
    baseNode = "GLIB.TRK_DATA.OptoHybrid_%d"%(gtx)

    data = nesteddict()
    data["isFULL"]    = device.readRegister("%s.ISFULL"%(baseNode))
    data["isEMPTY"]   = device.readRegister("%s.ISEMPTY"%(baseNode))
    data["Occupancy"] = device.readRegister("%s.DEPTH"%(baseNode))
    return data
        
def setTriggerSBits(isGLIB,device,gtx,source):
    """
    Set the trigger sbit source
    """
    if isGLIB:
        device.writeRegister("GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(gtx),source)
    else:
        device.writeRegister("GEM_AMC.OH.OH%d.TRIGGER.TDC.SBits"%(gtx),source)
        sendResync(device,1,1)
    return

def getTriggerSBits(isGLIB,device,gtx):
    """
    Get the trigger sbit source
    """
    if isGLIB:
        return device.readRegister("GLIB.GLIB_LINKS.LINK%d.TRIGGER.TDC_SBits"%(gtx))
    else:
        return device.readRegister("GEM_AMC.OH.OH%d.TRIGGER.TDC.SBits"%(gtx))

### DAQ Link functionality
def resetDAQLink(device):
    """
    Reset the DAQ link
    """
    
    device.writeRegister("GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x1)
    device.writeRegister("GEM_AMC.DAQ.CONTROL.DAQ_LINK_RESET",0x0)
    pass

def enableDAQLinkMask(linkno, linkEnableMask=0x0):
    linkEnableMask |= (0x1<<linkno)
    return linkEnableMask

def enableDAQLink(device, linkEnableMask=0x1, doReset=False):
    msg = "%s: Reset daq_enable: %i"%(device,1)
    amclogger.info(colormsg(msg,logging.INFO))
    if (doReset):
        resetDAQLink(device)
        
    device.writeRegister( "GEM_AMC.DAQ.CONTROL.DAQ_ENABLE",        0x1)
    device.writeRegister( "GEM_AMC.DAQ.CONTROL.TTS_OVERRIDE",      0x8)
    device.writeRegister( "GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK", linkEnableMask)
    device.writeRegister( "GEM_AMC.DAQ.CONTROL.DAV_TIMEOUT",       0x30D40)
    for olink in range(NGTX):
        # in 160MHz clock cycles, so multiply by 4 to get in terms of BX
        # 0xc35 -> 781 BX
        device.writeRegister("GEM_AMC.DAQ.OH%d.CONTROL.EOE_TIMEOUT"%(olink),0x30D4)

def blockL1A(device):
    device.writeRegister( "GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x0)

def enableL1A(device):
    device.writeRegister( "GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x1)

def getL1ACount(device):
    return device.readRegister( "GEM_AMC.TTC.CMD_COUNTERS.L1A")

def printSystemTTCInfo(amc,debug=False):
    print "--=======================================--"
    print "-> GEM SYSTEM TTC INFORMATION"
    print "--=======================================--"
    print
    mmcmLck      = amc.readRegister("GEM_AMC.TTC.STATUS.CLK.MMCM_LOCKED")
    mmcmULckCnt  = amc.readRegister("GEM_AMC.TTC.STATUS.CLK.MMCM_UNLOCK_CNT")
    phaseLck     = amc.readRegister("GEM_AMC.TTC.STATUS.CLK.PHASE_LOCKED")
    phaseULckCnt = amc.readRegister("GEM_AMC.TTC.STATUS.CLK.PHASE_UNLOCK_CNT")
    bc0Lck       = amc.readRegister("GEM_AMC.TTC.STATUS.BC0.LOCKED")
    bc0ULckCnt   = amc.readRegister("GEM_AMC.TTC.STATUS.BC0.UNLOCK_CNT")
    syncDone     = amc.readRegister("GEM_AMC.TTC.STATUS.CLK.SYNC_DONE")
    ttcSglErrCnt = amc.readRegister("GEM_AMC.TTC.STATUS.TTC_SINGLE_ERROR_CNT")
    ttcDblErrCnt = amc.readRegister("GEM_AMC.TTC.STATUS.TTC_DOUBLE_ERROR_CNT")
    bc0OflwCnt   = amc.readRegister("GEM_AMC.TTC.STATUS.BC0.OVERFLOW_CNT")
    bc0UflwCnt   = amc.readRegister("GEM_AMC.TTC.STATUS.BC0.UNDERFLOW_CNT")
    print("BC0.LOCKED           %s0x%08x%s"%(colors.GREEN if bc0Lck           else colors.RED,bc0Lck,colors.ENDC))
    print("MMCM_LOCKED          %s0x%08x%s"%(colors.GREEN if mmcmLck          else colors.RED,mmcmLck,colors.ENDC))
    print("PHASE_LOCKED         %s0x%08x%s"%(colors.GREEN if phaseLck         else colors.RED,phaseLck,colors.ENDC))
    print("SYNC_DONE            %s0x%08x%s"%(colors.GREEN if syncDone         else colors.RED,syncDone,colors.ENDC))
    print("MMCM_UNLOCK_CNT      %s0x%08x%s"%(colors.RED   if mmcmULckCnt > 0  else colors.GREEN,mmcmULckCnt,colors.ENDC))
    print("BC0.UNLOCK_CNT       %s0x%08x%s"%(colors.RED   if bc0ULckCnt > 0   else colors.GREEN,bc0ULckCnt,colors.ENDC))
    print("PHASE_UNLOCK_CNT     %s0x%08x%s"%(colors.RED   if phaseULckCnt > 0 else colors.GREEN,phaseULckCnt,colors.ENDC))
    print("PHASE_UNLOCK_TIME    %s0x%08x%s"%(colors.BLUE,amc.readRegister("GEM_AMC.TTC.STATUS.CLK.PHASE_UNLOCK_TIME"),colors.ENDC))
    print("SYNC_DONE_TIME       %s0x%08x%s"%(colors.BLUE,amc.readRegister("GEM_AMC.TTC.STATUS.CLK.SYNC_DONE_TIME"   ),colors.ENDC))
    print("TTC_SINGLE_ERROR_CNT %s0x%08x%s"%(colors.GREEN if ttcSglErrCnt == 0  else colors.YELLOW if ttcSglErrCnt < 0xffff else colors.RED,ttcSglErrCnt,colors.ENDC))
    print("TTC_DOUBLE_ERROR_CNT %s0x%08x%s"%(colors.GREEN if ttcDblErrCnt == 0  else colors.YELLOW if ttcDblErrCnt < 0xffff else colors.RED,ttcDblErrCnt,colors.ENDC))
    print("BC0.OVERFLOW_CNT     %s0x%08x%s"%(colors.GREEN if bc0OflwCnt == 0    else colors.YELLOW if bc0OflwCnt < 0xffff else colors.RED,bc0OflwCnt,colors.ENDC))
    print("BC0.UNDERFLOW_CNT    %s0x%08x%s"%(colors.GREEN if bc0UflwCnt == 0    else colors.YELLOW if bc0UflwCnt < 0xffff else colors.RED,bc0UflwCnt,colors.ENDC))
    print
    pass

def printSystemSCAInfo(amc,debug=False):
    print "--=======================================--"
    print "-> GEM SYSTEM SCA INFORMATION"
    print "--=======================================--"
    print
    nOHs = amc.readRegister("GEM_AMC.GEM_SYSTEM.CONFIG.NUM_OF_OH")
    scaRdy  = amc.readRegister("GEM_AMC.SLOW_CONTROL.SCA.STATUS.READY")
    scaCErr = amc.readRegister("GEM_AMC.SLOW_CONTROL.SCA.STATUS.CRITICAL_ERROR")
    print("READY             %s0x%08x%s"%(colors.BLUE,scaRdy,colors.ENDC))
    print("CRITICAL_ERROR    %s0x%08x%s"%(colors.RED if scaCErr else colors.GREEN,scaCErr,colors.ENDC))
    for li in range(nOHs):
        print("NOT_READY_CNT_OH%02d %s0x%08x%s"%(li,
                                                 colors.GREEN,
                                                 amc.readRegister("GEM_AMC.SLOW_CONTROL.SCA.STATUS.NOT_READY_CNT_OH%d"%(li)),
                                                 colors.ENDC))
    pass
