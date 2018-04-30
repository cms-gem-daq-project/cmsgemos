#!/bin/env python

from gempython.tools.glib_system_info_uhal import getSystemFWVer,getSystemFWDate
from gempython.tools.vfat_user_functions_uhal import *
from gempython.tools.amc_user_functions_uhal import *
from gempython.utils.rate_calculator import errorRate

import logging
from gempython.utils.gemlogger import colors,getGEMLogger

from gempython.utils.nesteddict import nesteddict
from gempython.utils.standardopts import parser

parser.add_option("-z", "--sleep", action="store_true", dest="sleepAll",
		  help="set all chips into sleep mode", metavar="sleepAll")
parser.add_option("--bias", action="store_true", dest="biasAll",
		  help="set all chips with default bias parameters", metavar="biasAll")
parser.add_option("--enable", type="string", dest="enabledChips",
		  help="list of chips to enable, comma separated", metavar="enabledChips", default=[])
parser.add_option("--testi2c", type="int", dest="testi2c", default=-1,
		  help="Testing the I2C lines (select I2C line 0-5, or 6 for all", metavar="testi2c")
parser.add_option("--testsingle", action="store_true", dest="testsingle",
		  help="Testing single VFAT transactions (turn on every other VFAT)", metavar="testsingle")

(options, args) = parser.parse_args()

gemlogger = getGEMLogger(__name__)
gemlogger.setLevel(logging.INFO)

uhal.setLogLevelTo( uhal.LogLevel.FATAL )

amc     = getAMCObject(options.slot,options.shelf,options.debug)
ohboard = getOHObject(options.slot,options.gtx,options.shelf,options.debug)
setOHLogLevel(logging.INFO)
setOHLogLevel(logging.INFO)
setAMCLogLevel(logging.INFO)
setVFATLogLevel(logging.INFO)

v3regs = [
    "CFG_PULSE_STRETCH",
    "CFG_SELF_TRIGGER_MODE",
    "CFG_FP_FE",
    "CFG_RES_PRE",
    "CFG_CAP_PRE",
    "CFG_PT",
    "CFG_EN_HYST",
    "CFG_SEL_POL",
    "CFG_SEL_COMP_MODE",
    "CFG_IREF",
    "CFG_THR_ZCC_DAC",
    "CFG_THR_ARM_DAC",
    "CFG_HYST",
    "CFG_CAL_SEL_POL",
    "CFG_CAL_PHI",
    "CFG_CAL_DAC",
    "CFG_CAL_MODE",
    "CFG_CAL_FS",
    "CFG_CAL_DUR",
    "CFG_LATENCY",
    "CFG_BIAS_CFD_DAC_2",
    "CFG_BIAS_CFD_DAC_1",
    "CFG_BIAS_PRE_I_BSF",
    "CFG_BIAS_PRE_I_BIT",
    "CFG_BIAS_PRE_I_BLCC",
    "CFG_BIAS_PRE_VREF",
    "CFG_BIAS_SH_I_BFCAS",
    "CFG_BIAS_SH_I_BDIFF",
    "CFG_BIAS_SH_I_BFAMP",
    "CFG_BIAS_SD_I_BDIFF",
    "CFG_BIAS_SD_I_BSF",
    "CFG_BIAS_SD_I_BFCAS",
    "CFG_RUN",
    "HW_CHIP_ID"
]
