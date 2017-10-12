import os, sys, time, signal
sys.path.append('${GEM_PYTHON_PATH}')

from gempython.tools.amc_user_functions_uhal import getAMCObject
from gempython.tools.amc_user_functions_xhal import ctp7Params
from gempython.tools.glib_system_info_uhal import getSystemFWVer
from gempython.utils.gemlogger import colormsg
from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_uhal import *
from gempython.utils.registers_xhal import *

import logging
ohlogger = logging.getLogger(__name__)

class HwOptoHybrid:
    def __init__(self, slot, link, shelf=1, debug=False):
        """
        Initialize the HW board an open an RPC connection
        """
        # Debug flag
        self.debug = debug
        
        # Logger
        self.ohlogger = logging.getLogger(__name__)

        # Check v2b/v3 behavior
        amc = getAMCObject(slot,shelf,debug)
        
        # Store HW info
        self.link = link
        self.parentAmc = ctp7Params.cardLocation[(shelf,slot)]
        self.parentAmcFwVersion = getSystemFWVer(amc,debug)
        self.parentShelf = shelf
        self.parentSlot = slot

        # Open RPC Connection
        print "Initializing OH"
        self.rpc = rpcService()
        self.rpc.connect(self.parentAmc)
        
        #if(int(self.parentAmcFwVersion.split(".")[0]) > 2):
        #    print "Initializing OHv3"
        #    self.ohboard = rpcService()
        #    self.ohboard.connect(self.parentAmc)
        #else:
        #    print "Initializing OHv2b"
        #    # For Now IPBus stuff
        #    connection_file = "file://${GEM_ADDRESS_TABLE_PATH}/connections.xml"
        #    manager         = uhal.ConnectionManager(connection_file )
        #    self.ohboard    = manager.getDevice( "gem.shelf%02d.amc%02d.optohybrid%02d"%(shelf,slot,link) )
        #    if self.checkOHBoard():
        #        msg = "%s: Success!"%(self.ohboard)
        #        self.ohlogger.info(colormsg(msg,logging.INFO))
        #    else:
        #        msg = "%s: Failed to create OptoHybrid object"%(ohboard)
        #        raise OptoHybridException(colormsg(msg,logging.FATAL))
    
        return

    def checkOHBoard(self):
        # TO BE IMPLEMENTED
        return True
        
    #def getBoard(self):
    #    return self.ohboard

    def broadcastRead(register,value,mask=0xff000000):
        """
        Perform a broadcast RPC write on the VFATs specified by mask
        Will return when operation has completed
        """
        try:
            retCode = rpc.broadcastRead(self.link, register, mask)
        except Exception as e:
            print e

        return retCode
    
    def broadcastWrite(register,value,mask=0xff000000):
        """
        Perform a broadcast RPC write on the VFATs specified by mask
        Will return when operation has completed
        """
        try:
            retCode = rpc.broadcastWrite(self.link, register, value, mask)
        except Exception as e:
            print e

        return retCode

def getOHObject(slot,link,shelf=1,debug=False):
    hwOH = HwOptoHybrid(slot, link, shelf, debug)
    #return hwOH.getBoard()
    return hwOH
