#!/usr/bin/python

import getopt
import httplib
import os
import sys
import time

import GEMTestbeamDAQConfig
import GEMDAQProcess


def usage():
    print """
GEMTestbeamDAQ.py [--config <configfile>] [--reconfigure] [--glib] [--optohybrid]
          -h --help:         print this message and exit
          -c --config:       path to a config file (default ./GEMTestbeamDAQ.cfg)
          -r --reconfigure:  ask for all configuration options (default takes them from configfile if it exists)
          -g --glib:         list of GLIBs to use
          -o --optohybrid:   list of OptoHybrids to use
    """


def main(argv):
    os.environ["XDAQ_ROOT"] = "/opt/xdaq"
    os.environ["XDAQ_DOCUMENT_ROOT"] = "/opt/xdaq/htdocs"
    try:
        os.environ["LD_LIBRARY_PATH"] = "/opt/xdaq/lib:"+os.environ["LD_LIBRARY_PATH"]
    except KeyError:
        os.environ["LD_LIBRARY_PATH"] = "/opt/xdaq/lib"
    try:
        os.environ["PATH"] = "/opt/xdaq/bin:"+os.environ["PATH"]
    except KeyError:
        os.environ["PATH"] = "/opt/xdaq/bin"

    configfile  = "GEMTestbeamDAQ.cfg"
    reconfigure = False
    glib        = 1
    optohybrid  = 0

    try:
        opts,args = getopt.getopt(argv,"c:rgo",["config=","reconfigure","glib","optohybrid"])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            usage()
            sys.exit()
        elif opt in ("-c", "--config"):
            configfile = arg
        elif opt in ("-r", "--reconfigure"):
            reconfigure = True
        elif opt in ("-g", "--glib"):
            glib = arg
        elif opt in ("-o", "--optohybrid"):
            optohybrid = arg

    print("Welcome to the GEM Testbeam DAQ")
    print("===============================")

    reconfigure = reconfigure or not os.path.isfile(configfile)
    GEMTestbeamDAQConfig = GEMTestbeamDAQConfig.GEMTestbeamDAQConfig(configfile,reconfigure,useDummyFerol)

    print("Starting run...")

    for xdaq in GEMTestbeamDAQConfig.xdaqProcesses:
        xdaq.create(GEMTestbeamDAQConfig.configFilePath)

    time.sleep(2)

    for xdaq in GEMTestbeamDAQConfig.xdaqProcesses[::-1]:
        xdaq.start()

    url=GEMTestbeamDAQConfig.xdaqProcesses[-1].getURL()
    print("Running. Point your browser to http://"+url)

    ans = "m"
    while ans.lower() != 'q':

        if ans.lower() == 'm':
            print("""
m   display this Menu
q   stop the run and Quit
            """)

        if ans and ans.lower()[0] == 'f':
            try:
                count = int(ans[1:])
            except (ValueError,NameError,TypeError):
                count = 1
            print("Dumping "+str(count)+" FED fragments to /tmp")
            conn = httplib.HTTPConnection(url)
            urn = GEMTestbeamDAQConfig.xdaqProcesses[-1].getURN("evb::EVM",0)
            urn += "/writeNextFragmentsToFile?count="+str(count)
            conn.request("GET",urn)

        if ans and ans.lower()[0] == 'e':
            try:
                count = int(ans[1:])
            except (ValueError,NameError,TypeError):
                count = 1
            print("Dumping "+str(count)+" events to /tmp")
            conn = httplib.HTTPConnection(url)
            urn = GEMTestbeamDAQConfig.xdaqProcesses[-1].getURN("evb::BU",0)
            urn += "/writeNextEventsToFile?count="+str(count)
            conn.request("GET",urn)

        ans = raw_input("=> ")


    for xdaq in GEMTestbeamDAQConfig.xdaqProcesses:
        xdaq.stop()


if __name__ == "__main__":
    main(sys.argv[1:])
