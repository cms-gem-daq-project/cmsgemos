#!/bin/env python                                                                                                                                                                                                                       

if __name__ == '__main__':
    from gempython.tools.amc_user_functions_xhal import *
    
    from optparse import OptionParser
    
    parser = OptionParser()
    parser.add_option("-c", "--cardName", type="string", dest="cardName", default=None,
            help="hostname of the AMC you are connecting too, e.g. 'eagle64'", metavar="cardName")
    parser.add_option("-g", "--gtx", type="int", dest="gtx",
            help="GTX on the AMC", metavar="gtx", default=0)
    (options, args) = parser.parse_args()
    
    amcboard = HwAMC(options.cardName)

    mask = str(hex(amcboard.getLinkVFATMask(options.gtx))).strip('L')

    print("The VFAT Mask you should use for OH{0} is: {1}".format(options.gtx,mask))  
