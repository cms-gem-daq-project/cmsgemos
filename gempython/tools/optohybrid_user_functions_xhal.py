from gempython.tools.amc_user_functions_xhal import *

import logging
import os, sys

class OHRPCException(Exception):
    def __init__(self, message, errors):
        super(OHRPCException, self).__init__(message)

        self.errors = errors
        return

class HwOptoHybrid(object):
    def __init__(self, cardName, link, debug=False):
        """
        Initialize the HW board an open an RPC connection
        """
        # Debug flag
        self.debug = debug
        
        # Logger
        self.ohlogger = logging.getLogger(__name__)

        # Store HW info
        self.link = link
        #self.mask = self.getVFATMask()
        self.mask = 0x0
        self.nVFATs = 24
        self.parentAMC = HwAMC(cardName, debug)

        # Define broadcast read
        self.bRead = self.parentAMC.lib.broadcastRead
        self.bRead.argtypes = [c_uint, c_char_p, c_uint, POINTER(c_uint32)]
        self.bRead.restype = c_uint

        # Define broadcast write
        self.bWrite = self.parentAMC.lib.broadcastWrite
        self.bWrite.argtypes = [c_uint, c_char_p, c_uint, c_uint]
        self.bWrite.restype = c_uint
       
        # Define the sbit mapping scan modules
        self.sbitMappingWithCalPulse = self.parentAMC.lib.checkSbitMappingWithCalPulse
        self.sbitMappingWithCalPulse.restype = c_uint
        self.sbitMappingWithCalPulse.argtypes = [c_uint, c_uint, c_uint, c_bool,
                                                 c_bool, c_uint, c_uint, c_uint,
                                                 c_uint, POINTER(c_uint32)]

        self.sbitRateWithCalPulse = self.parentAMC.lib.checkSbitRateWithCalPulse
        self.sbitRateWithCalPulse.restype = c_uint
        self.sbitRateWithCalPulse.argtypes = [c_uint, c_uint, c_uint, c_bool,
                                              c_bool, c_uint, c_uint, c_uint, c_uint,
                                              POINTER(c_uint32), POINTER(c_uint32),
                                              POINTER(c_uint32)]

        # Define the generic scan modules
        self.genScan = self.parentAMC.lib.genScan
        self.genScan.restype = c_uint
        self.genScan.argtypes = [c_uint, c_uint, c_uint, c_uint,
                                 c_uint, c_uint, c_bool, c_bool, c_uint, c_uint,
                                 c_char_p, c_bool, c_bool, POINTER(c_uint32)]
        self.genChannelScan = self.parentAMC.lib.genChannelScan
        self.genChannelScan.restype = c_uint
        self.genChannelScan.argtypes = [c_uint, c_uint, c_uint, c_uint,
                                        c_uint, c_uint, c_bool, c_bool,
                                        c_uint, c_bool, c_char_p, c_bool,
                                        POINTER(c_uint32)]

        self.dacScan = self.parentAMC.lib.dacScan
        self.dacScan.restype = c_uint
        self.dacScan.argtypes = [ c_uint, c_uint, c_uint, c_uint, c_bool, POINTER(c_uint32) ]

        # Define the known V2b electronics scan registers
        self.KnownV2bElScanRegs = [
                    "Latency",
                    "VCal",
                    "VThreshold1",
                    "VThreshold1PerChan",
                    "VThreshold1Trk"
                ]

        return

    def broadcastRead(self,register,mask=0xff000000):
        """
        Perform a broadcast RPC read on the VFATs specified by mask
        Will return when operation has completed
        """
        outData = (c_uint32 * self.nVFATs)()

        try:
            rpcResp = self.bRead(self.link, register, mask, outData)
        except Exception as e:
            print e
            pass

        if 0 != rpcResp:
            raise OHRPCException("broadcastRead failed for device %i; reg: %s; with mask %x"%(self.link,register,mask), os.EX_SOFTWARE)

        return outData
    
    def broadcastWrite(self,register,value,mask=0xff000000):
        """
        Perform a broadcast RPC write on the VFATs specified by mask
        Will return when operation has completed
        """
        
        rpcResp = 0

        try:
            rpcResp = self.bWrite(self.link, register, value, mask)
        except Exception as e:
            print e
            pass

        if 0 != rpcResp:
            raise OHRPCException("broadcastWrite failed for device %i; reg: %s; with mask %x"%(self.link,register,mask), os.EX_SOFTWARE)

        return rpcResp

    def checkSbitMappingWithCalPulse(self, **kwargs):
        """
        Only supported for v3 electronics.

        Sends a calpulse to a given channel and sees which
        sbit is received by the OH, repeats for all channels
        on supplied vfat.

        Only the channel that is being pulsed is unmasked.
        Returns the RPC Message response code.

        List of supported arguments:

        calSF       - V3 electronics only.  The value of the
                      CFG_CAL_FS register to be used when operating in
                      current pulse mode.
        currentPulse- V3 electronics only. The calibration module uses
                      the current pulse mode rather than the voltage
                      pulse mode.
        enableCal   - If true the calpulse is used
        L1AInterval - Number of BX's inbetween L1A's
        mask        - VFAT mask to use for excluding vfats from the trigger
        nevts       - Number of events for each dac value in scan
        outData     - Pointer to an array of size (24*128*8*nevts) which
                      stores the results of the scan:
                            bits [0,7] channel pulsed,
                            bits [8:15] sbit observed,
                            bits [16:20] vfat pulsed,
                            bits [21,25] vfat observed,
                            bit 26 isValid.
                            bits [27,29] are the cluster size
        pulseDelay  - Delay between Calpulse and L1A in BX's
        vfat        - VFAT to pulse
        """

        # Set Defaults
        calSF=0x0
        currentPulse=True
        enableCal=False
        L1AInterval=250
        mask=0x0
        nevts=100
        pulseDelay=40

        # Check if outData is present
        if("outData" not in kwargs):
            print("HwOptoHybrid::checkSbitMappingWithCalPulse(): You must supply an outData Pointer")
            exit(os.EX_USAGE)

        # Check if vfat is present
        if("vfat" not in kwargs):
            print("HwOptoHybrid::checkSbitMappingWithCalPulse(): You must supply the vfat to pulse")
            exit(os.EX_USAGE)

        # Get Parameters
        if "calSF" in kwargs:
            calSF = kwargs["calSF"]
        if "currentPulse" in kwargs:
            currentPulse = kwargs["currentPulse"]
        if "enableCal" in kwargs:
            enableCal = kwargs["enableCal"]
        if "L1AInterval" in kwargs:
            L1AInterval = kwargs["L1AInterval"]
        if "mask" in kwargs:
            mask = kwargs["mask"]
        if "nevts" in kwargs:
            nevts = kwargs["nevts"]
        if "pulseDelay" in kwargs:
            pulseDelay = kwargs["pulseDelay"]

        return self.sbitMappingWithCalPulse(self.link, kwargs["vfat"], mask, enableCal, currentPulse, calSF, nevts, L1AInterval, pulseDelay, kwargs["outData"])

    def checkSbitRateWithCalPulse(self, **kwargs):
        """
        Only supported for v3 electronics.

        Sends cyclic calpulses to a given channel and records
        the rate of sbits measures by the OH, repeats for all
        channels on the supplied vfat.

        Only the channel being pulsed is unmasked.
        Returns the RPC Message response code.

        List of supported arguments:

        calSF           - V3 electronics only.  The value of the
                          CFG_CAL_FS register to be used when operating in
                          current pulse mode.
        currentPulse    - V3 electronics only. The calibration module uses
                          the current pulse mode rather than the voltage
                          pulse mode.
        enableCal       - If true the calpulse is used
        mask            - VFAT mask to use for excluding vfats from the trigger
        outDataCTP7Rate - Pointer to an array of size 3072 where the index
                          is defined as idx = 128 * vfat + chan; this array
                          stores the value of GEM_AMC.TRIGGER.OHX.TRIGGER_RATE
                          for each (vfat,channel) after waitTime
        outDataFPGARate - As outDataCTP7Rate but for the value of the
                          GEM_AMC.OH.OHX.FPGA.TRIG.CNT.CLUSTER_COUNT
                          register for X = self.link
        outDataVFATRate - As outDataCTP7Rate but for the value of the
                          GEM_AMC.OH.OHX.FPGA.TRIG.CNT.VFATY_SBITS register
                          for X = self.link and Y = vfat defined by the array
                          idx convention
        pulseDelay      - Delay between Calpulse and L1A in BX's
        pulseRate       - Rate of calpulses to be sent in Hz
        vfat            - VFAT to pulse
        waitTime        - Time to wait before measuring sbit rate in milliseconds
        """

        # Set Defaults
        calSF=0x0
        currentPulse=True
        enableCal=False
        mask=0x0
        pulseDelay=40
        pulseRate=10000
        waitTime=1000

        # Check if outData pointers are present
        if("outDataCTP7Rate" not in kwargs):
            print("HwOptoHybrid::checkSbitRateWithCalPulse(): You must supply an outDataCTP7Rate Pointer")
            exit(os.EX_USAGE)
        if("outDataFPGARate" not in kwargs):
            print("HwOptoHybrid::checkSbitRateWithCalPulse(): You must supply an outDataFPGARate Pointer")
            exit(os.EX_USAGE)
        if("outDataVFATRate" not in kwargs):
            print("HwOptoHybrid::checkSbitRateWithCalPulse(): You must supply an outDataVFATRate Pointer")
            exit(os.EX_USAGE)

        # Check if vfat is present
        if("vfat" not in kwargs):
            print("HwOptoHybrid::checkSbitMappingWithCalPulse(): You must supply the vfat to pulse")
            exit(os.EX_USAGE)

        # Get Parameters
        if "calSF" in kwargs:
            calSF = kwargs["calSF"]
        if "currentPulse" in kwargs:
            currentPulse = kwargs["currentPulse"]
        if "enableCal" in kwargs:
            enableCal = kwargs["enableCal"]
        if "mask" in kwargs:
            mask = kwargs["mask"]
        if "pulseDelay" in kwargs:
            pulseDelay = kwargs["pulseDelay"]
        if "pulseRate" in kwargs:
            pulseRate = kwargs["pulseRate"]
        if "waitTime" in kwargs:
            waitTime = kwargs["waitTime"]

        return self.sbitRateWithCalPulse(self.link, kwargs["vfat"], mask, enableCal, currentPulse, calSF, waitTime, pulseRate, pulseDelay, kwargs["outDataCTP7Rate"], kwargs["outDataFPGARate"], kwargs["outDataVFATRate"])

    def getL1ACount(self):
        if self.parentAMC.fwVersion < 3:
            return self.parentAMC.readRegister("GEM_AMC.OH.OH%s.COUNTERS.T1.SENT.L1A"%(self.link))
        else:
            print("HwOptoHybrid.getL1ACount(): Presently the CTP7 <-> OHv3 communication is not present")
            print("\tAt the moment the OHv3 FPGA doesn't receive L1As")
            print("\tThis count is meaningless, exiting")
            #return -1
            sys.exit(os.EX_USAGE)

    def getSBitMask(self):
        """
        v3 electronics only

        Gets the sbit mask in the OH FPGA
        """

        if self.parentAMC.fwVersion < 3:
            print("Parent AMC Major FW Version: %i"%(self.parentAMC.fwVersion))
            print("Only implemented for v3 electronics, exiting")
            sys.exit(os.EX_USAGE)

        return self.parentAMC.readRegister("GEM_AMC.OH.OH%i.TRIG.CTRL.VFAT_MASK"%(self.link))

    def getTriggerSource(self):
        """
        v2b electronics only
        Get the trigger source
        OH:   0 = TTC over GTX
              1 = FIRMWARE
              2 = EXTERNAL
              3 = LOOPBACK
              4 = LOGICAL OR
              5 = TTC over GBT
        """

        if self.parentAMC.fwVersion < 3:
            return self.parentAMC.readRegister("GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(self.link))
        else:
            print("HwOptoHybrid.getTriggerSource() - No support for v3 electronics, exiting")
            sys.exit(os.EX_USAGE)
    
    def getVFATMask(self):
        """
        V3 electronics only

        Returns a 24 bit number that should be used as the VFAT Mask
        """

        return self.parentAMC.getLinkVFATMask(self.link)

    def performCalibrationScan(self, **kwargs):
        """
        Performs either a v2b ultra scan or a v3 generic scan of either a single
        channel or all channels depending on input arguments.  Returns the RPC
        response code.

        List of supported arguments is:

        calSF       - V3 electronics only.  The value of the
                      CFG_CAL_FS register to be used when operating in
                      current pulse mode.
        chan        - VFAT channel to be scanned. If not supplied
                      Or a number less than 0 the scan will be performed
                      for all channels, one after another
        currentPulse- V3 electronics only. The calibration module uses
                      the current pulse mode rather than the voltage
                      pulse mode.
        dacMin      - Starting dac value of the scan
        dacMax      - Ending dac value of the scan
        enableCal   - V3 electronics only. Enable cal pulse
        mask        - VFAT mask to use
        nevts       - Number of events for each dac value in scan
        outData     - Array of type c_uint32, if chan >= 0 array size
                      must be: ((dacMax - dacMin + 1) / stepSize) * 24.
                      The first ((dacMax - dacMin + 1) / stepSize)
                      array positions are for VFAT0, the next
                      ((dacMax - dacMin + 1) / stepSize) are for VFAT1,
                      etc...  If a VFAT is masked entries in the array
                      are still allocated but assigned a 0 value.
                      However if chan < 0 the array size must be:
                      (self.nVFATs * 128 * (dacMax - dacMin + 1) / stepSize)
                      Where the array includes data from all channels in
                      sequence.
        scanReg     - Name of register to be scanned.  For v3
                      electronics consult the address table.
                      For v2b electronics see self.KnownV2bElScanRegs
        stepSize    - Step size for moving from dacMin to dacMax
        useUltra    - V2b electronics only, perform an ultra scan
                      instead of a FW scan.  Note if false the VFAT
                      to be scanned is taken as the first non-masked
                      VFAT defined in mask
        useExtTrig  - Scan is performed using L1A's from backplane
        """

        # Set Defaults
        chan=-1
        calSF=0x0
        currentPulse=True
        dacMin=0
        dacMax=254
        enableCal=True
        mask=0x0
        nevts=1000
        stepSize=1
        useUltra=True
        useExtTrig=False

        # Check if outData is present
        if("outData" not in kwargs):
            print("HwOptoHybrid::performCalibrationScan(): You must supply an outData Pointer")
            exit(os.EX_USAGE)

        # Check for scanReg
        scanReg=""
        if("scanReg" not in kwargs):
            print("HwOptoHybrid::performCalibrationScan(): You must supply the name of the scan register scanReg")
            exit(os.EX_USAGE)
        else:
            scanReg = kwargs["scanReg"]

        # Check scan reg validity
        if self.parentAMC.fwVersion < 3:
            if scanReg not in self.KnownV2bElScanRegs:
                print("Parent AMC Major FW Version: %i"%(self.parentAMC.fwVersion))
                print("V2b Electronics Behavior Detected")
                print("scanReg %s Not Known")
                print("Available scanReg values are:", self.KnownV2bElScanRegs)
                sys.exit(os.EX_USAGE)
        else:
            if "CFG_" in scanReg:
                scanReg = str.replace(scanReg,"CFG_","")

        #Get Parameters
        if "chan" in kwargs:
            chan = kwargs["chan"]
        if "calSF" in kwargs:
            calSF = kwargs["calSF"]
        if "currentPulse" in kwargs:
            currentPulse = kwargs["currentPulse"]
        if "dacMin" in kwargs:
            dacMin = kwargs["dacMin"]
        if "dacMax" in kwargs:
            dacMax = kwargs["dacMax"]
        if "enableCal" in kwargs:
            enableCal = kwargs["enableCal"]
        if "mask" in kwargs:
            mask = kwargs["mask"]
        if "nevts" in kwargs:
            nevts = kwargs["nevts"]
        if "stepSize" in kwargs:
            stepSize = kwargs["stepSize"]
        if "useUltra" in kwargs:
            useUltra = kwargs["useUltra"]
        if "useExtTrig" in kwargs:
            useExtTrig = kwargs["useExtTrig"]

        if chan < 0:
            return self.genChannelScan(nevts, self.link, mask, dacMin, dacMax, stepSize, enableCal, currentPulse, calSF, useExtTrig, scanReg, useUltra, kwargs["outData"])
        else:
            return self.genScan(nevts, self.link, dacMin, dacMax, stepSize, chan, enableCal, currentPulse, calSF, mask, scanReg, useUltra, useExtTrig, kwargs["outData"])

    def performDacScan(self, outData, dacSelect, dacStep=1, mask=0x0, useExtRefADC=False):
        """
        Scans the DAC defined by dacSelect, see VFAT3 Manual

        V3 Electronics only

        outData - Array of type c_uint32,
        dacSelect - Integer which specifies the DAC to scan against the ADC.  See VFAT3 Manual
        dacStep - Step size to scan the DAC with
        mask - VFAT mask
        useExtRefADC - If true the DAC scan will be made using the externally referenced ADC on the VFAT3s
        """

        # Check we are v3 electronics
        if self.parentAMC.fwVersion < 3:
            print("HwOptoHybrid::performDacScan(): No support for v2b electronics")
            exit(os.EX_USAGE)

        # Check if dacSelect is valid
        if dacSelect not in maxVfat3DACSize.keys():
            print("HwOptoHybrid::performDacScan(): Invalid dacSelect {0} value.  Valid values are:".format(dacScan))
            print(maxVfat3DACSize.keys())
            exit(os.EX_USAGE)

        #Check length of results container
        lenExpected = (maxVfat3DACSize[dacSelect][0] - 0+1)*24 / dacStep
        if (len(outData) != lenExpected):
            print("HwOptoHybrid::performDacScan(): I expected container of lenght {0} but provided 'outData' has length {1}",format(lenExpected, len(outData)))
            exit(os.EX_USAGE)

        return self.dacScan(self.link, dacSelect, dacStep, mask, useExtRefADC, outData)

    def setDebug(self, debug):
        self.debug = debug
        return

    def setSBitMask(self, sbitMask):
        """
        v3 electronics only

        Sets the sbit mask in the OH FPGA
        """
        
        if self.parentAMC.fwVersion < 3:
            print("Parent AMC Major FW Version: %i"%(self.parentAMC.fwVersion))
            print("Only implemented for v3 electronics, exiting")
            sys.exit(os.EX_USAGE)

        return self.parentAMC.writeRegister("GEM_AMC.OH.OH{0}.FPGA.TRIG.CTRL.VFAT_MASK".format(self.link),sbitMask)

    def setTriggerSource(self,source):
        """
        v2b electronics only
        Set the trigger source
        OH:   0 = TTC over GTX
              1 = FIRMWARE
              2 = EXTERNAL
              3 = LOOPBACK
              4 = LOGICAL OR
              5 = TTC over GBT
        """

        if self.parentAMC.fwVersion < 3:
            return self.parentAMC.writeRegister("GEM_AMC.OH.OH%d.CONTROL.TRIGGER.SOURCE"%(self.link),source)
        else:
            print("HwOptoHybrid.setTriggerSource() - No support for v3 electronics, exiting")
            sys.exit(os.EX_USAGE)

    def setTriggerThrottle(self,throttle):
        """
        Set the trigger throttle
        """
       
        if self.parentAMC.fwVersion < 3:
            return self.parentAMC.writeRegister("GEM_AMC.OH.OH%d.CONTROL.TRIGGER.THROTTLE"%(self.link),throttle)
        else:
            print("HwOptoHybrid.setTriggerThrottle() - There is no way to presecale the L1As being sent in v3 electronics, exiting")
            sys.exit(os.EX_USAGE)

    def setVFATMask(self, mask=None):
        if mask is None:
            self.mask = self.parentAMC.getVFATMask(self.link)
        else:
            self.mask = mask
