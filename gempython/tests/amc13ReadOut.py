#!/bin/env python

def readAllL1ACounters(amc13board, amcs, key="Initial", trigCounts=None):
    """
    Reads the L1A counters from amc13board and all AMCs in amcs.
    Returns the L1A counters in a nesteddict trigCounts

    amc13board - instance of the AMC13manager class
    amcs - dictionary of uhal devices where key is the slot number in the uTCA crate
    key - key to add to or insert in trigCounts
    trigCounts - either None or a nesteddict
    """
    from gempython.utils.nesteddict import nesteddict
    if trigCounts is None:
        trigCounts = nesteddict()

    trigCounts[key]["AMC13"] = (amc13board.device.read(amc13board.device.Board.T1, "STATUS.GENERAL.L1A_COUNT_HI") << 32) | (amc13board.device.read(amc13board.device.Board.T1, "STATUS.GENERAL.L1A_COUNT_LO"))

    from gempython.tools.amc_user_functions_uhal import getL1ACount
    for slot,amc in amcs.iteritems():
        trigCounts[key]["AMC{0}".format(slot)] = getL1ACount(amc)

    return trigCounts

def startDataTaking(amc13board, outFile):
    """
    Start data taking

    amc13board - instance of the AMC13manager class
    outFile - filename to use for data taking
    """

    amc13board.startDataTaking(outFile)
    return

def stopDataTaking(amc13board,acquireTime=60):
    """
    Stop data taking after acquireTime seconds

    amc13board - instance of the AMC13manager class
    acquireTime - time in seconds to wait before stopping data taking
    """
    from time import sleep

    sleep(acquireTime)
    amc13board.stopDataTaking()
    return

if __name__ == '__main__':
    """
    Script to readout amc13
    By: Brian Dorney (brian.l.dorney@cern.ch)
    """

    # create the parser
    import argparse
    parser = argparse.ArgumentParser(description="Arguments to supply to amc13ReadOut.py")
    
    # Positional arguments
    from reg_utils.reg_interface.common.reg_xml_parser import parseInt
    parser.add_argument("shelf", type=int, help="uTCA shelf number", metavar="shelf")
    parser.add_argument("slotList", type=str, help="Comma and dash separated list of AMC slots to readout, e.g. '1,3-5,7", metavar="slotList")
    parser.add_argument("acquireTime", type=int, help="time in seconds to acquire data", metavar="acquireTime")
    parser.add_argument("filename", type=str, help="Filename to write data too", metavar="filename")
    
    # Optional arguments
    parser.add_argument("--debug", action="store_true", dest="debug",
            help="print additional debugging information")
    parserTrigGroup = parser.add_mutually_exclusive_group()
    parserTrigGroup.add_argument("--t3trig", action="store_true", dest="t3trig",
            help="Set up for using AMC13 T3 trigger input")
    parserTrigGroup.add_argument("--internal", action="store_true", dest="internal",
            help="Configure the amc13 to generate local triggers")

    args = parser.parse_args()
    options = vars(args)
    
    # Get the amc13
    import uhal
    if args.debug:
        uhal.setLogLevelTo(uhal.LogLevel.INFO)
    else:
        uhal.setLogLevelTo(uhal.LogLevel.ERROR)
    from gempython.tools.xdaq.amc13manager import AMC13manager
    amc13base  = "gem.shelf%02d.amc13"%(args.shelf)
    amc13board = AMC13manager()
    amc13board.connect(amc13base,args.debug)
    
    # Get the slots
    from gempython.tools.amc_user_functions_uhal import *
    dict_amcs = {}
    slotList = args.slotList.split(',')
    for slotRange in slotList:
        if "-" in slotRange:
            slotsInRange = [int(slot) for slot in slotRange.split("-")]
            for slot in range(min(slotsInRange),max(slotsInRange)+1):
                dict_amcs[slot] = getAMCObject(slot,shelf=args.shelf,debug=args.debug)
        else:
            dict_amcs[slotRange] = getAMCObject(int(slotRange),shelf=args.shelf,debug=args.debug)

    # Define the parallel processes
    from multiprocessing import Process
    dataTaking = Process(target=startDataTaking, args=(amc13board,args.filename))
    halt = Process(target=stopDataTaking, args=(amc13board,args.acquireTime))

    # Stop Triggers
    for slot,amc in dict_amcs.iteritems():
        blockL1A(amc)
    amc13board.device.enableLocalL1A(False)

    # Setup for data-taking and get L1A counters
    amc13board.reset()
    amc13board.configureInputs(args.slotList)
    trigCount = readAllL1ACounters(amc13board, dict_amcs, "INITIAL")

    # Enable triggers to amc's
    for slot,amc in dict_amcs.iteritems():
        enableL1A(amc)
    amc13board.device.enableLocalL1A(True)
    amc13board.configureTrigger(ena=args.internal,t3trig=args.t3trig)
    if args.internal:
        amc13board.configureCalPulse()

    # Take data
    try:
        #dataTaking.start()
        dataTaking.run()
        #halt.start()
        halt.run()
        halt.join()
        print("Data Taking has completed")
    except KeyboardInterrupt:
        amc13board.stopDataTaking()
        #halt.terminate()

        # Disable triggers & get final L1A counters
        for slot,amc in dict_amcs.iteritems():
            blockL1A(amc)
        amc13board.device.enableLocalL1A(False)
        trigCount = readAllL1ACounters(amc13board, dict_amcs, "FINAL", trigCount)

        # Calculate difference in L1A Counters
        print("| Board | L1A Count |")
        print("| :---: | :-------: |")
        for slot in trigCount["INITIAL"].keys():
            print("| {0} | {1} |".format(
                slot,
                trigCount["FINAL"][slot] - trigCount["INITIAL"][slot]))
        
        print("Data Taking has completed")
    except Exception as e:
        print("Caught Exception {0}, terminating workers".format(e))
        amc13board.stopDataTaking()
        #halt.terminate()

        # Disable triggers & get final L1A counters
        for slot,amc in dict_amcs.iteritems():
            blockL1A(amc)
        amc13board.device.enableLocalL1A(False)
        trigCount = readAllL1ACounters(amc13board, dict_amcs, "FINAL", trigCount)

        # Calculate difference in L1A Counters
        print("| Board | L1A Count |")
        print("| :---: | :-------: |")
        for slot in trigCount["INITIAL"].keys():
            print("| {0} | {1} |".format(
                slot,
                trigCount["FINAL"][slot] - trigCount["INITIAL"][slot]))
        
        print("Data Taking has completed")

    # Disable triggers & get final L1A counters
    for slot,amc in dict_amcs.iteritems():
        blockL1A(amc)
    amc13board.device.enableLocalL1A(False)
    trigCount = readAllL1ACounters(amc13board, dict_amcs, "FINAL", trigCount)

    # Calculate difference in L1A Counters
    print("| Board | L1A Count |")
    print("| :---: | :-------: |")
    for slot in trigCount["INITIAL"].keys():
        print("| {0} | {1} |".format(
            slot,
            trigCount["FINAL"][slot] - trigCount["INITIAL"][slot]))

    print("amc13ReadOut.py has exited successfully")
