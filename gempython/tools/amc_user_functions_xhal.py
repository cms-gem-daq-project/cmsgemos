from ctypes import *
from gempython.utils.gemlogger import colors

from reg_utils.reg_interface.common.reg_base_ops import rpc_connect, rReg, writeReg
from reg_utils.reg_interface.common.reg_xml_parser import getNode, parseInt, parseXML

from xhal.reg_interface_gem.core.reg_extra_ops import rBlock

import logging

gMAX_RETRIES = 5
gRetries = 5

maxVfat3DACSize = {
        #ADC Measures Current
        #0:(0x3f, "CFG_IREF"), # This should never be scanned per VFAT3 Team's Instructions
        1:(0xff,"CFG_CAL_DAC"), # as current
        2:(0xff,"CFG_BIAS_PRE_I_BIT"),
        3:(0x3f,"CFG_BIAS_PRE_I_BLCC"),
        4:(0x3f,"CFG_BIAS_PRE_I_BSF"),
        5:(0xff,"CFG_BIAS_SH_I_BFCAS"),
        6:(0xff,"CFG_BIAS_SH_I_BDIFF"),
        7:(0xff,"CFG_BIAS_SD_I_BDIFF"),
        8:(0xff,"CFG_BIAS_SD_I_BFCAS"),
        9:(0x3f,"CFG_BIAS_SD_I_BSF"),
        10:(0x3f,"CFG_BIAS_CFD_DAC_1"),
        11:(0x3f,"CFG_BIAS_CFD_DAC_2"),
        12:(0x3f,"CFG_HYST"),
        14:(0xff,"CFG_THR_ARM_DAC"),
        15:(0xff,"CFG_THR_ZCC_DAC"),
        #16:(0xff,""),Don't know reg in CTP7 address space

        #ADC Measures Voltage
        33:(0xff,"CFG_CAL_DAC"), # as voltage
        34:(0xff,"CFG_BIAS_PRE_VREF"),
        35:(0xff,"CFG_THR_ARM_DAC"),
        36:(0xff,"CFG_THR_ZCC_DAC"),
        39:(0x3,"CFG_ADC_VREF")
        #41:(0x3f,""))Don't know reg in CTP7 address space
        }

ohBoardTempArray = c_uint32 * 9 # Temperature Array
class SCAMonitorParams(Structure):
    _fields_ = [
            ("ohBoardTemp", ohBoardTempArray),
            ("ohFPGACoreTemp", c_uint32),
            ("scaTemp", c_uint32),
            ("AVCCN", c_uint32),
            ("AVTTN", c_uint32),
            ("voltage1V0_INT", c_uint32),
            ("voltage1V8F", c_uint32),
            ("voltage1V5", c_uint32),
            ("voltage2V5_IO", c_uint32),
            ("voltage3V0", c_uint32),
            ("voltage1V8", c_uint32),
            ("VTRX_RSSI2", c_uint32),
            ("VTRX_RSSI1", c_uint32)
            ]
SCAMonitorArrayType = SCAMonitorParams * 12 # SCA Monitor Array

class SysmonMonitorParams(Structure):
    _fields_ = [
            ("isOverTemp", c_bool),
            ("isInVCCAuxAlarm", c_bool),
            ("isInVCCIntAlarm", c_bool),
            ("cntOverTemp", c_uint32),
            ("cntVCCAuxAlarm", c_uint32),
            ("cntVCCIntAlarm", c_uint32),
            ("fpgaCoreTemp", c_uint32),
            ("fpgaCore1V0", c_uint32),
            ("fpgaCore2V5_IO", c_uint32)
            ]
