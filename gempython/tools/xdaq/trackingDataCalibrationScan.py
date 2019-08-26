#!/bin/env python

from gempython.utils.gemlogger import colors, getGEMLogger, printRed

# From https://github.com/cms-gem-daq-project/cmsgemos/issues/230
dict_scanRegsByRunType = {
            0x2:"CFG_LATENCY",
            0x3:"CFG_THR_ARM_DAC",
            0x4:"CFG_CAL_DAC"
        }

def toggleRunMode(dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableRun=True):
    """
    Function documentation goes here
    """
    
    # Loop over all AMCs
    for amcSlot, vfatBoard in dict_vfatBoards.iteritems():
        for ohN in range(vfatBoard.parentOH.parentAMC.nOHs):
            if ((dict_ohMasks[amcSlot] >> ohN) & 0x0):
                continue
            
            # Set the link in vfatBoard
            vfatBoard.parentOH.link = ohN
            
            # Place these VFATs into Run Mode?
            vfatBoard.setRunModeAll(mask=dict_vfatMasks[amcSlot][ohN], enable=enableRun)
            pass # End Loop over OH's
        pass # End Loop over AMCs
    return

def toggleCalPulseAndRunMode(chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableCal=True, enableCalDF=True, enableRun=True):
    """
    Toggles calpulse on all VFATs for channel of interest; will also toggle run mode on all VFATs
    Triggers should be stopped before calling this method

    chan            - VFAT Channel to configure calpulse for
    dict_ohMasks    - dictionary of integers where the keys are AMC slot numbers and the values are
                      a 12 bit number representing the ohMask; a 1 in the N^th bit means use the N^th
                      optohybrid
    dict_vfatBoards - dictionary of HwVFAT objects where the key values are AMC Slot Numbers 
    dict_vfatMasks  - dictionary of ctype arrays; each ctype array has a size of 12 and each array 
                      position stores the vfatmask values to use with the optohybrid at (slot,link).
                      Here each vfatmask is a 24 bit number; if the N^th bit is 1 this means skip 
                      the N^th VFAT.
    enableCal       - If True (False) enables (disables) the calibration pulse to chan on all unmasked VFATs
    enableCalDF     - If True (False) enables (disables) the calibration data format during the event building
    enableRun       - If True (False) places all unmasked VFATs into Run (Sleep) mode
    """

    # Loop over all AMCs
    for amcSlot,vfatBoard in dict_vfatBoards.iteritems():
        # Set the calibration channel
        if(enableCal):
            try:
                vfatBoard.parentOH.parentAMC.configureCalDataFormat(chan, enableCalDF)
            except RuntimeError as err:
                printYellow(err.message)
                pass

        # Loop Over OH's on this AMC and CalPulse to this chan and set VFATs to run mode
        for ohN in range(vfatBoard.parentOH.parentAMC.nOHs):
            if ((dict_ohMasks[amcSlot] >> ohN) & 0x0):
                continue

            # Set the link in vfatBoard
            vfatBoard.parentOH.link = ohN

            # Update the calibration pulse setting for this chan
            vfatBoard.configureCalPulseAllVFATs(chan,toggleOn=enableCal,mask=dict_vfatMasks[amcSlot][ohN])

            # Place these VFATs into Run Mode?
            vfatBoard.setRunModeAll(mask=dict_vfatMasks[amcSlot][ohN], enable=enableRun)

            ## Order Matters as SlowControl commands are not prioritized in run mode
            #if enableRun: # Place VFATs into Run Mode, first perform slow control
            #    # Update the calibration pulse setting for this chan
            #    vfatBoard.configureCalPulseAllVFATs(chan,toggleOn=enableCal,mask=dict_vfatMasks[amcSlot][ohN])
            #    
            #    # Place these VFATs into Run Mode
            #    vfatBoard.setRunModeAll(mask=dict_vfatMasks[amcSlot][ohN])
            #else:         # Place VFATs into Sleep Mode, first do that then try other slow control cmds
            #    # Take these VFATs out of Run Mode
            #    vfatBoard.setRunModeAll(mask=dict_vfatMasks[amcSlot][ohN],enable=False)
            #    
            #    # Update the calibration pulse setting for this chan
            #    vfatBoard.configureCalPulseAllVFATs(chan,toggleOn=enableCal,mask=dict_vfatMasks[amcSlot][ohN])
            #    pass
            pass # End Loop over OH's
        pass # End Loop over AMCs
    return

