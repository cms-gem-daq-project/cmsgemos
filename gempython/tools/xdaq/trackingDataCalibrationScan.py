#!/bin/env python

from gempython.utils.gemlogger import colors, getGEMLogger, printRed

# From https://github.com/cms-gem-daq-project/cmsgemos/issues/230
dict_scanRegsByRunType = {
            0x2:"CFG_LATENCY",
            0x3:"CFG_THR_ARM_DAC",
            0x4:"CFG_CAL_DAC"
        }

def buildRunParamRegV3(runType,scanReg,supportReg,pulseStretch=0x3,scanMode=0x0):
    """
    Builds the 24 bit number to be set to GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS
    using the following rules:

    runType = 0x2: Latency Scan
        [23:21] scanMode - isCurrentPulse (0 = False, 1 = True)
        [20:13] supportReg - CFG_CAL_DAC
        [12:10] pulseStretch - CFG_PULSE_STRETCH
        [9:0]   scanReg - CFG_LATENCY
    
    runType = 0x3: Threshold Scan
        [23:21] scanMode - CFG_SEL_COMP_MODE (0 = CFG; 1 = ARM; 2 = ZCC)
        [20:13] scanReg - CFG_THR_ARM_DAC
        [12:10] pulseStretch - CFG_PULSE_STRETCH
        [9:0]   supportReg - CFG_THR_ZCC_DAC

    runType = 0x4: Scurve
        [23:21] scanMode - isCurrentPulse (0 = False, 1 = True)
        [20:13] scanReg - CFG_CAL_DAC
        [12:10] pulseStretch - CFG_PULSE_STRETCH
        [9:0]   supportReg - CFG_LATENCY
    """

    if runType == 0x2:
        return ( ((scanMode&0x7)<<21) | ((supportReg&0xff)<<13) | ((pulseStretch&0x7)<<10) | (scanReg&0x3ff) )
    elif runType == 0x3:
        return ( ((scanMode&0x7)<<21) | ((scanReg&0xff)<<13) | ((pulseStretch&0x7)<<10) | (supportReg&0x3ff) )
    elif runType == 0x4:
        return ( ((scanMode&0x7)<<21) | ((scanReg&0xff)<<13) | ((pulseStretch&0x7)<<10) | (supportReg&0x3ff) )

