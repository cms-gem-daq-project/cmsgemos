from ctypes import *
from gempython.utils.gemlogger import colors, printRed, printYellow
from gempython.utils.wrappers import runCommand, runCommandWithOutput

from reg_utils.reg_interface.common.jtag import initJtagRegAddrs
from reg_utils.reg_interface.common.reg_base_ops import rpc_connect, rReg, writeReg
from reg_utils.reg_interface.common.reg_xml_parser import getNode, parseInt, parseXML
from reg_utils.reg_interface.common.sca_utils import sca_reset 

from xhal.reg_interface_gem.core.reg_extra_ops import rBlock

import logging, os

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
        #33:(0xff,"CFG_CAL_DAC"), # as voltage; removing, harder to convert to charge
        34:(0xff,"CFG_BIAS_PRE_VREF"),
        35:(0xff,"CFG_THR_ARM_DAC"),
        36:(0xff,"CFG_THR_ZCC_DAC"),
        39:(0x3,"CFG_VREF_ADC")
        #41:(0x3f,""))Don't know reg in CTP7 address space
        }

gbtValueArray = c_uint32 * 3
class OHLinkMonitorParams(Structure):
    _fields_ = [
            ("gbtRdy",gbtValueArray),
            ("gbtNotRdy",gbtValueArray),
            ("gbtRxOverflow",gbtValueArray),
            ("gbtRxUnderflow",gbtValueArray)
            ]
OHLinkMonitorArrayType = OHLinkMonitorParams * 12

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

vfatValueArray = c_uint32 * 24
class VFATLinkMonitorParams(Structure):
    _fields_ = [
            ("daqCRCErrCnt",vfatValueArray),
            ("daqEvtCnt",vfatValueArray),
            ("syncErrCnt",vfatValueArray)
            ]
VFATLinkMonitorArrayType = VFATLinkMonitorParams * 12