def trackingDataScan(args, runType):
    """
    Performs a calibration scan with the intent of using the full data path.  The type of scan
    is based on runType and for each runType the following VFAT3 register will be scanned:

        runType - 0x2:"CFG_LATENCY",
        runType - 0x3:"CFG_THR_ARM_DAC",
        runType - 0x4:"CFG_CAL_DAC"

    For scurve scans the channel register configuration will be retrieved and stored; then the
    channel mask bit (bit 14) will be set to 1 for all channels on all links before the scan
    is started.  After the scan the original channel reigster configuration will be restored

    The args namespace is expected to have the following parameters:    

        amc13SendsCalPulses True (False) AMC13 BGO generator (CTP7 calibration mode) sends calibration pulse commands
        amcMask             This should be a 12 bit number where the N^th bit corresponds to amc slot N+1
        debug               If true prints additional debugging info
        detType             String that defines the detector type within GEM variants
        gemType             String that defines the GEM variant being used
        shelf               uTCA shelf number to be used

        # VFAT Specific Parameters (should come from an argument group...?)
        latency             CFG_LATENCY value that will be applied for scurves
        pulseStretch        CFG_PULSE_STRETCH value that will be applied at the start of all scans
        calPhase            CFG_CAL_PHI value that will be applied at the start of all scans

        # Scan settings
        chMax               Maximum channel to be scanned
        chMin               Minimum channel to be scanned
        dacMax              Maximum DAC value to be used in the scan
        dacMin              Minimum DAC value to be used in the scan
        dacStep             Step size to use in the scan
        nevts               Number of events taken at every scan point
    """
    from ctypes import c_uint32

    from gempython.utils.gracefulKiller import GracefulKiller
    killer = GracefulKiller()

    # Check if GEM_ADDRESS_TABLE_PATH
    from gempython.utils.wrappers import envCheck
    envCheck('GEM_ADDRESS_TABLE_PATH')

    import os
    import sys
    try:
        scanReg = dict_scanRegsByRunType[runType]
    except KeyError as err:
        msg="trackingDataScan() - KeyError: RunType = {0} Not understood, possible values are:\n".format(runType)
        for key,regName in dict_scanRegsByRunType.iteritems():
            msg = "{0}\n{1}\t{2}".format(msg,key,regName)
            pass
        printRed(msg)
        return os.EX_USAGE

    # Declare the following dictionaries
    from gempython.utils.nesteddict import nesteddict as ndict
    dict_vfatBoards = ndict() # dict_vfatBoards[slot] = HwVFAT(args.shelf,slot, dummyLink) for this (args.shelf,slot)
    dict_ohMasks = ndict() # dict_ohMasks[slot] = ohMask for this (args.shelf,slot)
    dict_vfatMasks = ndict() #dict_vfatMasks[slot][link] = vfatmask for this (args.shelf,slot,link)
    dict_origChanRegs = ndict() #dict_origChanRegs[slot][link] = c_uint32 array 3072 in length fo this (args.shelf,slot,link)

    # Create an AMC13 class object, stop triggers and reset counters
    import amc13
    import os
    import uhal
    uhal.setLogLevelTo( uhal.LogLevel.FATAL )
    connection_file = "%s/connections.xml"%(os.getenv("GEM_ADDRESS_TABLE_PATH"))
    amc13base  = "gem.shelf%02d.amc13"%(args.shelf)
    amc13board = amc13.AMC13(connection_file,"%s.T1"%(amc13base),"%s.T2"%(amc13base))
    amc13board.enableLocalL1A(False)
    amc13board.stopContinuousL1A()
    amc13board.resetCounters()

    # Declare all hardware connections, reset TTC/DAQ counters and set initial register values
    from gempython.tools.vfat_user_functions_xhal import HwVFAT
    from gempython.tools.amc_user_functions_uhal import getAMCObject
    import logging
    gemlogger = getGEMLogger(__name__)
    gemlogger.setLevel(logging.ERROR)
    isCurrent = False
    selCompMode = None
    for amcN in range(12):
        # Skip masked AMC's        
        if( not ((args.amcMask >> amcN) & 0x1)):
            continue
        thisSlot = amcN+1
       
        # Open connection to the AMC - RPC
        cardName = "gem-shelf%02d-amc%02d"%(args.shelf,thisSlot)
        dict_vfatBoards[thisSlot] = HwVFAT(cardName, -1, args.debug, args.gemType, args.detType) # Assign a dummy link for now

        # Set the RUN TYPE for this AMC
        dict_vfatBoards[thisSlot].parentOH.parentAMC.writeRegister("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE",runType)

        # Determine OH's present
        dict_ohMasks[thisSlot] = dict_vfatBoards[thisSlot].parentOH.parentAMC.getOHMask()
        if args.debug:
            print("ohMask for AMC{:d} determined to be 0x{:x}".format(thisSlot,dict_ohMasks[thisSlot]))
       
        # Reset TTC Command Counters
        dict_vfatBoards[thisSlot].parentOH.parentAMC.ttcCmdCntReset()

        # Determine which VFATs to use for all OH's on this AMC
        dict_vfatMasks[thisSlot] = dict_vfatBoards[thisSlot].parentOH.parentAMC.getMultiLinkVFATMask(dict_ohMasks[thisSlot])
        for ohN in range(dict_vfatBoards[thisSlot].parentOH.parentAMC.nOHs):
            if (not ((dict_ohMasks[thisSlot] >> ohN) & 0x1)):
                continue

            # Set the link in dict_vfatBoards[thisSlot]
            dict_vfatBoards[thisSlot].parentOH.link = ohN

            # Ensure All Cal Pulses are OFF
            dict_vfatBoards[thisSlot].stopCalPulses(mask=dict_vfatMasks[thisSlot][ohN])

            # Set initial register parameters
            dict_vfatBoards[thisSlot].setVFATLatencyAll(mask=dict_vfatMasks[thisSlot][ohN], lat=args.latency)
            dict_vfatBoards[thisSlot].setVFATMSPLAll(mask=dict_vfatMasks[thisSlot][ohN], mspl=args.pulseStretch)
            dict_vfatBoards[thisSlot].setVFATCalPhaseAll(mask=dict_vfatMasks[thisSlot][ohN], phase=args.calPhase)
            if runType == 0x2: # Only for Latency scan
                # Set calpulse height
                dict_vfatBoards[thisSlot].setVFATCalHeightAll(mask=dict_vfatMasks[thisSlot][ohN], currentPulse=False, height=250)
                pass # End runType == 0x2 (latency scan)
            elif runType == 0x3: # Only for Threshold scan
                # Set comparator mode
                if args.compModeARM:
                    selCompMode = 0x1
                    dict_vfatBoards[thisSlot].writeAllVFATs("CFG_SEL_COMP_MODE",selCompMode,dict_vfatMasks[thisSlot][ohN])
                elif args.compModeZCC:
                    selCompMode = 0x2
                    dict_vfatBoards[thisSlot].writeAllVFATs("CFG_SEL_COMP_MODE",selCompMode,dict_vfatMasks[thisSlot][ohN])
                else:
                    print("Setting CFG_SEL_COMP_MODE of VFATs on (shelf,slot,link)=({:d},{:d},{:d}) to 0x0 (CFD Mode)".format(
                        args.shelf,
                        dict_vfatBoards[thisSlot].parentOH.parentAMC.slot,
                        ohN) )
                    selCompMode = 0x0
                    dict_vfatBoards[thisSlot].writeAllVFATs("CFG_SEL_COMP_MODE",selCompMode,dict_vfatMasks[thisSlot][ohN])
                    pass

                # Ensure calpulse is disabled on all VFATs
                dict_vfatBoards[thisSlot].configureCalPulseAllVFATs(ch=0,toggleOn=False,mask=dict_vfatMasks[thisSlot][ohN])
            elif runType == 0x4: # Only for Scurve
                # Get original channel mask
                dict_origChanRegs[thisSlot][ohN] = dict_vfatBoards[thisSlot].getAllChannelRegisters(mask=dict_vfatMasks[thisSlot][ohN])

                # Set the mask bit of all channel registers to 0x1
                print("Masking all channels on (shelf,slot,link)=({:d},{:d},{:d})".format(
                    args.shelf,
                    dict_vfatBoards[thisSlot].parentOH.parentAMC.slot,
                    ohN) )
                tmpChanArray = (c_uint32 * 3072)()
                for chan in range(3072):
                    tmpChanArray[chan] = (0x1 << 14) | dict_origChanRegs[thisSlot][ohN][chan]
                    pass

                # Update the channel registers
                dict_vfatBoards[thisSlot].setAllChannelRegisters(chMask=tmpChanArray,vfatMask=dict_vfatMasks[thisSlot][ohN])
                pass
            pass # End loop over OH's on this AMC
        pass # End loop over AMC's

    # Configure AMC13 Trigger Mode and Calpulses (runType == 0x2 or 0x4 only)
    if (args.amc13SendsCalPulses):    # Calpulses sent by AMC13 as BGO commands
        if ((runType == 0x2) or (runType == 0x4)):
            # Configure BGO generator to send a CalPulse before the L1A
            cmdCalPulse = 0x14
            amc13board.configureBGOShort(0, cmdCalPulse, 450, 0, True) # Should send CalPulse at BX = 450; but might actually be 470 from debugging
            pass

        # Configure locally generated triggers for one L1A per orbit @ BX=500
        amc13board.configureLocalL1A(True, 0, args.nevts, 1, 0)
        rate = int((1.0 / 0.0000909091)) # (1 / orbit period)
    else:                           # Calpulses sent by CTP7 on receipt of L1A from AMC13
        # Configure AMC's when receiving an L1A to delay the L1A and send a calpulse before the L1A
        # Only valid for GEM_AMC FW >= 3.8.4
        # implement new features of: https://github.com/evka85/GEM_AMC/releases/tag/v3.8.4
        if ((runType == 0x2) or (runType == 0x4)):
            for vfatBoard in dict_vfatBoards.values():
                try:
                    vfatBoard.parentOH.parentAMC.configureCalMode(enable=True)
                except RuntimeError as err:
                    printRed(err.message)
                    printRed("Please re-launch with option --amc13SendsCalPulses")
                    sys.exit(os.EX_CONFIG)
                pass
            pass

        # Disable all BGO's
        for chan in range(4):
            amc13board.disableBGO(chan)

        # Configure locally generated triggers for 4 L1A's per orbit
        rate = int((1.0 / 0.0000909091) * 4) # (1 / orbit period) * N_L1A's/orbit
        amc13board.configureLocalL1A(True, 1, args.nevts, rate, 0)
        pass

    # Perform the scan
    from gempython.tools.hw_constants import vfatsPerGemVariant
    vfatList = [x for x in range(vfatsPerGemVariant[args.gemType])]
    if args.debug:
        print("SCAN_REG: {:s}".format(scanReg))
        print("DAC MIN:  {:d}".format(args.dacMin))
        print("DAC MAX:  {:d}".format(args.dacMax))
        print("DAC STEP: {:d}".format(args.dacStep))
        pass

    msg = "%11s:: amc13_nL1A amc_nL1A amc_nCalPulse slot ohN %s"%('','   '.join(map(str, vfatList)))
    print(msg)
    for dacVal in range(args.dacMin,args.dacMax+args.dacStep,args.dacStep):
        # stop triggers coming from AMC13
        amc13board.stopContinuousL1A()

        # Get the FED ID
        sourceID = amc13board.read(amc13board.Board(1),"CONF.ID.SOURCE_ID") # readT1 in AMC13Tool

        # Get Last L1A ID
        lastL1A = amc13board.read(amc13board.Board(0),"STATUS.TTC.L1A_COUNTER") # readT2 in AMC13Tool

        # Loop over all AMCs and set the dacVal and update the RUN_PARAMS words
        for amcSlot,vfatBoard in dict_vfatBoards.iteritems():
            # Update RunParams word
            if runType == 0x2:
                runParams = ( (0x1 << 22) | (0x0 << 21) | (0x6 << 13) | ((args.pulseStretch & 0x7) << 10) | (dacVal) )
                pass
            elif runType == 0x3:
                runParams = ( ((selCompMode & 0x3) << 21 | ((dacVal & 0xff) << 13) | ((args.pulseStretch & 0x7) << 10) | 0xA) )
                pass
            elif runType == 0x4:
                runParams = ( (0x0 << 21) | ((dacVal & 0xff) << 13) | ((args.pulseStretch & 0x7) << 10) | 0x64)
                pass
            vfatBoard.parentOH.parentAMC.writeRegister("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS",runParams)

            # Get TTC Command Counters
            lastL1A_AMC = vfatBoard.parentOH.parentAMC.readRegister("GEM_AMC.TTC.CMD_COUNTERS.L1A")
            lastCalPulse_AMC = vfatBoard.parentOH.parentAMC.readRegister("GEM_AMC.TTC.CMD_COUNTERS.CALPULSE")

            # Loop Over OH's on this AMC and set the scanReg to the reg of interest
            for ohN in range(vfatBoard.parentOH.parentAMC.nOHs):
                if (not ((dict_ohMasks[amcSlot] >> ohN) & 0x1)):
                    continue

                # Set the link in vfatBoard
                vfatBoard.parentOH.link = ohN

                # Write the DAC Value
                vfatBoard.writeAllVFATs(scanReg,dacVal,dict_vfatMasks[amcSlot][ohN])
                
                # Print current status to user
                readBackVals = vfatBoard.readAllVFATs(scanReg,dict_vfatMasks[amcSlot][ohN])
                badreg = map(lambda isbad: 0 if isbad == dacVal else 1, readBackVals)
                perreg = "%s0x%02x%s"
                regmap = map(lambda currentVal: perreg%((colors.GREEN,currentVal&0xffff,colors.ENDC) if currentVal&0xffff==dacVal else (colors.RED,currentVal&0xffff,colors.ENDC) ), readBackVals)
                msg = "%11s:: 0x%x 0x%x 0x%x %2d %2d  %s"%("CurrentStep",lastL1A,lastL1A_AMC,lastCalPulse_AMC,amcSlot,ohN,'   '.join(map(str, regmap)))
                print(msg)

                pass # End Loop over all OH's on this slot
            pass # End Loop over all AMC's on this shelf?

        # Enable triggers from AMC13
        amc13board.enableLocalL1A(True)

        if runType == 0x3:  # Threshold Scan
            if args.doNotUseCalDataFormat:
                # Place VFATs into run mode
                toggleRunMode(dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableRun=True)

                # Send Triggers
                amc13board.sendL1ABurst()

                # Take VFATs out of run mode
                toggleRunMode(dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableRun=False)
            else:
                for chan in range(args.chMin,args.chMax+1):
                    # Place VFATs into run mode
                    toggleCalPulseAndRunMode(chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableCal=False, enableCalDF=True, enableRun=True)

                    # Send Triggers
                    amc13board.sendL1ABurst()

                    # Take VFATs out of run mode
                    toggleCalPulseAndRunMode(chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableCal=False, enableCalDF=True, enableRun=False)
        elif ((runType == 0x2) or (runType == 0x4)): #Latency Scan or Scurve
            for chan in range(args.chMin,args.chMax+1):
                # Set this channel to pulse for all VFATs and place all VFATs into run mode
                toggleCalPulseAndRunMode(chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableCal=True, enableCalDF=(not args.doNotUseCalDataFormat), enableRun=True)

                # Send Triggers
                amc13board.sendL1ABurst()

                # Stop calpulse to this channel for all VFATs and take VFATs out of run mode
                toggleCalPulseAndRunMode(chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, enableCal=False, enableCalDF=(not args.doNotUseCalDataFormat), enableRun=False)
                pass # End Loop over VFAT Channels
            pass # End if-elif statement
        pass # End Loop over DAC Range

    # Turn off triggers
    amc13board.stopContinuousL1A()
    amc13board.enableLocalL1A(False)

    # Take AMC's out of calibration mode if they where in calibration mode
    if (args.amc13SendsCalPulses):    # Calpulses sent by AMC13 as BGO commands
        if ((runType == 0x2) or (runType == 0x4)):
            # Disable BGO for channel 0
            amc13board.configureBGOShort(0, 0x0, 450, 0, False)
            amc13board.disableBGO(0)
            pass
    else:
        for vfatBoard in dict_vfatBoards.values():
            vfatBoard.parentOH.parentAMC.configureCalMode(enable=False)
            pass
        pass

    # Turn off calibration data format if it was used
    if not args.doNotUseCalDataFormat:
        for amcSlot,vfatBoard in dict_vfatBoards.iteritems():
            vfatBoard.parentOH.parentAMC.configureCalDataFormat(0,False)
            pass
        pass

    # Restore original channel configuration to frontend
    if runType == 0x4:
        for amcSlot,vfatBoard in dict_vfatBoards.iteritems():
            for ohN in range(vfatBoard.parentOH.parentAMC.nOHs):
                if ( not (dict_ohMasks[amcSlot] >> ohN) & 0x1):
                    continue
                
                # Set the link in vfatBoard
                vfatBoard.parentOH.link = ohN
            
                # Restore original channel number
                vfatBoard.setAllChannelRegisters(chanRegData=dict_origChanRegs[amcSlot][ohN],vfatMask=dict_vfatMasks[amcSlot][ohN])
                pass
            pass
        pass

    return 0

