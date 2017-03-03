from optparse import OptionParser

parser = OptionParser()
## from GLIB board info
parser.add_option("-s", "--slot", type="int", dest="slot",
                  help="slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
                  help="GTX on the AMC", metavar="gtx", default=0)
parser.add_option("-c", "--clksrc", type="string", dest="clksrc",
                  help="select the input for the XPoint1 outputs", metavar="clksrc", default="local")
parser.add_option("-r", "--reset", action="store_true", dest="resetCounters",
                  help="reset link error counters", metavar="resetCounters")
parser.add_option("-x", "--external", type="int", dest="trgSrc",
                  help="change trigger source", metavar="trgSrc")
parser.add_option("-b", "--sbitout", type="int", dest="sbitSrc",
                  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-d", "--debug", action="store_true", dest="debug",
                  help="print extra debugging information", metavar="debug")
parser.add_option("-t", "--ttc", type="int", dest="gemttc", default=2,
                  help="choose the TTC encoding (gem/csc=0, amc13=1,default=2 meaning no modification)", metavar="gemttc")
parser.add_option("-e", "--errors", type="int", dest="errorRate", default=1,
                  help="calculate link error rates for N seconds", metavar="errorRate")
parser.add_option("-u", "--user", action="store_true", dest="userOnly",
                  help="print user information only", metavar="userOnly")
parser.add_option("--testbeam", action="store_true", dest="testbeam",
                  help="fixed IP address for testbeam", metavar="testbeam")
parser.add_option("--daq_enable", type="int", dest="daq_enable",
                  help="enable daq output", metavar="daq_enable", default=-1)
parser.add_option("--rd", type="int", dest="reset_daq",
                  help="reset daq", metavar="reset_daq", default=-1)
parser.add_option("--l1a_block", action="store_true", dest="l1a_block",
                  help="Inhibit the L1As at the TTC backplane link", metavar="l1a_block")
parser.add_option("--short", action="store_true", dest="short",
                  help="Skip extended information", metavar="short")
parser.add_option("--invertTX", type="string", dest="invertTX",default="",
                  help="Flip polarity for SFPs in list", metavar="invertTX")
parser.add_option("--shelf", type="int", dest="shelf",default=1,
		  help="uTCA shelf to access", metavar="shelf")
## from OH board info
parser.add_option("-s", "--slot", type="int", dest="slot",
		  help="slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
		  help="GTX on the GLIB", metavar="gtx", default=0)
parser.add_option("-r", "--reset", action="store_true", dest="resetCounters",
		  help="reset link error counters", metavar="resetCounters")
parser.add_option("-k", "--clkSrc", type="int", dest="clkSrc",
		  help="which reference clock to use on OH", metavar="clkSrc")
parser.add_option("-x", "--extTrig", type="int", dest="trgSrc",
		  help="change trigger source", metavar="trgSrc")
parser.add_option("-b", "--sbitout", type="int", dest="sbitSrc",
		  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-d", "--debug", action="store_true", dest="debug",
		  help="print extra debugging information", metavar="debug")
parser.add_option("-l", "--localT1", action="store_true", dest="localT1",
		  help="enable the localT1 controller", metavar="localT1")
parser.add_option("-e", "--errors", type="int", dest="errorRate", default=1,
		  help="calculate link error rates for N seconds", metavar="errorRate")
parser.add_option("--testbeam", action="store_true", dest="testbeam",
		  help="fixed IP address for testbeam", metavar="testbeam")
parser.add_option("--v2b", action="store_true", dest="v2b",
		  help="Specific functionality only in v2b", metavar="v2b")
parser.add_option("--sbitmask", type="int", dest="sbitmask",default=0x0,
		  help="use s-bit mask", metavar="sbitmask")
parser.add_option("--shelf", type="int", dest="shelf",default=1,
		  help="uTCA shelf to access", metavar="shelf")

