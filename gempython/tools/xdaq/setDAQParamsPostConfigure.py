#!/bin/env python

from gempython.tools.amc_user_functions_uhal import *
from gempython.utils.gemlogger import printGreen, printRed, printYellow

def setDAQParams(args):
    from gempython.utils.gracefulKiller import GracefulKiller
    killer = GracefulKiller()

    # Get ENV variable
    from gempython.utils.wrappers import envCheck
    envCheck("GEM_ADDRESS_TABLE_PATH")
    import os
    gemAddrPath = os.getenv("GEM_ADDRESS_TABLE_PATH")

    # Determine INPUT_ENABLE_MASK
    dict_ohMask = {}
    import sys
    if os.path.isfile("{}/gem-shelf{:02d}-input-en-mask.txt".format(gemAddrPath,args.shelf)):
        printGreen("Loading existing input enable mask file at: $GEM_ADDRESS_TABLE_PATH/gem-shelf{:02d}-input-en-mask.txt".format(args.shelf))
        # Load masks
        inptEnMaskFile = open("{}/gem-shelf{:02d}-input-en-mask.txt".format(gemAddrPath,args.shelf),"r")
        for row,line in enumerate(inptEnMaskFile):
            # Skip commented lines
            if line[0] == "#":
                continue

            # Parse line
            line = line.strip("\n")
            slotList = line.split(",")

            # Get slot
            try:
                thisSlot = int(slotList[0],10)
            except ValueError as err:
                printRed("Reading $GEM_ADDRESS_TABLE_PATH/gem-shelf{:02d}-input-en-mask.txt: Invalid value given for AMC slot {}.  I was expecting an integer in [1,12] in decimal representation".format(args.shelf,slotList[0]))
                inptEnMaskFile.close()
                sys.exit(os.EX_DATAERR)

            # Get ohMask
            try:
                dict_ohMask[thisSlot] = int(slotList[1],16)
            except ValueError as err:
                printRed("Reading $GEM_ADDRESS_TABLE_PATH/gem-shelf{:02d}-input-en-mask.txt: For AMC slot {} invalid value given for ohMask: {}.  I was expecting an integer [0x0,0xfff] in hexidecimal representation with a leading '0x'".format(args.shelf,thisSlot,slotList[1]))
                inptEnMaskFile.close()
                sys.exit(os.EX_DATAERR)
                pass

            if dict_ohMask[thisSlot] > 0xfff:
                printYellow("OH Mask determined fro slot {} found to be greater than maximum allowed value 0xfff.  Reseting to 0xfff".format(thisSlot))
                dict_ohMask[thisSlot] = 0xfff
                pass
            pass
    else:
        printYellow("No existing input enable mask file found, creating one with default settings at: $GEM_ADDRESS_TABLE_PATH/gem-shelf{:02d}-input-en-mask.txt".format(args.shelf))
        inptEnMaskFile = open("{}/gem-shelf{:02d}-input-en-mask.txt".format(gemAddrPath,args.shelf),"w")
        inptEnMaskFile.write("#slot,ohMask#\n")
        inptEnMaskFile.write("#############\n")
        for slot in range(1,13):
            dict_ohMask[slot]=0xfff
            inptEnMaskFile.write("{},{}\n".format(slot,dict_ohMask[slot]))
            pass
        pass

    # Configure backend electronics
    for slot in range(0,12):
        # Skip unmasked slots
        if (not ((args.slotMask >> slot) & 0x1)):
            continue
        thisSlot = slot+1

        # Make the AMC Object w/uhal
        global amc
        amc = getAMCObject(thisSlot,args.shelf,args.d)

        # Get ohMask for this slot
        from gempython.tools.amc_user_functions_xhal import HwAMC
        cardName = "gem-shelf%02d-amc%02d"%(args.shelf,thisSlot)
        amcRPC = HwAMC(cardName,debug=args.d)
        try:
            ohMask = dict_ohMask[thisSlot]
        except KeyError as err:
            printYellow("AMC Slot {} not found in $GEM_ADDRESS_TABLE_PATH/gem-shelf{:02d}-input-en-mask.txt. Defaulting to dynamically determined ohMask".format(thisSlot,args.shelf))
            ohMask = amcRPC.getOHMask(raiseIfNoOHs=False)

        # Set the input enable mask
        amc.getNode("GEM_AMC.DAQ.CONTROL.INPUT_ENABLE_MASK").write(ohMask)
        amc.dispatch()

        # Ensure that L1A's are enabled to this AMC
        enableL1A(amc)

        # Ensure the TTC Generator is Disabled (not sure if this is in uhal addr table...)
        amcRPC.writeRegister("GEM_AMC.TTC.GENERATOR.ENABLE",0x0,args.d)
        pass

    from time import sleep
    printYellow("I will now wait 30 seconds; you must navigate to the RCMS Page and Press Start")
    sleep(30)
    amc.getNode("GEM_AMC.DAQ.CONTROL.ZERO_SUPPRESSION_EN").write(0x0)
    amc.dispatch()
    printGreen("Zero Suppression has been correctly disabled; happy data taking!!")

    return

if __name__ == '__main__':
    from reg_utils.reg_interface.common.reg_xml_parser import parseInt
    
    import argparse
    parser = argparse.ArgumentParser()

    parser.add_argument("-s","--slotMask", help="Slot mask to apply, a 1 in the n^th bit indicates the n^th slot should be considered", type=parseInt)
    parser.add_argument("--shelf", help="uTCA shelf number", type=int)
    parser.add_argument("-d", help="debug", action='store_true')

    args = parser.parse_args()

    from gempython.utils.gemlogger import *
    gemlogger = getGEMLogger(__name__)
    gemlogger.setLevel(logging.INFO)

    import uhal
    uhal.setLogLevelTo( uhal.LogLevel.FATAL )

    if args.d:
        print("Running checks")
    if args.slotMask < 0x0 or args.slotMask > 0xfff:
        printRed("Invalid slotMask specified 0x{0:x}\nslotMask must be in [0x0,0xfff]".format(args.slotMask))
        exit(os.EX_USAGE)

    if args.d:
        print("Executing setDAQParams()")
    setDAQParams(args)
