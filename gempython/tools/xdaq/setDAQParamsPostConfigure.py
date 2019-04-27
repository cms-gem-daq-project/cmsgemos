#!/bin/env python

from gempython.tools.amc_user_functions_uhal import *
from gempython.utils.gemlogger import printGreen, printRed, printYellow

def setDAQParams(args):
    from gempython.utils.gracefulKiller import GracefulKiller
    killer = GracefulKiller()

    for slot in range(1,13):
        # Skip unmasked slots
        if (not ((args.slotMask >> slot) & 0x1)):
            continue

        # Make the AMC Object w/uhal
        global amc
        amc = getAMCObject(args.slot,args.shelf,args.d)

        # Get ohMask for this slot
        from gempython.tools.amc_user_functions_xhal import HwAMC
        cardName = "gem-shelf%02d-amc%02d"%(args.shelf,args.slot)
        amcRPC = HwAMC(cardName,debug=args.d)
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
