#!/bin/env python

from gempython.utils.gemlogger import colors, printRed

# FIXME place in a HwAMC class or HwVFAT?
# FIXME place in hw_constants.py?
# From https://github.com/cms-gem-daq-project/cmsgemos/issues/230
dict_scanRegsByRunType = {
            0x2:"CFG_LATENCY",
            0x3:"CFG_THR_ARM_DAC",
            0x4:"CFG_CAL_DAC"
        }

def toggleSettings4DAQ(amcMask, chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, runParams, runType, enableCal=True, enableRun=True):
    """
    Toggles both DAQ settings and calpulse settings of VFAT3s.
    Triggers should be stopped before calling this method

    amcMask         - 12 bit number specifying which AMC's to use, if the N^th bit is 1 this means use 
                      the (N+1)^th AMC slot
    chan            - VFAT Channel to configure calpulse for
    dict_ohMasks    - dictionary of integers where the keys are AMC slot numbers and the values are
                      a 12 bit number representing the ohMask; a 1 in the N^th bit means use the N^th
                      optohybrid
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
        dict_vfatBoards[thisSlot].parentOH.parentAMC.setRunParams(runParams)
        dict_vfatBoards[thisSlot].parentOH.parentAMC.setRunType(runType)

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
    dict_vfatBoards = ndict() # dict_vfatBoards[slot] = HwVFAT(args.shelf,slot, dummyLink) for this (args.shelf,slot)
    dict_ohMasks = ndict() # dict_ohMasks[slot] = ohMask for this (args.shelf,slot)
    dict_vfatMasks = ndict() #dict_vfatMasks[slot][link] = vfatmask for this (args.shelf,slot,link)

    # Create an AMC13 class object and stop triggers
    import amc13, os
    connection_file = "%s/connections.xml"%(os.getenv("GEM_ADDRESS_TABLE_PATH"))
    amc13base  = "gem.shelf%02d.amc13"%(args.shelf)
    amc13board = amc13.AMC13(connection_file,"%s.T1"%(amc13base),"%s.T2"%(amc13base))
    amc13board.enableLocalL1A(False)
    amc13board.stopContinuousL1A()

    # Declare all hardware connections and set initial register values
    from gempython.tools.vfat_user_functions_xhal import HwVFAT
    for amcN in range(12):
        # Skip masked AMC's        
        if( not ((args.amcMask >> amcN) & 0x1)):
            continue
        
        # Open connection to the AMC
        thisSlot = amcN+1
        cardName = "gem-shelf%02d-amc%02d"%(args.shelf,thisSlot)
        #dict_vfatBoards[thisSlot] = HwVFAT(cardName, -1, args.debug, args.gemType, args.detType) # Assign a dummy link for now
        dict_vfatBoards[thisSlot] = HwVFAT(cardName, -1, args.debug) # Assign a dummy link for now

        # Determine OH's present
        dict_ohMasks[thisSlot] = dict_vfatBoards[thisSlot].parentOH.parentAMC.getOHMask()
        
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

    # Configure AMC13 Trigger Mode
    if (args.amc13SendsCalPulses):    # Calpulses sent by AMC13 as BGO commands
        if ((runType == 0x2) or (runType == 0x4)):
            # Configure BGO generator to send a CalPulse before the L1A
            cmdCalPulse = 0x14
            amc13board.configureBGOShort(0, cmdCalPulse, 450, 0, True) # Should send CalPulse at BX = 450; but might actually be 470 from debugging
            pass

        # Configure locally generated triggers for one L1A per orbit @ BX=500
        amc13board.configureLocalL1A(True, 0, 1, 1, 0)
    else:                           # Calpulses sent by CTP7 on receipt of L1A from AMC13
        # place holder
        # Here a higher frequency of L1A's can probably be used
        # implement new features of: https://github.com/evka85/GEM_AMC/releases/tag/v3.8.4
        raise RuntimeError("trackingDataScan(): Mode Not Implemented Yet")

    # Perform the scan
    for dacVal in range(args.dacMin,args.dacMax+args.dacStep,args.dacStep):
        # stop triggers coming from AMC13
        amc13board.stopContinuousL1A()

        # Loop over all AMCs and mark data as bad with RUN_TYPE and RUN_PARAMS words
        for amcN in range(12):
            # Skip masked AMC's 
            if( not ((args.amcMask >> amcN) & 0x1)):
                continue

            # Set the slot number as amcN+1
            thisSlot = amcN+1

            # Mark data as invalid
            dict_vfatBoards[thisSlot].parentOH.parentAMC.setRunParams(0x0)
            dict_vfatBoards[thisSlot].parentOH.parentAMC.setRunType(0xf)

            # Loop Over OH's on this AMC and set the scanReg to the reg of interest
            for ohN in range(dict_vfatBoards[thisSlot].parentOH.parentAMC.nOHs):
                if ((dict_ohMasks[thisSlot] >> ohN) & 0x0):
                    continue

                # Set the link in dict_vfatBoards[thisSlot]
                dict_vfatBoards[thisSlot].parentOH.link = ohN

                # Write the DAC Value
                dict_vfatBoards[thisSlot].writeAllVFATs(scanReg,dacVal,dict_vfatMasks[thisSlot][ohN])
                pass # End Loop over all OH's on this slot
            pass # End Loop over all AMC's on this shelf?

        # Enable triggers from AMC13
        amc13board.enableLocalL1A(True)

        if runType == 0x3:  # Threshold Scan
            # placeholder
            raise RuntimeError("trackingDataScan(): Mode Not Implemented Yet")
        elif ((runType == 0x2) or (runType == 0x4)): #Latency Scan or Scurve
            # placeholder
            for chan in range(args.chMin,args.chMax+1):
                #FIXME might need amc13board.enableLocalL1A(True) here?

                # Set this channel to pulse for all VFATs and place all VFATs into run mode
                toggleSettings4DAQ(args.amcMask, chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, dacVal, runType, enableCal=True, enableRun=True)

                # Send Triggers
                for evt in range(args.nevts):
                    amc13board.sendL1ABurst()
                    pass # End Event loop
                #FIXME might need amc13board.enableLocalL1A(False) here?

                # Stop calpulse to this channel for all VFATs and take VFATs out of run mode
                toggleSettings4DAQ(args.amcMask, chan, dict_ohMasks, dict_vfatBoards, dict_vfatMasks, 0x0, 0xf, enableCal=False, enableRun=False)
                pass # End Loop over VFAT Channels
            pass # End if-elif statement
        pass # End Loop over DAC Range

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
    parser.add_argument("-a","--amc13SendsCalPulses",help="If provided AMC13 will generate calpulse commands.  Otherwise the CTP7's will delay the original L1A received and generate a calpulse", action="store_true")
    parser.add_argument("-d","--debug", help="prints additional debugging information", action="store_true")
    #parser.add_argument("--gemType",type=str,help="String that defines the GEM variant, available from the list: {0}".format(gemVariants.keys()),default="ge11")
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
        print("Executing Scan:")

    trackingDataScan(args, runType)

    print("Scan finished. Goodbye")