def toggleSettings4DAQ(amcMask, chan, dict_ohMasks, dict_amcBoards, dict_vfatBoards, dict_vfatMasks, runParams, runType, enableCal=True, enableRun=True):
    """
    Toggles both DAQ settings and calpulse settings of VFAT3s.
    Triggers should be stopped before calling this method

    amcMask         - 12 bit number specifying which AMC's to use, if the N^th bit is 1 this means use 
                      the (N+1)^th AMC slot
    chan            - VFAT Channel to configure calpulse for
    dict_ohMasks    - dictionary of integers where the keys are AMC slot numbers and the values are
                      a 12 bit number representing the ohMask; a 1 in the N^th bit means use the N^th
                      optohybrid
    dict_amcBoards  - dictionary of uhal._core.HwInterface objects where the key values are AMC Slot Numbers
    dict_vfatBoards - dictionary of HwVFAT objects where the key values are AMC Slot Numbers 
    dict_vfatMasks  - dictionary of ctype arrays; each ctype array has a size of 12 and each array 
                      position stores the vfatmask values to use with the optohybrid at (slot,link).
                      Here each vfatmask is a 24 bit number; if the N^th bit is 1 this means skip 
                      the N^th VFAT.
    runParams       - Run Parameters words to insert
    runType         - Run Type word to insert
    enableCal       - If True (False) enables (disables) the calibration pulse to chan on all unmasked VFATs
    enableRun       - If True (False) places all unmasked VFATs into Run (Sleep) mode
    """

    # Loop over all AMCs
    for amcN in range(12):
        # Skip masked AMC's        
        if( not ((amcMask >> amcN) & 0x1)):
            continue

        # Update the slot number
        thisSlot = amcN+1

        # Update RunParams and RunType words
        dict_amcBoards[thisSlot].getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS").write(runParams)
        dict_amcBoards[thisSlot].getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE").write(runType)
        dict_amcBoards[thisSlot].dispatch()

        # Loop Over OH's on this AMC and CalPulse to this chan and set VFATs to run mode
        for ohN in range(dict_vfatBoards[thisSlot].parentOH.parentAMC.nOHs):
            if ((dict_ohMasks[thisSlot] >> ohN) & 0x0):
                continue

            # Set the link in dict_vfatBoards[thisSlot]
            dict_vfatBoards[thisSlot].parentOH.link = ohN

            # Update the calibration pulse setting for this chan
            dict_vfatBoards[thisSlot].configureCalPulseAllVFATs(chan,toggleOn=enableCal,mask=dict_vfatMasks[thisSlot][ohN])

            # Place these VFATs into Run Mode?
            dict_vfatBoards[thisSlot].setRunModeAll(mask=dict_vfatMasks[thisSlot][ohN], enable=enableRun)

            ## Order Matters as SlowControl commands are not prioritized in run mode
            #if enableRun: # Place VFATs into Run Mode, first perform slow control
            #    # Update the calibration pulse setting for this chan
            #    dict_vfatBoards[thisSlot].configureCalPulseAllVFATs(chan,toggleOn=enableCal,mask=dict_vfatMasks[thisSlot][ohN])
            #    
            #    # Place these VFATs into Run Mode
            #    dict_vfatBoards[thisSlot].setRunModeAll(mask=dict_vfatMasks[thisSlot][ohN])
            #else:         # Place VFATs into Sleep Mode, first do that then try other slow control cmds
            #    # Take these VFATs out of Run Mode
            #    dict_vfatBoards[thisSlot].setRunModeAll(mask=dict_vfatMasks[thisSlot][ohN],enable=False)
            #    
            #    # Update the calibration pulse setting for this chan
            #    dict_vfatBoards[thisSlot].configureCalPulseAllVFATs(chan,toggleOn=enableCal,mask=dict_vfatMasks[thisSlot][ohN])
            #    pass
            pass # End Loop over OH's
        pass # End Loop over AMCs
    return