if __name__ == '__main__':
    from reg_utils.reg_interface.common.reg_xml_parser import parseInt
    
    import argparse
    parser = argparse.ArgumentParser()

    import sys

    # Required Parameters
    parser.add_argument("amcMask", help="Slot mask to apply, a 1 in the N^th bit indicates the (N+1)^th slot should be considered", type=parseInt)
    runTypeGroup = parser.add_mutually_exclusive_group(required=True)
    runTypeGroup.add_argument("--latScan", help="Perform a latency scan using the calibration module", action="store_true")
    runTypeGroup.add_argument("--scurve", help="Take an scurve scan", action="store_true")
    runTypeGroup.add_argument("--thrScan", help="Perform a scan of the CFG_THR_ARM_DAC register", action="store_true")

    # Optional Parameters
    from gempython.tools.hw_constants import gemVariants
    parser.add_argument("-a","--amc13SendsCalPulses",help="If provided AMC13 will generate calpulse commands.  Otherwise the CTP7's will delay the original L1A received and generate a calpulse", action="store_true")
    parser.add_argument("-d","--debug", help="prints additional debugging information", action="store_true")
    parser.add_argument("--detType",type=str,help="String that defines the detector type within the GEM variant, available from the list: {0}".format(gemVariants.values()),default="short")
    parser.add_argument("--doNotUseCalDataFormat",action="store_true",help="If provided the full GEM data format will be used instead of the GEM calibration data format")
    parser.add_argument("--gemType",type=str,help="String that defines the GEM variant, available from the list: {0}".format(gemVariants.keys()),default="ge11")
    parser.add_argument("--shelf", help="uTCA shelf number", type=int, default=1)

    vfatParamsGroup = parser.add_argument_group(title="VFAT3 Parameters for Scan", description="Settings for the VFATs that need to be applied at runtime")
    vfatParamsGroup.add_argument("-c","--calPhase", help="Value to write to CFG_CAL_PHI", type=int, default=0x0)
    vfatParamsGroup.add_argument("-l","--latency", help="Value to write to CFG_LATENCY during an scurve", type=int, default=500) 
    vfatParamsGroup.add_argument("-p","--pulseStretch", help="Value to write to CFG_PULSE_STRETCH", type=int, default=0x3)
    vfatCompMode = vfatParamsGroup.add_mutually_exclusive_group(required=False)
    #vfatCompMode.add_argument("--compModeCFD", action="store_true", help="Set the comparator mode on all VFATs to CFD Mode (CFG_SEL_COMP_MODE = 0x0)")
    vfatCompMode.add_argument("--compModeARM", action="store_true", help="Set the comparator mode on all VFATs to ARM Mode (CFG_SEL_COMP_MODE = 0x1)")
    vfatCompMode.add_argument("--compModeZCC", action="store_true", help="Set the comparator mode on all VFATs to ZCC Mode (CFG_SEL_COMP_MODE = 0x2)")

    scanParamsGroup = parser.add_argument_group(title="Scan Parameters", description="Settings that determine the range of channels and DACs for the scan")
    scanParamsGroup.add_argument("--chMax", help="Maximum VFAT Channel to be scanned", type=int, default=127)
    scanParamsGroup.add_argument("--chMin", help="Minimum VFAT Channel to be scanned", type=int, default=0)
    scanParamsGroup.add_argument("--dacMax", help="Maximum DAC value the scan will reach", type=int, default=None)
    scanParamsGroup.add_argument("--dacMin", help="Minimum DAC value the scan will reach", type=int, default=0)
    scanParamsGroup.add_argument("--dacStep", help="Step size to move through the DAC in", type=int, default=1)
    scanParamsGroup.add_argument("-n","--nevts", help="For {0}thrScan{1} this is the number of events that will be acquired per DAC value.  For {0}scurve{1} or {0}latScan{1} this is the number of events that will be acquired per DAC value per channel".format(colors.GREEN,colors.ENDC),type=int,default=100)

    args = parser.parse_args()
    
    # Check validity of input arguments
    if args.debug:
        print("Running checks")
    if args.amcMask < 0x0 or args.amcMask > 0xfff:
        printRed("Invalid amcMask specified 0x{0:x}\namcMask must be in [0x0,0xfff]".format(args.amcMask))
        exit(os.EX_USAGE)

    # Determine run type
    if args.latScan:
        runType = 0x2
        if args.dacMax is None:
            args.dacMax = 1023
    elif args.scurve:
        runType = 0x4
        if args.dacMax is None:
            args.dacMax = 255
    elif args.thrScan:
        runType = 0x3
        if args.dacMax is None:
            args.dacMax = 255
        pass

    if args.debug:
        print("Executing Scan: {:s}".format(dict_scanRegsByRunType[runType]))

    try:
        trackingDataScan(args, runType)
    except KeyboardInterrupt as err:
        printYellow("Keyboard Interrupt; scan is terminating.  Please note your frontend configuration may no longer match expectation and a re-configuration may be required")
        sys.exit(0)

    print("Scan finished. Goodbye")
