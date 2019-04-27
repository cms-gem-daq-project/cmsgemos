#!/bin/env python

if __name__ == '__main__':
    # create the parser
    import argparse
    parser = argparse.ArgumentParser(description="Tool for writing GBT phase for a single elink")

    parser.add_argument("shelf",type=int,help="uTCA shelf number")
    parser.add_argument("slot",type=int,help="AMC slot number in the uTCA shelf")
    parser.add_argument("link",type=int,help="OH number on the AMC")
    parser.add_argument("vfat",type=int,help="VFAT number on the OH")
    parser.add_argument("phase",type=int,help="GBT Phase Value to Write")
    args = parser.parse_args()

    cardName = "gem-shelf%02d-amc%02d"%(args.shelf,args.slot)

    from xhal.reg_interface_gem.core.gbt_utils_extended import setPhase
    setPhase(cardName,args.link,args.vfat,args.phase)

    from gempython.tools.vfat_user_functions_xhal import HwVFAT
    vfatBoard = HwVFAT(cardName,args.link)
    vfatBoard.parentOH.parentAMC.writeRegister("GEM_AMC.GEM_SYSTEM.CTRL.LINK_RESET",0x1)
    
    print("Goodbye")