def trackingDataScan(args, runType):
    """
    Notes

    args needs to have:

        args.amc13SendsCalPulses true/false, if true use BGO's if False use evka85's FW (3.8.4 or higher...?)
        args.amcMask ... ? this should be a 12 bit number but the N^th bit corresponds to amc slot N+1
        args.debug
        ~~args.detType~~
        ~~args.gemType~~
        args.shelf

        # VFAT Specific Parameters (should come from an argument group...?)
        args.latency
        args.pulseStretch (default 0x3)
        args.calPhase (default 0x0)
        args.ZSMode ...? Something about zero suppression?

        # Scan settings
        args.chMax, default 127
        args.chMin, default 0
        args.dacMax, default 255
        args.dacMin, default 0
        args.dacStep, default 1
        args.nevts, default 100

    Additional options:

        runType - integer, see https://github.com/cms-gem-daq-project/cmsgemos/issues/230
                  Maybe we should declare a class for the possibilities here...? 

    """
    from time import sleep

    from gempython.utils.gracefulKiller import GracefulKiller
    killer = GracefulKiller()

    # Check if GEM_ADDRESS_TABLE_PATH
    from gempython.utils.wrappers import envCheck
    envCheck('GEM_ADDRESS_TABLE_PATH')

    import os
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
    dict_amcBoardsUHAL = ndict() # dict_amcBoardsUHAL[slot] = uhal._core.HwInterface(slot,args.shelf)
    dict_vfatBoards = ndict() # dict_vfatBoards[slot] = HwVFAT(args.shelf,slot, dummyLink) for this (args.shelf,slot)
    dict_ohMasks = ndict() # dict_ohMasks[slot] = ohMask for this (args.shelf,slot)
    dict_vfatMasks = ndict() #dict_vfatMasks[slot][link] = vfatmask for this (args.shelf,slot,link)

    # Create an AMC13 class object, stop triggers and reset counters
    import amc13, os, uhal
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
    for amcN in range(12):
        # Skip masked AMC's        
        if( not ((args.amcMask >> amcN) & 0x1)):
            continue
        thisSlot = amcN+1
       
        # Open connection to the AMC - uHAL
        dict_amcBoardsUHAL[thisSlot] = getAMCObject(thisSlot,args.shelf,args.debug)

        # Open connection to the AMC - RPC
        cardName = "gem-shelf%02d-amc%02d"%(args.shelf,thisSlot)
        dict_vfatBoards[thisSlot] = HwVFAT(cardName, -1, args.debug, args.gemType, args.detType) # Assign a dummy link for now

        # Determine OH's present
        dict_ohMasks[thisSlot] = dict_vfatBoards[thisSlot].parentOH.parentAMC.getOHMask()
       
        # Reset TTC Command Counters
        dict_vfatBoards[thisSlot].parentOH.parentAMC.ttcCmdCntReset()

        # Determine which VFATs to use for all OH's on this AMC
        dict_vfatMasks[thisSlot] = dict_vfatBoards[thisSlot].parentOH.parentAMC.getMultiLinkVFATMask(dict_ohMasks[thisSlot])
        for ohN in range(dict_vfatBoards[thisSlot].parentOH.parentAMC.nOHs):
            if ((dict_ohMasks[thisSlot] >> ohN) & 0x0):
                continue

            # Set the link in dict_vfatBoards[thisSlot]
            dict_vfatBoards[thisSlot].parentOH.link = ohN

            # Ensure All Cal Pulses are OFF
            dict_vfatBoards[thisSlot].stopCalPulses(mask=dict_vfatMasks[thisSlot][ohN])

            # Set initial register parameters
            dict_vfatBoards[thisSlot].setVFATLatencyAll(mask=dict_vfatMasks[thisSlot][ohN], lat=args.latency)
            dict_vfatBoards[thisSlot].setVFATMSPLAll(mask=dict_vfatMasks[thisSlot][ohN], mspl=args.pulseStretch)
            dict_vfatBoards[thisSlot].setVFATCalPhaseAll(mask=dict_vfatMasks[thisSlot][ohN], phase =args.calPhase)
            if runType == 0x2: # Only for Latency scan
                dict_vfatBoards[thisSlot].setVFATCalHeightAll(mask=dict_vfatMasks[thisSlot][ohN], currentPulse=False, height=250)
                pass # End runType == 0x2 (latency scan)
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
            for amcN in range(12):
                # Skip masked AMC's        
                if( not ((args.amcMask >> amcN) & 0x1)):
                    continue
                
                # Set the slot number as amcN+1
                thisSlot = amcN+1

                dict_vfatBoards[thisSlot].parentOH.parentAMC.configureCalMode(enable=True)
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
    msg = "%11s:: fedID n_L1A slot ohN %s"%('','   '.join(map(str, vfatList)))
    print(msg)
    for dacVal in range(args.dacMin,args.dacMax+args.dacStep,args.dacStep):
        # stop triggers coming from AMC13
        amc13board.stopContinuousL1A()

        # Get the FED ID
        sourceID = amc13board.read(amc13board.Board(1),"CONF.ID.SOURCE_ID") # readT1 in AMC13Tool

        # Get Last L1A ID
        lastL1A = amc13board.read(amc13board.Board(0),"STATUS.TTC.L1A_COUNTER") # readT2 in AMC13Tool

        # Loop over all AMCs and mark data as bad with RUN_TYPE and RUN_PARAMS words
        for amcN in range(12):
            # Skip masked AMC's 
            if( not ((args.amcMask >> amcN) & 0x1)):
                continue

            # Set the slot number as amcN+1
            thisSlot = amcN+1

            # Mark data as invalid
            dict_amcBoardsUHAL[thisSlot].getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS").write(0)
            dict_amcBoardsUHAL[thisSlot].getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE").write(0xf)
            dict_amcBoardsUHAL[thisSlot].dispatch()

            # Loop Over OH's on this AMC and set the scanReg to the reg of interest
            for ohN in range(dict_vfatBoards[thisSlot].parentOH.parentAMC.nOHs):
                if (not ((dict_ohMasks[thisSlot] >> ohN) & 0x1)):
                    continue

                # Set the link in dict_vfatBoards[thisSlot]
                dict_vfatBoards[thisSlot].parentOH.link = ohN

                # Write the DAC Value
                dict_vfatBoards[thisSlot].writeAllVFATs(scanReg,dacVal,dict_vfatMasks[thisSlot][ohN])
                
                # Print current status to user
                readBackVals = dict_vfatBoards[thisSlot].readAllVFATs(scanReg,dict_vfatMasks[thisSlot][ohN])
                badreg = map(lambda isbad: 0 if isbad == dacVal else 1, readBackVals)
                perreg = "%s0x%02x%s"
                regmap = map(lambda currentVal: perreg%((colors.GREEN,currentVal&0xffff,colors.ENDC) if currentVal&0xffff==dacVal else (colors.RED,currentVal&0xffff,colors.ENDC) ), readBackVals)
                msg = "%11s:: 0x%x 0x%x %2d %2d  %s"%("CurrentStep",sourceID,lastL1A,thisSlot,ohN,'   '.join(map(str, regmap)))
                print(msg)

                pass # End Loop over all OH's on this slot
            pass # End Loop over all AMC's on this shelf?

        # Enable triggers from AMC13
        amc13board.enableLocalL1A(True)

        if runType == 0x3:  # Threshold Scan
            # placeholder
            raise RuntimeError("trackingDataScan(): Mode Not Implemented Yet")
        elif ((runType == 0x2) or (runType == 0x4)): #Latency Scan or Scurve
            for chan in range(args.chMin,args.chMax+1):
                # Set this channel to pulse for all VFATs and place all VFATs into run mode
                # FIXME RunParam value is not set correctly
                toggleSettings4DAQ(args.amcMask, chan, dict_ohMasks, dict_amcBoardsUHAL, dict_vfatBoards, dict_vfatMasks, dacVal, runType, enableCal=True, enableRun=True)

                # Send Triggers
                amc13board.sendL1ABurst()
                # FIXME something not right about this sleep...? makes this take way longer than I thought
                sleep(args.nevts/rate) # Sleep for the time it takes for the L1A's to be sent
                #sleep(1.05 * (args.nevts/rate)) # Sleep for the time it takes for the L1A's to be sent plus 5% of that time

                # Stop calpulse to this channel for all VFATs and take VFATs out of run mode
                toggleSettings4DAQ(args.amcMask, chan, dict_ohMasks, dict_amcBoardsUHAL, dict_vfatBoards, dict_vfatMasks, 0x0, 0xf, enableCal=False, enableRun=False)
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
            amc13board.disableBGO(0)
            pass
    else:
        for amcN in range(12):
            # Skip masked AMC's        
            if( not ((args.amcMask >> amcN) & 0x1)):
                continue
            
            # Open connection to the AMC
            thisSlot = amcN+1

            dict_vfatBoards[thisSlot].parentOH.parentAMC.configureCalMode(enable=False)
            pass
        pass

    return

if __name__ == '__main__':
    from reg_utils.reg_interface.common.reg_xml_parser import parseInt
    
    import argparse
    parser = argparse.ArgumentParser()

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
    parser.add_argument("--gemType",type=str,help="String that defines the GEM variant, available from the list: {0}".format(gemVariants.keys()),default="ge11")
    parser.add_argument("--shelf", help="uTCA shelf number", type=int, default=1)
    # FIXME complete the parser

    vfatParamsGroup = parser.add_argument_group(title="VFAT3 Parameters for Scan", description="Settings for the VFATs that need to be applied at runtime")
    vfatParamsGroup.add_argument("-l","--latency", help="Value to write to CFG_LATENCY during an scurve", type=int, default=500) 
    vfatParamsGroup.add_argument("-p","--pulseStretch", help="Value to write to CFG_PULSE_STRETCH", type=int, default=0x3)
    vfatParamsGroup.add_argument("-c","--calPhase", help="Value to write to CFG_CAL_PHI", type=int, default=0x0)

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

    trackingDataScan(args, runType)

    print("Scan finished. Goodbye")
