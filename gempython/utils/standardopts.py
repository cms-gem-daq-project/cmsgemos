from optparse import OptionParser

parser = OptionParser()
parser.add_option("-s", "--slot", type="int", dest="slot",
                  help="slot in uTCA crate", metavar="slot", default=10)
parser.add_option("-g", "--gtx", type="int", dest="gtx",
                  help="GTX on the AMC", metavar="gtx", default=0)
parser.add_option("--shelf", type="int", dest="shelf",default=1,
		  help="uTCA shelf to access", metavar="shelf")
parser.add_option("-d", "--debug", action="store_true", dest="debug",
                  help="print extra debugging information", metavar="debug")
parser.add_option("-r", "--reset", action="store_true", dest="resetCounters",
                  help="reset link error counters", metavar="resetCounters")
parser.add_option("--errors", type="int", dest="errorRate", default=1,
                  help="calculate link error rates for N seconds", metavar="errorRate")
parser.add_option("--testbeam", action="store_true", dest="testbeam",
                  help="fixed IP address for testbeam", metavar="testbeam")
