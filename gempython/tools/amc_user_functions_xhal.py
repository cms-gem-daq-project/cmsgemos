import os, signal, sys
sys.path.append('${GEM_PYTHON_PATH}')

from ctypes import *
from gempython.utils.gemlogger import colormsg
from gempython.utils.nesteddict import nesteddict
from gempython.utils.registers_xhal import *
from gempython.utils.wrappers import envCheck
from time import sleep

import logging

gMAX_RETRIES = 5
gRetries = 5

class colors:
        WHITE   = '\033[97m'
        CYAN    = '\033[96m'
        MAGENTA = '\033[95m'
        BLUE    = '\033[94m'
        YELLOW  = '\033[93m'
        GREEN   = '\033[92m'
        RED     = '\033[91m'
        ENDC    = '\033[0m'

class ctp7Params:
    # Key (shelf,slot); val = eagleXX
    # See: https://twiki.cern.ch/twiki/bin/view/CMS/GEMDAQExpert#List_of_CTP7_s
    cardLocation = { 
            (3,2):"eagle26", #QC8
            (3,5):"eagle60",
            #(?,?):"eagle61",
            (1,3):"eagle33", #P5
            (1,2):"eagle34"} #Coffin
            #(1,?):"eagle64"}

class HwAMC:
    def __init__(self, slot, shelf=1, debug=False):
        """
        Initialize the HW board an open an RPC connection
        """
        envCheck("XHAL_ROOT")
        self.addrTable = os.getenv("XHAL_ROOT")+'/etc/gem_amc_top.xml'
        
        # Debug flag
        self.debug = debug
        
        # Logger
        self.amclogger = logging.getLogger(__name__)

        # Store HW info
        self.name = ctp7Params.cardLocation[(shelf,slot)]
        self.shelf = shelf
        self.slot = slot

        # Define the connection
        self.lib = CDLL(os.getenv("XHAL_ROOT")+"/lib/x86_64/librpcman.so")
        self.rpc_connect = self.lib.init
        self.rpc_connect.argtypes = [c_char_p]
        self.rpc_connect.restype = c_uint
       
        # Define read register
        self.rReg = self.lib.getReg
        self.rReg.restype = c_uint
        self.rReg.argtypes=[c_uint]
        
        # Define read block
        self.rBlock = self.lib.getBlock
        self.rBlock.restype = c_uint
        self.rBlock.argtypes=[c_uint,POINTER(c_uint32)]

        # Define write register
        self.wReg = self.lib.putReg
        self.wReg.argtypes=[c_uint,c_uint]

        # Define TTC Functions
        self.ttcGenConf = self.lib.ttcGenConf
        self.ttcGenConf.restype = c_uint
        self.ttcGenConf.argtypes = [c_uint, c_uint, c_uint, c_uint, c_uint, c_uint, c_bool]

        self.ttcGenToggle = self.lib.ttcGenToggle
        self.ttcGenToggle.restype = c_uint
        self.ttcGenToggle.argtypes = [c_uint, c_bool]

        # Parse XML
        self.nodes = parseXML(self.addrTable)

        # Open RPC Connection
        print "Initializing AMC"
        self.rpc_connect(self.name)
        self.fwVersion = self.readRegister("GEM_AMC.GEM_SYSTEM.RELEASE.MAJOR") 
        print "My FW release major = ", self.fwVersion

        return

    def blockL1A(self):
        """
        v3  electronics: blocks all real ttc commands from backplane for this AMC
        v2b electronics: blocks L1A's being sent to this AMC's OH's
        """
        
        if self.fwVersion < 3:
            self.writeRegister("GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x0)
        else:
            self.toggleTTC(ohN=-1,enable=True)
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

    def enableL1A(self):
        """
        v3  electronics: enables real ttc commands from backplane for this AMC
        v2b electronics: enables L1A's being sent to this AMC's OH's
        """
        
        if self.fwVersion < 3:
            self.writeRegister(device, "GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x1)
        else:
            #Yes False, this turns OFF the TTC Generator which suppresses ttc cmds from backplane
            self.toggleTTC(ohN=-1,enable=False) 
        return

    def getL1ACount(self):
        return self.readRegister("GEM_AMC.TTC.CMD_COUNTERS.L1A")

    def getNode(self,nodeName):
        #return next((node for node in self.nodes if node.name == nodeName),None)
        try: 
            return self.nodes[nodeName]
        except KeyError:
            print "Node %s not found" %(nodeName)
            return None

    def readAddress(self,address):
        output = self.rReg(address) 
        return '{0:#010x}'.format(parseInt(str(output)))

    def readBlock(register, nwords, debug=False):
        """
        read block 'register' from uhal device 'device'
        returns 'nwords' values in the register
        """
        global gRetries
        nRetries = 0
        m_node = self.getNode(register)
        if m_node is None:
            print colors.MAGENTA,"NODE %s NOT FOUND" %(register),colors.ENDC
            return 0x0
    
        #if debug:
        #    print "Trying to read block of %d words\n" %(nwords)
        #    print m_node.output()
     
        p = (c_uint32*nwords)()
        words = []
        #while (nRetries < gMAX_RETRIES):
        if (debug):
            print "reading %d words from register %s"%(nwords,register)
            pass
        #for i in range(nwords):
        #   words.append(readRegister(device,register, False))
        #if (debug):
        #    print "reading result: %s" %(words)
        #    pass
        res = self.rBlock(m_node.real_address,p,len(p))
        if (res == 0):
            words = list(p)
            if (debug):
                print "ReadBlock result:\n"
                print words
                pass
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
        m_node = self.getNode(register)
        if m_node is None:
            print colors.MAGENTA,"NODE %s NOT FOUND" %(register),colors.ENDC
            return 0x0
        elif 'r' not in m_node.permission:
            print colors.MAGENTA,"No read permission for register %s" %(register),colors.ENDC
            return 0x0
        if debug:
            print "Trying to read\n"
            print m_node.output()
            pass
        while (nRetries<gRetries):
            res = self.rReg(parseInt(m_node.real_address))
            if res == 0xdeaddead:
                print colors.MAGENTA,"Bus error encountered while reading (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),colors.ENDC
                continue
                #return 0xdeaddead
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

    def toggleTTC(self, ohN, enable):
        """
        v3  electronics: enable = true (false) ignore (take) ttc commands from backplane for this AMC
        v2b electronics: enable = true (false) start (stop) the T1Controller for link ohN
        """

        return self.ttcGenToggle(ohN, enable)

    def writeReg(self, reg, value):
        address = reg.real_address
        if 'w' not in reg.permission:
            return 'No write permission!'
        # Apply Mask if applicable
        print "Initial value to write: %s, register %s"% (value,reg.name)
        if reg.mask is not None:
            shift_amount=0
            for bit in reversed('{0:b}'.format(reg.mask)):
                if bit=='0': shift_amount+=1
                else: break
            shifted_value = value << shift_amount
            for i in range(10):
                initial_value = self.readAddress(address)
                try: initial_value = parseInt(initial_value) 
                except ValueError: return 'Error reading initial value: '+str(initial_value)
                if initial_value == 0xdeaddead:
                    print "Writing masked reg %s : Error while reading, retry attempt (%s)"%(reg.name,i)
                    sleep(0.1)
                    continue
                else: break
            if initial_value == 0xdeaddead:
                 print "Writing masked reg %s failed. Exiting..." %(reg.name)
                 #sys.exit()
            final_value = (shifted_value & reg.mask) | (initial_value & ~reg.mask)
        else: final_value = value
        output = self.wReg(parseInt(address),parseInt(final_value))
        if output != final_value:
            print "Writing masked reg %s failed. Exiting..." %(reg.name)
            print "wReg output %s" % (output)
            #sys.exit()
        return str('{0:#010x}'.format(final_value)).rstrip('L')+'('+str(value)+')\twritten to '+reg.name

    def writeRegister(self, register, value, debug=False):
        """
        write value 'value' into register 'register' using remote procedure call
        """
        global gRetries
        nRetries = 0
        m_node = self.getNode(register)
        if m_node is None:
            print colors.MAGENTA,"NODE %s NOT FOUND"%(register),colors.ENDC
            return 0x0
    
        if debug:
            print "Trying to write\n"
            print m_node.output()
            pass
     
        while (nRetries < gMAX_RETRIES):
            rsp = self.writeReg(m_node, value)
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
            pass
        pass

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
                pass
            return
        pass