class NoUnmaskedOHException(Exception):
    def __init__(self, message, errors):
        super(NoUnmaskedOHException, self).__init__(message)

        self.errors = errors
        return

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
        if (("gem" in cardName) and ("shelf" in cardName) and ("amc" in cardName)):
            # Geographic addressing used, extract HW Info
            hwInfo      = cardName.split("-")
            self.shelf  = int(hwInfo[1].strip("shelf"))
            self.slot   = int(hwInfo[2].strip("amc"))
            if debug:
                printYellow("Initializing shelf{0}:slot{1}".format(self.shelf,self.slot))
        else:
            self.slot   = 0
            self.shelf  = 0 

        # Open the foreign function library
        self.lib = CDLL("librpcman.so")
        
        # Define Link Monitoring
        self.getmonGBTLink = self.lib.getmonGBTLink
        self.getmonGBTLink.argTypes = [ OHLinkMonitorArrayType, c_uint, c_uint, c_bool ]
        self.getmonGBTLink.restype = c_uint

        self.getmonOHLink = self.lib.getmonOHLink
        self.getmonOHLink.argTypes = [ OHLinkMonitorArrayType, VFATLinkMonitorArrayType, c_uint, c_uint, c_bool ]
        self.getmonOHLink.restype = c_uint

        self.getmonTRIGGEROHmain = self.lib.getmonTRIGGEROHmain
        self.getmonTRIGGEROHmain.argTypes = [ POINTER(c_uint32), c_uint, c_uint ]
        self.getmonTRIGGEROHmain.restype = c_uint

        self.getmonVFATLink = self.lib.getmonVFATLink
        self.getmonVFATLink.argTypes = [ VFATLinkMonitorArrayType, c_uint, c_uint, c_bool ]
        self.getmonVFATLink.restype = c_uint
        
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
        self.ttcGenToggle.argTypes = [c_uint, c_bool]
        self.ttcGenToggle.restype = c_uint

        # Define SBIT Functionality
        self.readSBits = self.lib.sbitReadOut
        self.readSBits.argTypes = [c_uint, c_uint, c_char_p]
        self.readSBits.restype = c_uint

        self.sbitRateScanMulti = self.lib.sbitRateScan
        self.sbitRateScanMulti.argTypes = [c_uint, c_uint, c_uint, c_uint, c_uint, c_char_p, POINTER(c_uint32), POINTER(c_uint32), POINTER(c_uint32)]
        self.sbitRateScanMulti.restype = c_uint

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

    def configureVFAT3DacMonitorMulti(self, dacSelect, ohMask=None):
        """
        Configure the DAC Monitoring to monitor the register defined by dacSelect
        on all unmasked VFATs for optohybrids given by ohMask.

        Will automatically determine the vfatmask for *each* optohybrid

        dacSelect - An integer defining the monitored register.
                    See VFAT3 Manual GLB_CFG_CTR_4 for details.
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        """
        
        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::configureVFAT3DacMonitorMulti(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        ohVFATMaskArray = self.getMultiLinkVFATMask(ohMask)
        return self.confDacMonitorMulti(ohMask, ohVFATMaskArray, dacSelect)

    def enableL1A(self):
        """
        enables L1A's from backplane for this AMC
        """
        
        self.writeRegister("GEM_AMC.TTC.CTRL.L1A_ENABLE", 0x1)
        return

    def getGBTLinkStatus(self,doReset=False,printSummary=False, ohMask=None):
        """
        Get's the GBT Status and can print a table of the status for each unmasked OH.
        Returns True if all unmasked OH's have all GBT's with:
            1. GBT_READY = 0x1
            2. GBT_NOT_READY = 0x0
            3. RX_HAD_OVERFLOW = 0x0
            4. RX_HAD_UNDERFLOW = 0x0
        
        doReset - Issues a link reset if True
        printSummary - prints a table summarizing the status of the GBT's for each unmasked OH
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        """
        
        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::getGBTLinkStatus(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        gbtMonData = OHLinkMonitorArrayType()
        self.getmonGBTLink(gbtMonData, self.nOHs, ohMask, doReset)

        if(printSummary):
            print("--=======================================--")
            print("-> GEM SYSTEM GBT INFORMATION")
            print("--=======================================--")
            print("")
            pass

        allRdy = 1
        noneNotRdy = 0
        hadOverflow = 0
        hadUnderflow = 0
        for ohN in range(self.nOHs):
            # Skip Masked OH's
            if (not ((ohMask >> ohN) & 0x1)):
                continue

            if(printSummary):
                print("----------OH{0}----------".format(ohN))
                pass

            for gbtN in range(3):
                gbtRdy      = gbtMonData[ohN].gbtRdy[gbtN]
                gbtNotRdy   = gbtMonData[ohN].gbtNotRdy[gbtN]
                gbtRxOver   = gbtMonData[ohN].gbtRxOverflow[gbtN]
                gbtRxUnder  = gbtMonData[ohN].gbtRxUnderflow[gbtN]

                allRdy *= gbtRdy
                noneNotRdy += gbtNotRdy
                hadOverflow += gbtRxOver
                hadUnderflow += gbtRxUnder

                if(printSummary):
                    print("GBT{0}.READY              {1}{2}{3}".format(
                        gbtN,colors.RED if (not gbtRdy) else colors.GREEN,gbtRdy,colors.ENDC))
                    print("GBT{0}.NOT_READY          {1}{2}{3}".format(
                        gbtN,colors.RED if gbtNotRdy else colors.GREEN,gbtNotRdy,colors.ENDC))
                    print("GBT{0}.RX_HAD_OVERFLOW    {1}{2}{3}".format(
                        gbtN,colors.RED if gbtRxOver else colors.GREEN,gbtRxOver,colors.ENDC))
                    print("GBT{0}.RX_HAD_UNDERFLOW   {1}{2}{3}".format(
                        gbtN,colors.RED if gbtRxUnder else colors.GREEN,gbtRxUnder,colors.ENDC))
                    pass
                pass
            pass

        return (allRdy == 1 and noneNotRdy == 0 and hadOverflow == 0 and hadUnderflow == 0)

    def getL1ACount(self):
        return self.readRegister("GEM_AMC.TTC.CMD_COUNTERS.L1A")

    def getLinkVFATMask(self,ohN):
        """
        V3 electronics only

        Returns a 24 bit number that can be used as the VFAT Mask
        for the Optohybrid ohN
        """
        if self.fwVersion < 3:
            printRed("HwAMC::getLinkVFATMask() - No support in v2b FW")
            return os.EX_USAGE

        mask = self.getOHVFATMask(ohN)
        if mask == 0xffffffff:
            raise Exception("RPC response was overflow, failed to determine VFAT mask for OH{0}".format(ohN))
        else:
            return mask

    def getMultiLinkVFATMask(self,ohMask=None):
        """
        v3 electronics only

        Returns a ctypes array of c_uint32 of size 12.  Each element of the
        array is the vfat mask for the ohN defined by the array index

        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        """
        
        if self.fwVersion < 3:
            printRed("HwAMC::getLinkVFATMask() - No support in v2b FW")
            return os.EX_USAGE

        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::getMultiLinkVFATMask(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        vfatMaskArray = (c_uint32 * self.nOHs)()
        rpcResp = self.getOHVFATMaskMultiLink(ohMask, vfatMaskArray)

        if rpcResp != 0:
            raise Exception("RPC response was non-zero, failed to determine VFAT masks for ohMask {0}".format(hex(ohMask)))
        else:
            return vfatMaskArray

    def getOHLinkStatus(self,doReset=False,printSummary=False, ohMask=None):
        #place holder
        printYellow("HwAMC::getOHLinkStatus() not yet implemented")
        return

    def getOHMask(self):
        """
        Gets the OH Mask to use with this AMC
        """
        scaReady = self.readRegister("GEM_AMC.SLOW_CONTROL.SCA.STATUS.READY")
        scaError = self.readRegister("GEM_AMC.SLOW_CONTROL.SCA.STATUS.CRITICAL_ERROR")
        return (scaReady & (~scaError & 0xfff))

    def getShelf(self):
        return self.shelf
    
    def getSlot(self):
        return self.slot

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

    def getTriggerLinkStatus(self,printSummary=False, checkCSCTrigLink=False, ohMask=None):
        """
        Gets the trigger link status for each unmasked OH and returns a dictionary where with keys of ohN and 
        values as the sum of all trigger link status counters.  Only unmasked OH's will exist in this dictionary

        If printSummary is set to True a summary table for the status of each unmasked OH is printed.
        ohMask is a 12 bit number, see checkCSCTrigLink for how each bits are interpreted.
        If checkCSCTrigLink is False the ohMask will define which OH's to query, a 1 in the N^th bit means to query the N^th optohybrid.
        If checkCSCTrigLink is True, the ohMask is expected to have all odd bits set to 0. Only even bits may be nonzero.
         If the N^th even bit is nonzero the trigger link status for that OHN and OHN+1 will be queried.
         In all cases if ohMask is None it will be determined automatically from HwAMC::getOHMask().
         It is expected that the GEM trigger link from the physical optohybrid is going to the OHN fiber slot on this AMC and the
         CSC Trigger Link from the physical optohybrid is going to the OHN+1 fiber slot on this AMC.
        """
        
        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::getTriggerLinkStatus(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        ohMask2Query=ohMask
        # Are we checking CSC Trigger links?
        if (checkCSCTrigLink):
            # If input mask is not even otherwise it quits
            if (int(ohMask)%2!=0):
                printRed("HwAMC::getTriggerLinkStatus(): checkCSCTrigLink=True and ohMask={0}. Checking the CSC trigger link on an odd bit is not allowed.".format(hex(ohMask)))
                exit(os.EX_USAGE)
                pass

            # Set the corresponding odd bits of the nonzero even bits in ohMask2Query to 1
            for ohN in range(0,self.nOHs,2):
                if (not ((ohMask >> ohN) & 0x1)):
                    continue
                ohMask2Query += (0x1 << (ohN+1))
                pass
            pass
        
        linkStatus=0
        arraySize=8*int(self.nOHs)
        linkResult = (c_uint32 * arraySize)()
        self.getmonTRIGGEROHmain(linkResult, self.nOHs, ohMask2Query)

        if(printSummary):
            print("--=======================================--")
            print("-> GEM SYSTEM TRIGGER LINK INFORMATION")
            print("--=======================================--")
            print("")
            pass

        ohSumLinkStatus = {}
        for ohN in range(self.nOHs):
            # Skip Masked OH's
            if (not ((ohMask >> ohN) & 0x1)):
                continue

            if(printSummary):
                print("----------OH{0}----------".format(ohN))
                print("LINK0_MISSED_COMMA_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN] > 0) else colors.GREEN,linkResult[ohN],colors.ENDC))
                print("LINK1_MISSED_COMMA_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN+self.nOHs] > 0) else colors.GREEN,linkResult[ohN+self.nOHs],colors.ENDC))
                print("LINK0_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN+2*self.nOHs] > 0) else colors.GREEN,linkResult[ohN+2*self.nOHs],colors.ENDC))
                print("LINK1_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN+3*self.nOHs] > 0) else colors.GREEN,linkResult[ohN+3*self.nOHs],colors.ENDC))
                print("LINK0_UNDERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN+4*self.nOHs] > 0) else colors.GREEN,linkResult[ohN+4*self.nOHs],colors.ENDC))
                print("LINK1_UNDERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN+5*self.nOHs] > 0) else colors.GREEN,linkResult[ohN+5*self.nOHs],colors.ENDC))
                print("LINK0_SBIT_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN+6*self.nOHs] > 0) else colors.GREEN,linkResult[ohN+6*self.nOHs],colors.ENDC))
                print("LINK1_SBIT_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[ohN+7*self.nOHs] > 0) else colors.GREEN,linkResult[ohN+7*self.nOHs],colors.ENDC))
                pass

            if (checkCSCTrigLink):
                if(printSummary):
                    print("----------OH{0}----------".format(ohN+1))
                    print("LINK0_MISSED_COMMA_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)] > 0) else colors.GREEN,linkResult[(ohN+1)],colors.ENDC))
                    print("LINK1_MISSED_COMMA_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)+self.nOHs] > 0) else colors.GREEN,linkResult[(ohN+1)+self.nOHs],colors.ENDC))
                    print("LINK0_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)+2*self.nOHs] > 0) else colors.GREEN,linkResult[(ohN+1)+2*self.nOHs],colors.ENDC))
                    print("LINK1_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)+3*self.nOHs] > 0) else colors.GREEN,linkResult[(ohN+1)+3*self.nOHs],colors.ENDC))
                    print("LINK0_UNDERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)+4*self.nOHs] > 0) else colors.GREEN,linkResult[(ohN+1)+4*self.nOHs],colors.ENDC))
                    print("LINK1_UNDERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)+5*self.nOHs] > 0) else colors.GREEN,linkResult[(ohN+1)+5*self.nOHs],colors.ENDC))
                    print("LINK0_SBIT_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)+6*self.nOHs] > 0) else colors.GREEN,linkResult[(ohN+1)+6]*self.nOHs,colors.ENDC))
                    print("LINK1_SBIT_OVERFLOW_CNT		{0}0x{1:x}{2}".format(colors.RED if (linkResult[(ohN+1)+7*self.nOHs] > 0) else colors.GREEN,linkResult[(ohN+1)+7*self.nOHs],colors.ENDC))
                    pass

            # Initialize trigger link status container
            ohSumLinkStatus[ohN] = 0
            if (checkCSCTrigLink):
                ohSumLinkStatus[ohN+1] = 0
                pass

            # Sum trigger link status registers per OH
            for idx in range(0,8):
                ohSumLinkStatus[ohN]+=linkResult[ohN+idx*self.nOHs]
                if (checkCSCTrigLink):
                    ohSumLinkStatus[ohN+1]+=linkResult[(ohN+1)+idx*self.nOHs]
                pass
            pass

        return ohSumLinkStatus

    def getVFATLinkStatus(self,doReset=False,printSummary=False, ohMask=None):
        """
        Get's the VFAT link status and can print a table of the status for each unmasked OH.
        Returns True if all unmasked OH's have all VFAT's with:
            1. SYNC_ERR_CNT = 0x0
        
        doReset - Issues a link reset if True
        printSummary - prints a table summarizing the status of the GBT's for each unmasked OH
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        """
        
        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::getVFATLinkStatus(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        vfatMonData = VFATLinkMonitorArrayType()
        self.getmonVFATLink(vfatMonData, self.nOHs, ohMask, doReset)

        if(printSummary):
            print("--=======================================--")
            print("-> GEM SYSTEM VFAT INFORMATION")
            print("--=======================================--")
            print("")
            pass

        totalSyncErrors = 0
        for ohN in range(self.nOHs):
            # Skip Masked OH's
            if (not ((ohMask >> ohN) & 0x1)):
                continue

            if(printSummary):
                print("----------OH{0}----------".format(ohN))
                pass

            for vfatN in range(24):
                nSyncErrors = vfatMonData[ohN].syncErrCnt[vfatN]
                totalSyncErrors += nSyncErrors

                if(printSummary):
                    print("VFAT{0}.SYNC_ERR_CNT {1}{2}{3}".format(
                        vfatN,colors.RED if (nSyncErrors > 0) else colors.GREEN,hex(nSyncErrors),colors.ENDC))
                    pass
                pass
            pass

        return (totalSyncErrors == 0)

    def performDacScanMultiLink(self, dacDataAll, dacSelect, dacStep=1, ohMask=None, useExtRefADC=False):
        """
        Scans the DAC defined by dacSelect for all links on this AMC.  See VFAT3 manual for more details
        on the available DAC selection.

        V3 electronics only.

        dacDataAll - Array of type c_uint32
        dacSelect - Integer which specifies the DAC to scan against the ADC.  See VFAT3 Manual
        dacStep - Step size to scan the DAC with
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        useExtRefADC - If true the DAC scan will be made using the externally referenced ADC on the VFAT3s
        """
        
        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        nUnmaskedOHs = bin(ohMask).count("1")
        if (not (nUnmaskedOHs > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::performDacScanMultiLink(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        # Check we are v3 electronics
        if self.fwVersion < 3:
            printRed("HwAMC::performDacScanMultiLink(): No support for v2b electronics")
            exit(os.EX_USAGE)

        # Check if dacSelect is valid
        if dacSelect not in maxVfat3DACSize.keys():
            printRed("HwAMC::performDacScanMultiLink(): Invalid dacSelect {0} value.  Valid values are:".format(dacScan))
            printYellow(maxVfat3DACSize.keys())
            exit(os.EX_USAGE)

        # Check number of nonzero bits doesn't exceed NOH's
        if nUnmaskedOHs > self.nOHs:
            printRed("HwAMC::performDacScanMultiLink(): Number of unmasked OH's {0} exceeds max number of OH's {1}".format(nUnmaskedOHs,self.nOHs))
            exit(os.EX_USAGE)

        # Check length of results container
        lenExpected = self.nOHs * (maxVfat3DACSize[dacSelect][0] - 0+1)*24 / dacStep
        if (len(dacDataAll) != lenExpected):
            printRed("HwAMC::performDacScanMultiLink(): I expected container of length {0} but provided 'dacDataAll' has length {1}",format(lenExpected, len(dacDataAll)))
            exit(os.EX_USAGE)

        return self.dacScanMulti(ohMask, self.nOHs, dacSelect, dacStep, useExtRefADC, dacDataAll)

    def performSBITRateScanMultiLink(self, outDataDacVal, outDataTrigRate, outDataTrigRatePerVFAT, chan=128, dacMin=0, dacMax=254, dacStep=1, ohMask=None, scanReg="THR_ARM_DAC"):
        """
        Measures the rate of sbits sent by all unmasked optobybrids on this AMC

        V3 electronics only.
        
        outDataDacVal           - Array of type c_uint32, array size must be:
                                  (12 * (dacMax - dacMin + 1) / stepSize)
                                  The i^th position here is the DAC value that
                                  the i^th rate in outDataTrigRate was obtained at
        outDataTrigRate         - As outDataDacVal but for trigger rate
        outDataTrigRatePerVFAT  - As outDataTrigRate but for each VFAT, array size
                                  must be:
                                  (24 * (12 * (dacMax - dacMin + 1) / stepSize))
        chan                    - VFAT channel to be considered, for all channels
                                  set to 128
        dacMin                  - Starting dac value of the scan
        dacMax                  - Ending dac value of the scan
        dacStep                 - Step size for moving from dacMin to dacMax
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        scanReg                 - Name of register to be scanned.
        """
        
        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        nUnmaskedOHs = bin(ohMask).count("1")
        if (not (nUnmaskedOHs > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::performSBITRateScanMultiLink(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        # Check we are v3 electronics
        if self.fwVersion < 3:
            printRed("HwAMC::performSBITRateScanMultiLink(): No support for v2b electronics")
            exit(os.EX_USAGE)

        # Check number of nonzero bits doesn't exceed NOH's
        if nUnmaskedOHs > self.nOHs:
            printRed("HwAMC::performSBITRateScanMultiLink(): Number of unmasked OH's {0} exceeds max number of OH's {1}".format(nUnmaskedOHs,self.nOHs))
            exit(os.EX_USAGE)

        # Check length of results container - outDataDacVal
        lenExpected = self.nOHs * (dacMax - dacMin + 1) / dacStep
        if (len(outDataDacVal) != lenExpected):
            printRed("HwAMC::performSBITRateScanMultiLink(): I expected container of length {0} but provided 'outDataDacVal' has length {1}".format(lenExpected, len(outDataDacVal)))
            exit(os.EX_USAGE)

        # Check length of results container - outDataTrigRate
        if (len(outDataTrigRate) != lenExpected):
            printRed("HwAMC::performSBITRateScanMultiLink(): I expected container of length {0} but provided 'outDataTrigRate' has length {1}".format(lenExpected, len(outDataTrigRate)))
            exit(os.EX_USAGE)

        # Check length of results container - outDataTrigRatePerVFAT
        if (len(outDataTrigRatePerVFAT) != (24*lenExpected)):
            printRed("HwAMC::performSBITRateScanMultiLink(): I expected container of length {0} but provided 'outDataTrigRatePerVFAT' has length {1}".format(24*lenExpected, len(outDataTrigRatePerVFAT)))
            exit(os.EX_USAGE)

        return self.sbitRateScanMulti(ohMask, dacMin, dacMax, dacStep, chan, scanReg, outDataDacVal, outDataTrigRate, outDataTrigRatePerVFAT)
    
    def programAllOptohybridFPGAs(self, maxIter=5, ohMask=None):
        """
        Will make up to maxIter attempts to program the FPGA of all unmasked optohybrids.
        Before the first attempt the function will check on the AMC that the PROMLESS programming
        is enabled, if it isn't this will call gemloader_configure.sh on the AMC. Then
        for each attempt a TTC Hard Reset will be sent from the TTC Generator and then it will
        check if slow control with the unmasked OH FPGA's is possible.  If it is not, an SCA reset
        will be sent and then the next attempt will be tried.

        It will return a list of OH's, out of ohMask, who after maxIter is performed are still
        unprogrammed. If all OH's in ohMask are programmed before maxIter is reached the procedure will exit
        and return an empty list.

        maxIter- Maximum number of attempts to program all OH's in ohMask
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        """

        # Determine if PROM-Less programming is enabled, if not enable it
        mpeekCmd = "mpeek 0x6a000000"
        shellCmd = [
                'ssh',
                'gemuser@{0}'.format(self.name),
                'sh -c "{0}"'.format(mpeekCmd)
                ]
        promlessEnabled = runCommandWithOutput(shellCmd).strip('\n')
        if "0x" in promlessEnabled:
            promlessEnabled = int(promlessEnabled,16)
        else:
            promlessEnabled = int(promlessEnabled)
            pass

        if promlessEnabled != 0x1:
            shellCmd = [
                    'ssh',
                    'gemuser@{0}'.format(self.name),
                    'sh -c "/mnt/persistent/gemdaq/gemloader/gemloader_configure.sh"'
                    ]
            runCommand(shellCmd)

        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::programAllOptohybridFPGAs(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        # Program FPGA's
        self.writeRegister("GEM_AMC.TTC.GENERATOR.ENABLE",0x1)
        initJtagRegAddrs()
        for trial in range(0,maxIter):
            self.writeRegister("GEM_AMC.TTC.GENERATOR.SINGLE_HARD_RESET",0x1)
            isDead = True
            listOfDeadFPGAs = []
            ohMaskNeedSCAReset = 0x0
            for ohN in range(self.nOHs):
                # Skip masked OH's
                if( not ((ohMask >> ohN) & 0x1)):
                    continue
                fwVerMaj = self.readRegister("GEM_AMC.OH.OH{0}.FPGA.CONTROL.RELEASE.VERSION.MAJOR".format(ohN))
                if fwVerMaj != 0xdeaddead:
                    isDead = False
                else:
                    isDead = True
                    listOfDeadFPGAs.append(ohN)
                    ohMaskNeedSCAReset += (0x1 << ohN)
                    pass
                pass

            if not isDead:
                fpgaCommPassed = True
                break
            else:
                sca_reset(ohMaskNeedSCAReset)
            pass
        self.writeRegister("GEM_AMC.TTC.GENERATOR.ENABLE",0x0)

        return listOfDeadFPGAs

    def readADCsMultiLink(self, adcDataAll, useExtRefADC=False, ohMask=None, debug=False):
        """
        Reads the ADC value from all unmasked VFATs

        adcDataAll - Array of type c_uint32 of size 24*12=288
        useExtRefADC - True (False) use the externally (internally) referenced ADC
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        """

        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::readADCsMultiLink(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        if debug:
            print("getting vfatmasks for each OH")

        ohVFATMaskArray = self.getMultiLinkVFATMask(ohMask)
        if debug:
            print("| ohN | vfatmask |")
            print("| :-: | :------: |")
            for ohN in range(0,12):
                print("| {0} | 0x{1:x} |".format(ohN, ohVFATMaskArray[ohN]))

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
                if debug:
                    print colors.MAGENTA,"Bus error encountered while reading (%s), retrying operation (%d,%d)"%(register,nRetries,gRetries),colors.ENDC
                nRetries+=1
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

    def scaMonitorMultiLink(self, ohMask=None):
        """
        v3 electronics only.
        Reads SCA monitoring data for multiple links on the AMC

        NOH    - number of OH's on this AMC
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        """
        
        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::scaMonitorMultiLink(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        scaMonData = SCAMonitorArrayType()
        rpcResp = self.getmonOHSCAmain(scaMonData, self.nOHs, ohMask)

        if rpcResp != 0:
            raise Exception("RPC response was non-zero, reading SCA Monitoring Data from OH's in ohMask = 0x{0:x} failed".format(ohMask))

        return scaMonData

    def setShelf(self,shelf):
        self.shelf = shelf
        return

    def setSlot(self,slot):
        self.slot = slot
        return

    def sysmonMonitorMultiLink(self, NOH=12, ohMask=None, doReset=False):
        """
        v3 eletronics only.
        Reads FPGA sysmon data for multiple links on the AMC

        NOH - number of OH's on this AMC
        ohMask - Mask which defines which OH's to query; 12 bit number where
                 having a 1 in the N^th bit means to query the N^th optohybrid.
                 If None will be determined automatically using HwAMC::getOHMask()
        doReset - Resets the sysmon alarm counters (generally unwise)
        """

        # Automatically determine ohMask if not provided
        if ohMask is None:
            ohMask = self.getOHMask()

        # Are any optohybrids available?
        if (not (bin(ohMask).count("1") > 0) ):
            raise NoUnmaskedOHException("{0}HwAMC::sysmonMonitorMultiLink(): there are no unmasked optohybrids{1}".format(colors.RED,colors.ENDC),os.EX_SOFTWARE)

        sysmonData = SysmonMonitorArrayType()
        rpcResp = self.getmonOHSysmon(sysmonData, NOH, ohMask, doReset)

        if rpcResp != 0:
            raise Exception("RPC response was non-zero, reading Sysmon Monitoring Data from OH's in ohMask = 0x{0:x} failed".format(ohMask))

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
