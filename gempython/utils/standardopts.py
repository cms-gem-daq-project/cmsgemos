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
parser.add_option("--sbitout", type="int", dest="sbitSrc",
                  help="use s-bit from VFAT <num>", metavar="sbitSrc")
parser.add_option("-d", "--debug", action="store_true", dest="debug",
                  help="print extra debugging information", metavar="debug")
parser.add_option("--errors", type="int", dest="errorRate", default=1,
                  help="calculate link error rates for N seconds", metavar="errorRate")
parser.add_option("--testbeam", action="store_true", dest="testbeam",
                  help="fixed IP address for testbeam", metavar="testbeam")
parser.add_option("--shelf", type="int", dest="shelf",default=1,
		  help="uTCA shelf to access", metavar="shelf")
