import gempython.tools.optohybrid_user_functions_uhal as optofuncs
import gempython.tools.vfat_functions_uhal as vfatfuncs

from gempython.utils.gemlogger import colormsg

import logging
scanlogger = logging.getLogger(__name__)

def checkScanParams(vfat,scanmin,scanmax,stepsize,channel=None):
    if vfat not in range(24):
        raise ValueError

    if channel:
        if channel not in range(128):
            raise ValueError
        pass

    if scanmax > 0xff:
        raise ValueError

    if scanmax >= scanmin:
        raise ValueError
    pass
    
def scanThresholdByVFAT(device,gtx,vfat,
                        scanmin=0x0,scanmax=0xff,
                        stepsize=0x1,numtrigs=1000,
                        trkmode=False,debug=False):
    """ 
    Run a firmware threshold  scan on the selected VFAT
    trkmode=True requires a trigger source
    @returns an array containing the scan results
    """

    optofuncs.configureScanModule(device,gtx,
                                  optofuncs.scanmode.THRESHTRG if trkmode else optofuncs.scanmode.THRESHTRK,
                                  vfat,
                                  scanmin,scanmax,stepsize,numtrigs,
                                  debug=debug)
    optofuncs.startScanModule(device,gtx)
    if (debug):
        optofuncs.printScanConfiguration(device,gtx)
        pass

    msg = "LocalT1Controller status %d"%(optofuncs.getLocalT1Status(device,gtx))
    scanlogger.debug(colormsg(msg,logging.DEBUG))

    data_threshold = optofuncs.getScanResults(device,gtx,scanmax-scanmin,debug=debug)
    
    return data_threshold

def scanThresholdByChannel(device,gtx,vfat,channel,
                           scanmin=0x0,scanmax=0xff,
                           stepsize=0x1,numtrigs=1000,
                           debug=False):
    """ 
    Run a firmware threshold  scan on the selected VFAT/channel combination
    @returns an array containing the scan results
    """

    try:
        checkScanParams(vfat,scanmin,scanmax,stepsize)
    except ValueError:
        msg = "Invalid scan configuration specified"
        scanlogger.error(colormsg(msg,logging.ERROR))
        return None
        
    optofuncs.configureScanModule(device,gtx,
                                  optofuncs.scanmode.THRESHCH,
                                  vfat,channel,
                                  scanmin,scanmax,stepsize,numtrigs,
                                  debug=debug)
    optofuncs.startScanModule(device,gtx)
    if (debug):
        optofuncs.printScanConfiguration(device,gtx)
        pass

    msg = "LocalT1Controller status %d"%(optofuncs.getLocalT1Status(device,gtx))
    scanlogger.debug(colormsg(msg,logging.DEBUG))

    data_threshold = optofuncs.getScanResults(device,gtx,scanmax-scanmin,debug=debug)
    
    return data_threshold

def scanLatencyByVFAT(device,gtx,vfat,
                      scanmin=0x0,scanmax=0xff,
                      stepsize=0x1,numtrigs=1000,
                      debug=False):
    """
    Run a firmware latency  scan on the selected VFAT
    requires a trigger source
    @returns an array containing the scan results
    """

    try:
        checkScanParams(vfat,scanmin,scanmax,stepsize)
    except ValueError:
        msg = "Invalid scan configuration specified"
        scanlogger.error(colormsg(msg,logging.ERROR))
        return None
        
    channel = 10
    originalChannel = vfatfuncs.getChannelRegister(device,gtx,vfat,channel)
    originalVCal    = vfatfuncs.readVFAT(device,gtx,vfat,"VCal")

    vfatfuncs.setChannelRegister(device,gtx,vfat,channel,
                                 mask=0x0,pulse=0x1,trim=trim)
    vfatfuncs.writeVFAT(device,gtx,vfat,"VCal",0xff)

    optofuncs.configureScanModule(device,gtx,
                                  optofuncs.scanmode.LATENCY,
                                  vfat,
                                  scanmin,scanmax,
                                  stepsize=1,numtrigs=numtrigs,
                                  debug=debug)
    optofuncs.startScanModule(device,gtx)
    if (debug):
        optofuncs.printScanConfiguration(device,gtx)
        pass

    msg = "LocalT1Controller status %d"%(optofuncs.getLocalT1Status(device,gtx))
    scanlogger.debug(colormsg(msg,logging.DEBUG))

    data_latency = optofuncs.getScanResults(device,gtx,scanmax-scanmin,debug=debug)
    
    # return settings to start
    mask  = (originalChannel>>5)&0x1
    pulse = (originalChannel>>6)&0x1
    trim  = originalChannel&0x1f

    vfatfuncs.setChannelRegister(device,gtx,vfat,channel,
                                 mask=mask,pulse=pulse,trim=trim,
                                 debug=debug)
    vfatfuncs.writeVFAT(device,gtx,vfat,"VCal",originalVcal)

    return data_latency


def scanVCalByVFAT(device,gtx,vfat,channel,trim,
                   scanmin=0x0,scanmax=0xff,
                   stepsize=0x1,numtrigs=1000,
                   debug=False):
    """
    Run a firmware VCal  scan on the selected VFAT/channel combination
    requires a trigger source, cal pulse source, channel enabled
    @returns an array containing the scan results
    """
    
    try:
        checkScanParams(vfat,scanmin,scanmax,stepsize)
    except ValueError:
        msg = "Invalid scan configuration specified"
        scanlogger.error(colormsg(msg,logging.ERROR))
        return None
        
    originalChannel = vfatfuncs.getChannelRegister(device,gtx,vfat,channel)

    vfatfuncs.setChannelRegister(device,gtx,vfat,channel,
                                 mask=0x0,pulse=0x1,trim=trim)
    
    msg = "Channel %d register 0x%08x"%(channel,getChannelRegister(device,gtx,vfat,channel))
    scanlogger.debug(colormsg(msg,logging.DEBUG))

    optofuncs.configureScanModule(device,gtx,
                                  optofuncs.scanmode.SCURVE,
                                  vfat,
                                  channel=channel,
                                  scanmin=scanmin,
                                  scanmax=scanmax,
                                  stepsize=stepsize,
                                  numtrigs=numtrigs,
                                  debug=debug)
    optofuncs.startScanModule(device,gtx)

    if (debug):
        optofuncs.printScanConfiguration(device,gtx)
        pass

    msg = "LocalT1Controller status %d"%(optofuncs.getLocalT1Status(device,gtx))
    scanlogger.debug(colormsg(msg,logging.DEBUG))

    data_scurve = optofuncs.getScanResults(device,gtx,
                                           scanmax-scanmin,
                                           debug=debug)
    
    # return settings to start
    mask  = (originalChannel>>5)&0x1
    pulse = (originalChannel>>6)&0x1
    trim  = originalChannel&0x1f

    vfatfuncs.setChannelRegister(device,gtx,vfat,channel,
                                 mask=mask,pulse=pulse,trim=trim,
                                 debug=debug)

    msg = "Channel %d register 0x%08x"%(channel,vfatfuncs.getChannelRegister(device,gtx,vfat,channel))
    scanlogger.debug(colormsg(msg,logging.DEBUG))

    return data_scurve

### ULTRA scan mode

### DAC scans
