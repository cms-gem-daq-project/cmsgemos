import os, signal, sys, time
sys.path.append('${GEM_PYTHON_PATH}')

#from gempython.tools.amc_user_functions_uhal import getAMCObject
from gempython.tools.amc_user_functions_xhal import *
#from gempython.tools.glib_system_info_uhal import getSystemFWVer
from gempython.utils.gemlogger import colormsg
from gempython.utils.nesteddict import nesteddict
#from gempython.utils.registers_uhal import *
#from gempython.utils.registers_xhal import *

import logging
#ohlogger = logging.getLogger(__name__)

class scanmode:
    THRESHTRG = 0 # Threshold scan
    THRESHCH  = 1 # Threshold scan per channel
    LATENCY   = 2 # Latency scan
    SCURVE    = 3 # s-curve scan
    THRESHTRK = 4 # Threshold scan with tracking data
    pass

class HwOptoHybrid:
    def __init__(self, slot, link, shelf=1, debug=False):
        """
        Initialize the HW board an open an RPC connection
        """
        # Debug flag
        self.debug = debug
        
        # Logger
        self.ohlogger = logging.getLogger(__name__)

        # Store HW info
        self.link = link
        self.nVFATs = 24
        self.parentAMC = HwAMC(slot, shelf, debug)

        # Define broadcast read
        self.bRead = self.parentAMC.lib.broadcastRead
        self.bRead.argtypes = [c_uint, c_char_p, c_uint, POINTER(c_uint32)]
        self.bRead.restype = c_uint

        # Define broadcast write
        self.bWrite = self.parentAMC.lib.broadcastWrite
        self.bWrite.argtypes = [c_uint, c_char_p, c_uint, c_uint]
        self.bWrite.restype = c_uint
       
        # Define the v3 scan module
        self.genScan = self.parentAMC.lib.genScan
        self.genScan.restype = c_uint
        self.genScan.argtypes = [c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_char_p, POINTER(c_uint32)]

        return

    #def checkOHBoard(self):
    #    # TO BE IMPLEMENTED
    #    return True
        
    #def getBoard(self):
    #    return self.ohboard
    
    def broadcastRead(self,register,mask=0xff000000):
        """
        Perform a broadcast RPC read on the VFATs specified by mask
        Will return when operation has completed
        """
        outData = (c_uint32 * self.nVFATs)()

        try:
            if 0 != self.bRead(self.link, register, mask, outData):
                print("broadcastRead failed for device %i; reg: %s; with mask %x"%(self.link,register,mask))
                sys.exit(os.EX_SOFTWARE)
        except Exception as e:
            print e

        return outData
    
    def broadcastWrite(self,register,value,mask=0xff000000):
        """
        Perform a broadcast RPC write on the VFATs specified by mask
        Will return when operation has completed
        """
        
        rpcResp = 0

        try:
            rpcResp = self.bWrite(self.link, register, value, mask)

            if 0 != rpcResp:
                print("broadcastWrite failed for device %i; reg: %s; with mask %x"%(self.link,register,mask))
                sys.exit(os.EX_SOFTWARE)
        except Exception as e:
            print e

        return rpcResp

    def setDebug(self, debug):
        self.debug = debug
        return

#def getOHObject(slot,link,shelf=1,debug=False):
#    hwOH = HwOptoHybrid(slot, link, shelf, debug)
#    #return hwOH.getBoard()
#    return hwOH