SysmonMonitorArrayType = SysmonMonitorParams * 12 # Sysmon Monitor Array

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

        # Open the foreign function library
        self.lib = CDLL("librpcman.so")
       
        # Define VFAT3 DAC Monitoring
        self.confDacMonitorMulti = self.lib.configureVFAT3DacMonitorMultiLink
        self.confDacMonitorMulti.argTypes = [ c_uint, POINTER(c_uint32), c_uint ]
        self.confDacMonitorMulti.restype = c_uint

        self.readADCsMulti = self.lib.readVFAT3ADCMultiLink
        self.readADCsMulti.argTypes = [ c_uint, POINTER(c_uint32), POINTER(c_uint), c_bool ]
        self.readADCsMulti.restype = c_uint

        self.dacScanMulti = self.lib.dacScanMultiLink
        self.dacScanMulti.argTypes = [ c_uint, c_uint, c_uint, c_uint, c_bool, POINTER(c_uint) ]
        self.dacScanMulti.restype = c_uint

        # Define SCA & Sysmon Monitoring
        self.getmonOHSCAmain = self.lib.getmonOHSCAmain
        self.getmonOHSCAmain.argTypes = [ SCAMonitorArrayType, c_uint, c_uint ]
        self.getmonOHSCAmain.restype = c_uint

        self.getmonOHSysmon = self.lib.getmonOHSysmon
        self.getmonOHSysmon.argTypes = [ SysmonMonitorArrayType, c_uint, c_uint, c_bool ]
        self.getmonOHSysmon.restype = c_uint

        # Define Get OH Mask functionality
        self.getOHVFATMask = self.lib.getOHVFATMask
        self.getOHVFATMask.argTypes = [ c_uint ]
        self.getOHVFATMask.restype = c_uint

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
        self.nOHs = self.readRegister("GEM_AMC.GEM_SYSTEM.CONFIG.NUM_OF_OH")
        self.fwVersion = self.readRegister("GEM_AMC.GEM_SYSTEM.RELEASE.MAJOR")
        if debug:
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
        """
        Configure the DAC Monitoring to monitor the register defined by dacSelect
        on all unmasked VFATs for optohybrids given by ohMask.

        Will automatically determine the vfatmask for *each* optohybrid

        dacSelect - An integer defining the monitored register.
                    See VFAT3 Manual GLB_CFG_CTR_4 for details.
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid
        """

        ohVFATMaskArray = self.getMultiLinkVFATMask(ohMask)
        return self.confDacMonitorMulti(ohMask, ohVFATMaskArray, dacSelect)

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
        if mask == 0xffffffff:
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

    def performDacScanMultiLink(self, dacDataAll, dacSelect, dacStep=1, ohMask=0x3ff, useExtRefADC=False):
        """
        Scans the DAC defined by dacSelect for all links on this AMC.  See VFAT3 manual for more details
        on the available DAC selection.

        V3 electronics only.

        dacDataAll - Array of type c_uint32
        dacSelect - Integer which specifies the DAC to scan against the ADC.  See VFAT3 Manual
        dacStep - Step size to scan the DAC with
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid
        useExtRefADC - If true the DAC scan will be made using the externally referenced ADC on the VFAT3s
        """

        # Check we are v3 electronics
        if self.fwVersion < 3:
            print("HwAMC::performDacScanMultiLink(): No support for v2b electronics")
            exit(os.EX_USAGE)

        # Check if dacSelect is valid
        if dacSelect not in maxVfat3DACSize.keys():
            print("HwAMC::performDacScanMultiLink(): Invalid dacSelect {0} value.  Valid values are:".format(dacScan))
            print(maxVfat3DACSize.keys())
            exit(os.EX_USAGE)

        # Check number of nonzero bits doesn't exceed NOH's
        nUnmaskedOHs = bin(ohMask).count("1")
        if nUnmaskedOHs > self.nOHs:
            print("HwAMC::performDacScanMultiLink(): Number of unmasked OH's {0} exceeds max number of OH's {1}".format(nUnmaskedOHs,self.nOHs))
            exit(os.EX_USAGE)

        # Check length of results container
        lenExpected = self.nOHs * (maxVfat3DACSize[dacSelect][0] - 0+1)*24 / dacStep
        if (len(dacDataAll) != lenExpected):
            print("HwAMC::performDacScanMultiLink(): I expected container of lenght {0} but provided 'dacDataAll' has length {1}",format(lenExpected, len(dacDataAll)))
            exit(os.EX_USAGE)

        return self.dacScanMulti(ohMask, self.nOHs, dacSelect, dacStep, useExtRefADC, dacDataAll)

    def readADCsMultiLink(self, adcDataAll, useExtRefADC=False, ohMask=0xFFF, debug=False):
        """
        Reads the ADC value from all unmasked VFATs

        adcDataAll - Array of type c_uint32 of size 24*12=288
        useExtRefADC - True (False) use the externally (internally) referenced ADC
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid
        """

        if debug:
            print("getting vfatmasks for each OH")

        ohVFATMaskArray = self.getMultiLinkVFATMask(ohMask)
        if debug:
            print("| ohN | vfatmask |")
            print("| :-: | :------: |")
            for ohN in range(0,12):
                mask = str(hex(ohVFATMaskArray[ohN])).strip('L')
                print("| {0} | {1} |".format(ohN, mask))

        return self.readADCsMulti(ohMask,ohVFATMaskArray, adcDataAll, useExtRefADC)

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

    def scaMonitorToggle(self, ohMask=0x0, debug=False):
        """
        Toggle the SCA Monitoring for OH's on this AMC

        ohMask - 12 bit number where N^th bit corresponds to N^th OH.  Setting a bit to 1 will suspend ADC monitoring on that SCA
        """

        self.writeRegister("GEM_AMC.SLOW_CONTROL.SCA.ADC_MONITORING.MONITORING_OFF",ohMask,debug)

    def scaMonitorMultiLink(self, NOH=12, ohMask=0xfff):
        """
        v3 electronics only.
        Reads SCA monitoring data for multiple links on the AMC

        NOH - number of OH's on this AMC
        ohMask - 12 bit number where N^th bit corresponds to N^th OH.  Setting a bit to 1 will cause the SCA data to be monitored for this OH.
        """

        scaMonData = SCAMonitorArrayType()
        rpcResp = self.getmonOHSCAmain(scaMonData, NOH, ohMask)

        if rpcResp != 0:
            raise Exception("RPC response was non-zero, reading SCA Monitoring Data from OH's in ohMask = {0} failed".format(str(hex(args.ohMask)).strip('L')))

        return scaMonData

    def sysmonMonitorMultiLink(self, NOH=12, ohMask=0xfff, doReset=False):
        """
        v3 eletronics only.
        Reads FPGA sysmon data for multiple links on the AMC

        NOH - number of OH's on this AMC
        ohMask - 12 bit number where N^th bit corresponds to N^th OH.  Setting a bit to 1 will cause the SCA data to be monitored for this OH.
        doReset - Resets the sysmon alarm counters (generally unwise)
        """

        sysmonData = SysmonMonitorArrayType()
        rpcResp = self.getmonOHSysmon(sysmonData, NOH, ohMask, doReset)

        if rpcResp != 0:
            raise Exception("RPC response was non-zero, reading Sysmon Monitoring Data from OH's in ohMask = {0} failed".format(str(hex(args.ohMask)).strip('L')))

        return sysmonData

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
