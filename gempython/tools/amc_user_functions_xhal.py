from ctypes import *
from gempython.utils.gemlogger import colors

from reg_utils.reg_interface.common.reg_base_ops import rpc_connect, rReg, writeReg
from reg_utils.reg_interface.common.reg_xml_parser import getNode, parseInt, parseXML

from xhal.reg_interface_gem.core.reg_extra_ops import rBlock

import logging

gMAX_RETRIES = 5
gRetries = 5

class HwAMC(object):
    def __init__(self, cardName, debug=False):
        """
        Initialize the HW board an open an RPC connection
        """
        
        # Debug flag
        self.debug = debug
        
        # Logger
        self.amclogger = logging.getLogger(__name__)

        # Store HW info
        self.name = cardName
        #self.nOHs = 12
        self.nOHs = 4

        # Open the foreign function library
        self.lib = CDLL("librpcman.so")
       
        # Define VFAT3 DAC Monitoring
        self.confDacMonitorMulti = self.lib.configureVFAT3DacMonitorMultiLink
        self.confDacMonitorMulti.argTypes = [ c_uint, POINTER(c_uint32), c_uint ]
        self.confDacMonitorMulti.restype = c_uint

        self.readADCsMulti = self.lib.readVFAT3ADCMultiLink
        self.readADCsMulti.argTypes = [ c_uint, POINTER(c_uint32), c_uint, c_uint ]
        self.readADCsMulti.restype = c_uint

        # Define Get OH Mask functionality
        self.self.lib.getOHVFATMask = self.lib.getOHVFATMask
        self.self.lib.getOHVFATMask.argTypes = [ c_uint ]
        self.self.lib.getOHVFATMask.restype = c_uint

        self.getOHVFATMaskMultiLink = self.lib.getOHVFATMaskMultiLink
        self.getOHVFATMaskMultiLink.argTypes = [ c_uint, POINTER(c_uint32) ]
        self.getOHVFATMaskMultiLink.restype = c_uint

        # Define TTC Functions
        self.ttcGenConf = self.lib.ttcGenConf
        self.ttcGenConf.restype = c_uint
        self.ttcGenConf.argtypes = [c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_bool]

        self.ttcGenToggle = self.lib.ttcGenToggle
        self.ttcGenToggle.restype = c_uint
        self.ttcGenToggle.argtypes = [c_uint, c_bool]

        # Define SBIT Local Readout
        self.readSBits = self.lib.sbitReadOut
        self.readSBits.restype = c_uint
        self.readSBits.argtypes = [c_uint, c_uint, c_char_p]

        # Parse XML
        parseXML()

        # Open RPC Connection
        print "Initializing AMC", self.name
        rpc_connect(self.name)
        self.fwVersion = self.readRegister("GEM_AMC.GEM_SYSTEM.RELEASE.MAJOR")
        print "My FW release major = ", self.fwVersion

        return

    def acquireSBits(self, ohN, outFilePath, acquireTime=300):
        """
        Using the SBIT_MONITOR acquire sbit data for time in seconds given
        by acquireTime and write the data to the directory specified by outFilePath

        ohN - optohybrid to monitor
        outFilePath - filepath created data files will be written too
        acquireTime - time in seconds to acquire data for
        """

        return self.readSBits(ohN, acquireTime, outFilePath)

    def blockL1A(self):
        """
        blocks L1A's from backplane for this AMC
        """
       
        self.writeRegister("GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x0)
        return

    def configureTTC(self, pulseDelay, L1Ainterval, ohN=0, mode=0, t1type=0,  nPulses=0, enable=True):
        """
        Default values reflects v3 electronics behavior

        =======v3 electronics Behavior=======
             pulseDelay (only for enable = true), delay between CalPulse and L1A
             L1Ainterval (only for enable = true), how often to repeat signals
             enable = true (false) ignore (take) ttc commands from backplane for this AMC (affects all links)
        =======v2b electronics behavior=======
             Configure the T1 controller
             mode: 0 (Single T1 signal),
                   1 (CalPulse followed by L1A),
                   2 (pattern)
             type (only for mode 0, type of T1 signal to send):
                   0 L1A
                   1 CalPulse
                   2 Resync
                   3 BC0
             pulseDelay (only for mode 1), delay between CalPulse and L1A
             L1Ainterval (only for mode 0,1), how often to repeat signals
             nPulses how many signals to send (0 is continuous)
             enable = true (false) start (stop) the T1Controller for link ohN
        """

        return self.ttcGenConf(ohN, mode, t1type, pulseDelay, L1Ainterval, nPulses, enable)

    def configureVFAT3DacMonitorMulti(self, dacSelect, ohMask=0xFFF):
        ohVFATMaskArray = (c_uint32 * self.nOHs)()
        for ohN in range(0,self.nOHs):
            if ((ohMask >> ohN) & 0x0):
                continue

            ohVFATMaskArray[ohN] = self.getOHVFATMask(ohN)

    def enableL1A(self):
        """
        enables L1A's from backplane for this AMC
        """
        
        self.writeRegister("GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x1)
        return

    def getL1ACount(self):
        return self.readRegister("GEM_AMC.TTC.CMD_COUNTERS.L1A")

    def getTTCStatus(self, ohN, display=False):
        running = 0xdeaddead
        if self.fwVersion < 3:
            contBase = "GEM_AMC.OH.OH%i.T1Controller"%(ohN)
            running = self.readRegister("%s.MONITOR"%(contBase))
            if display:
                print("Info for %s"%(contBase))
                print("\tDELAY:\t\t%i"%(self.readRegister("%s.DELAY"%(contBase))))
                print("\tINTERVAL:\t%i"%(self.readRegister("%s.INTERVAL"%(contBase))))
                print("\tMODE:\t\t%i"%(self.readRegister("%s.MODE"%(contBase))))
                print("\tMONITOR:\t%i"%(running))
                print("\tNUMBER:\t\t%i"%(self.readRegister("%s.NUMBER"%(contBase))))
                print("\tTYPE:\t\t%i"%(self.readRegister("%s.TYPE"%(contBase))))
        else:
            contBase = "GEM_AMC.TTC.GENERATOR"
            running = self.readRegister("%s.ENABLE"%(contBase))
            if display:
                print("Info for %s"%(contBase))
                print("\tCYCLIC_L1A_GAP: \t\t%i"%(self.readRegister("%s.CYCLIC_L1A_GAP"%(contBase))))
                print("\tCYCLIC_CALPULSE_TO_L1A_GAP: \t%i"%(self.readRegister("%s.CYCLIC_CALPULSE_TO_L1A_GAP"%(contBase))))
                print("\tENABLE: \t\t\t%i"%(running))
        return running

    def getLinkVFATMask(self,ohN):
        """
        V3 electronics only

        Returns a 24 bit number that can be used as the VFAT Mask
        for the Optohybrid ohN
        """
        if self.fwVersion < 3:
            print("HwAMC::getLinkVFATMask() - No support in v2b FW")
            return os.EX_USAGE

        mask = self.getOHVFATMask(ohN)
        if mask = 0xffffffff:
            raise Exception("RPC response was overflow, failed to determine VFAT mask for OH{0}".format(ohN))
        else:
            return mask

    def getMultiLinkVFATMask(self,ohMask=0xfff):
        """
        v3 electronics only

        Returns a ctypes array of c_uint32 of size 12.  Each element of the
        array is the vfat mask for the ohN defined by the array index

        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid
        """

        if self.fwVersion < 3:
            print("HwAMC::getLinkVFATMask() - No support in v2b FW")
            return os.EX_USAGE

        vfatMaskArray = (c_uint32 * 12)()
        rpcResp = self.getOHVFATMaskMultiLink(ohMask, vfatMaskArray)

        if rpcResp != 0:
            raise Exception("RPC response was non-zero, failed to determine VFAT masks for ohMask {0}".format(hex(ohMask)))
        else:
            return vfatMaskArray

    def readBlock(register, nwords, debug=False):
        """
        read block 'register'
        returns 'nwords' values in the register
        """
        global gRetries
        nRetries = 0
        m_node = getNode(register)
        if m_node is None:
            print colors.MAGENTA,"NODE %s NOT FOUND" %(register),colors.ENDC
            return 0x0
     
        p = (c_uint32*nwords)()
        words = []
        if (debug):
            print "reading %d words from register %s"%(nwords,register)
        res = rBlock(m_node.real_address,p,len(p))
        if (res == 0):
            words = list(p)
            if (debug):
                print "ReadBlock result:\n"
                print words
            return words
        else:
            print colors.RED, "error encountered, retried read operation (%d)"%(nRetries),colors.ENDC
            nRetries += 1
        return words

    def readRegister(self, register, debug=False):
        """
        read register 'register' using remote procedure call
        returns value of the register
        """
        global gRetries
        nRetries = 0
        m_node = getNode(register)
        if m_node is None:
            print colors.MAGENTA,"NODE %s NOT FOUND" %(register),colors.ENDC
            return 0x0
        elif 'r' not in m_node.permission:
            print colors.MAGENTA,"No read permission for register %s" %(register),colors.ENDC
            return 0x0
        if debug:
            print "Trying to read\n"
            print m_node.output()
        while (nRetries<gRetries):
            res = rReg(parseInt(m_node.real_address))
            if res == 0xdeaddead:
                print colors.MAGENTA,"Bus error encountered while reading (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),colors.ENDC
                continue
            else:
                if m_node.mask is not None:
                    shift_amount=0
                    for bit in reversed('{0:b}'.format(m_node.mask)):
                        if bit=='0': shift_amount+=1
                        else: break
                    fin_res = (res&m_node.mask)>>shift_amount
                else:
                    fin_res = res
                if debug: print "Read register result: %s" %(fin_res)
                return fin_res
        return 0xdeaddead

    def toggleTTCGen(self, ohN, enable):
        """
        v3  electronics: enable = true (false) ignore (take) ttc commands from backplane for this AMC
        v2b electronics: enable = true (false) start (stop) the T1Controller for link ohN
        """

        return self.ttcGenToggle(ohN, enable)

    def writeRegister(self, register, value, debug=False):
        """
        write value 'value' into register 'register' using remote procedure call
        """
        global gRetries
        nRetries = 0
        #m_node = self.getNode(register)
        m_node = getNode(register)
        if m_node is None:
            print colors.MAGENTA,"NODE %s NOT FOUND"%(register),colors.ENDC
            return 0x0
    
        if debug:
            print "Trying to write\n"
            print m_node.output()
     
        while (nRetries < gMAX_RETRIES):
            rsp = writeReg(m_node, value)
            if "permission" in rsp:
                print colors.MAGENTA,"NO WRITE PERMISSION",colors.ENDC
                return
            elif "Error" in rsp:
                print colors.MAGENTA,"write error encountered (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),colors.ENDC
                return
            elif "0xdeaddead" in rsp:
                print colors.MAGENTA,"0xdeaddead found (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),colors.ENDC
                return
            else:
                return

    def writeRegisterList(self, regs_with_vals, debug=False):
        """
        write value 'value' into register 'register' using remote procedure call
        from an input dict
        """
        global gRetries
        nRetries = 0
        while (nRetries < gMAX_RETRIES):
            for reg in regs_with_vals.keys():
                self.writeRegister(reg,regs_with_vals[reg],debug)
            return
