#!/bin/env python

from gempython.tools.vfat_user_functions_xhal import *
from gempython.tools.amc_user_functions_uhal import *

import logging
from gempython.utils.gemlogger import colors,getGEMLogger,printGreen,printRed

import os

def setScanParamsLatency(args):
    from gempython.utils.gracefulKiller import GracefulKiller
    killer = GracefulKiller()

    global amc
    amc = getAMCObject(args.slot,args.shelf,args.d)
    rtype = amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE").read()
    rparm = amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS").read()
    amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE").write(0xf)
    amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS").write(0)
    amc.dispatch()
    printGreen("Initial: RUN_TYPE is 0x{:x}, RUN_PARAMS is 0x{:x}".format(rtype,rparm))

    if 'x' in args.latency:
        base = 16
    # wval = hex(args.latency)
    elif 'b' in args.latency:
        base = 2
        wval = bin(args.latency)
    else:
        base = 10
    try:
        wval = int(args.latency,base)
    except ValueError as e:
        print("Invalid value specified",e)
        exit(os.EX_USAGE)

    # Get vfatBoard
    cardName = "gem-shelf%02d-amc%02d"%(args.shelf,args.slot)
    vfatBoard = HwVFAT(cardName, 0, args.d)
    NOH = vfatBoard.parentOH.parentAMC.nOHs

    # Loop Over all Links
    for ohN in range(0,NOH+1):
        # Skip masked OH's        
        if( not ((args.ohMask >> ohN) & 0x1)):
            continue

        print("")
        vfatBoard.parentOH.link = ohN
        if args.d:
            print(args)    
  
        # Get VFAT Mask
        mask = vfatBoard.parentOH.getVFATMask()

        printRed("OH{0:d} ::Setting CFG_LATENCY to {1:d} (0x{1:02x})".format(ohN,wval))
        rivals = vfatBoard.readAllVFATs("CFG_LATENCY",    mask)
        perreg = "%s0x%02x%s"
        regmap = map(lambda chip: perreg%((colors.GREEN,chip&0xffff,colors.ENDC) if chip&0xffff==wval else (colors.RED,chip&0xffff,colors.ENDC) ), rivals)
        msg = "%11s::  %s"%("Before", '   '.join(map(str, regmap)))
        print(msg)

        vfatBoard.writeAllVFATs("CFG_LATENCY", wval, mask)
        rfvals = vfatBoard.readAllVFATs("CFG_LATENCY", mask)
        badreg = map(lambda isbad: 0 if  isbad&0xffff==wval else 1, rfvals)
        perreg = "%s0x%02x%s"
        regmap = map(lambda chip: perreg%((colors.GREEN,chip&0xffff,colors.ENDC) if chip&0xffff==wval else (colors.RED,chip&0xffff,colors.ENDC) ), rfvals)
        msg = "%11s::  %s"%("After", '   '.join(map(str, regmap)))
        print(msg)
        if sum(badreg) > 0:
            printRed("OH{} :: {} VFATs do not match expectation 0x{:02x}".format(ohN,sum(badreg),wval))
        
    amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE").write(0x2)
    amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS").write((wval<<16)|(rparm&0xffff))
    rtype = amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_TYPE").read()
    rparm = amc.getNode("GEM_AMC.DAQ.EXT_CONTROL.RUN_PARAMS").read()
    amc.dispatch()
    printGreen("Final: RUN_TYPE is 0x{:x}, RUN_PARAMS is 0x{:x}".format(rtype,rparm))

if __name__ == '__main__':
    from reg_utils.reg_interface.common.reg_xml_parser import parseInt
    
    import argparse
    parser = argparse.ArgumentParser()

    parser.add_argument("latency",     help="Latency to write",    type=str)
    parser.add_argument("ohMask",       help="ohMask to apply, a 1 in the n^th bit indicates the n^th OH should be considered", type=parseInt)
    parser.add_argument("-s","--slot", help="Slot number",         type=int)
    parser.add_argument("--shelf",     help="uTCA shelf number",   type=int)
    parser.add_argument("-d",          help="debug",    action='store_true')

    args = parser.parse_args()

    gemlogger = getGEMLogger(__name__)
    gemlogger.setLevel(logging.INFO)

    uhal.setLogLevelTo( uhal.LogLevel.FATAL )

    if args.d:
        print("Running checks")
    if args.slot < 0 or args.slot > 12:
        print("Invalid slot specified {}".format(args.slot))
        exit(os.EX_USAGE)

    if args.d:
        print("Executing setScanParamsLatency()")
    setScanParamsLatency(args)
