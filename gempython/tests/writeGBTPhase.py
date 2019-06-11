#!/bin/env python

def writeSinglePhase(args):
    """
    This function writes the phase for single elink
    """
    from xhal.reg_interface_gem.core.gbt_utils_extended import setPhase
    setPhase(cardName, args.link, args.vfat, args.phase)

    from gempython.tools.vfat_user_functions_xhal import HwVFAT
    vfatBoard = HwVFAT(cardName, args.link)
    vfatBoard.parentOH.parentAMC.writeRegister("GEM_AMC.GEM_SYSTEM.CTRL.LINK_RESET", 0x1)

def writeAllPhases(args):
    """
    This function writes the phase for all elink
    """
    listOflink_phases = dict() # define dict for holding link and phases from file
    for line in open(args.gbtPhaseFile, "r"):
        if line[0].isalpha() or line[0] == "#":
            pass
        else:
            dict_key = str(line.split("\t")[0])
            if dict_key not in listOflink_phases:
                listOflink_phases[dict_key] = []
            listOflink_phases[dict_key].append(int(line.split("\t")[2]))
    for link, listOfPhases in listOflink_phases.items():
        from xhal.reg_interface_gem.core.gbt_utils_extended import setPhaseAllVFATs
        setPhaseAllVFATs(cardName, int(link), listOfPhases)

    from gempython.tools.vfat_user_functions_xhal import HwVFAT
    vfatBoard = HwVFAT(cardName, int(list(listOflink_phases)[0])) # get first key from dict
    vfatBoard.parentOH.parentAMC.writeRegister("GEM_AMC.GEM_SYSTEM.CTRL.LINK_RESET", 0x1)

if __name__ == '__main__':
    # create the parser
    import argparse
    parent_parser = argparse.ArgumentParser(add_help=False, formatter_class=argparse.RawTextHelpFormatter)

    parent_parser.add_argument("shelf", type=int, help="uTCA shelf number")
    parent_parser.add_argument("slot", type=int, help="AMC slot number in the uTCA shelf")

    parser = argparse.ArgumentParser(description="Tool for writing GBT phase for a single or all elink")
    from gempython.utils.gemlogger import colors
    subparsers = parser.add_subparsers(help="Available subcommands and their descriptions."
                                       "To view the sub menu call {0}writeGBTPhase.py COMMAND -h{1}"
                                       " e.g. {0}writeGBTPhase.py single -h{1}".format(colors.GREEN,colors.ENDC),
                                       dest='command')

    parser_single = subparsers.add_parser("single", help='write GBT phase for single VFAT', parents=[parent_parser])
    parser_all = subparsers.add_parser("all", help='write GBT phase for all VFAT', parents=[parent_parser],
                                       formatter_class=argparse.RawTextHelpFormatter)

    # create the sub-parser for writing the GBT phase for "single" elink
    parser_single.add_argument("vfat", type=int, help="VFAT number on the OH")
    parser_single.add_argument("phase", type=int, help="GBT Phase Value to Write")
    parser_single.add_argument("link", type=int, help="OH number on the AMC")
    parser_single.set_defaults(func=writeSinglePhase)

    # create the sub-parser for writing the GBT phase for "all" elink
    parser_all.add_argument("gbtPhaseFile", type=str,
                            help="File having link, vfat and phase info.\n"
                            "The input file will look like:\n"
                            "--------------------------\n"
                            "link/i:vfatN/i:GBTPhase/i:\n"
                            "4    0    7\n"
                            "4    1    9\n"
                            "4    2    13\n"
                            "--------------------------\n"
                           )
    parser_all.set_defaults(func=writeAllPhases)

    args = parser.parse_args()

    cardName = "gem-shelf%02d-amc%02d"%(args.shelf, args.slot)
    args.func(args)

    print "Goodbye"
