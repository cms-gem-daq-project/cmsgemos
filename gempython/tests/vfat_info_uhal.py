#!/bin/env python

from gempython.tools.glib_system_info_uhal import getSystemFWVer,getSystemFWDate
from gempython.tools.vfat_user_functions_uhal import *
from gempython.tools.amc_user_functions_uhal import *
from gempython.utils.rate_calculator import errorRate

import logging
from gempython.utils.gemlogger import colors,getGEMLogger

from gempython.utils.nesteddict import nesteddict
from gempython.utils.standardopts import parser

parser.add_option("--verbose", action="store_true", dest="verbose")
parser.add_option("--enable", type="string", dest="enabledChips",
		  help="list of chips to enable, comma separated", metavar="enabledChips", default=[])

(options, args) = parser.parse_args()

gemlogger = getGEMLogger(__name__)
gemlogger.setLevel(logging.INFO)

uhal.setLogLevelTo(uhal.LogLevel.FATAL)

chips = []
if options.enabledChips:
    chips = [int(n) for n in options.enabledChips.split(",")]
    msg = "chips", chips
    gemlogger.info(msg)
    pass

amc     = getAMCObject(options.slot,options.shelf,options.debug)
ohboard = getOHObject(options.slot,options.gtx,options.shelf,options.debug)
setOHLogLevel(logging.FATAL)
setAMCLogLevel(logging.FATAL)
setVFATLogLevel(logging.FATAL)
setRegisterLogLevel(logging.FATAL)

if options.debug and False:
    print
    print "--=======================================--"
    print "  Using AMC ", amc
    print "  Using OH ", ohboard
    print "--=======================================--"
    getSystemInfo(amc)
    msg = "The nodes within GEM_AMC are:"
    gemlogger.debug(msg)
    for inode in amc.getNode("GEM_AMC").getNodes():
        msg = inode, "attributes ..."
        gemlogger.debug(msg)
        node = amc.getNode("GEM_AMC."+inode)
        msg = "Address:0x%08x"%(node.getAddress())
        gemlogger.debug(msg)
        # Bit-mask of node
        msg = "Mask:   0x%08x"%(node.getMask())
        gemlogger.debug(msg)
        # Mode enum - one of uhal.BlockReadWriteMode.SINGLE (default), INCREMENTAL and NON_INCREMENTAL,
        #  or HIERARCHICAL for top-level nodes nesting other nodes.
        msg = "Mode:", node.getMode()
        gemlogger.debug(msg)
        # One of uhal.NodePermission.READ, WRITE and READWRITE
        msg = "R/W:", node.getPermission()
        gemlogger.debug(msg)
        # In units of 32-bits. All single registers and FIFOs have default size of 1
        msg = "Size (in units of 32-bits):", node.getSize()
        gemlogger.debug(msg)
        pass
    pass

print
print "--=======================================--"
print

ebmask = 0x000000ff
vfats = []
for gebslot in range(24):
    vfats.append("slot%d"%(gebslot+1))

emptyMask = 0xFFFF
thechipid = 0x0000

print "Firmware:  %10s  %10s"%("Version","Date")
print "AMC     :  %10s  %10s"%(getSystemFWVer(amc),getSystemFWDate(amc))
print "OH      :  %10s  %10s"%(getFirmwareVersion(ohboard,options.gtx),
                               getFirmwareDateString(ohboard,options.gtx))

controls = []
chipmask = getConnectedVFATsMask(ohboard,options.gtx)
msg = "Trying to do a block read on all VFATs HW_CHIP_ID"
gemlogger.debug(msg)

chipids = getAllChipIDs(ohboard, options.gtx, chipmask,options.debug)
msg = chipids
gemlogger.debug(msg)

for chip in chips:
    msg = "enabling chip %d"%(chip)
    gemlogger.info(msg)
    setRunMode(ohboard, options.gtx, chip, True)
    pass

displayChipInfo(ohboard, options.gtx, chipids, verbose=options.verbose, debug=options.debug)

print
print "--=======================================--"
print
