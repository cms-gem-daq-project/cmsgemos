#!/bin/env python

import sys, os, time
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/kernel")
from ipbus import *

####################################################

print
print "This script will test the maximum readout rate of the system."
print "Thomas Lenzi - tlenzi@ulb.ac.be"
print

####################################################

Passed = '\033[92mPassed... \033[0m'
Failed = '\033[91mFailed... \033[0m' 

def txtTitle(str):
    print '\033[1m' + str + '\033[0m'

glibIP = raw_input("> Enter the GLIB's IP address: ")
testType = raw_input("> Select the test type. 0 = no write, 1 = write to disk: ")

glib = GLIB(glibIP.strip())

####################################################

txtTitle("A. Testing the GLIB's presence")

if (glib.get("board_id") != 0): print "   Test", Passed, "could read the GLIB's firmware version"
else:
    print "   Test", Failed, "couldn't read the GLIB's firmware version"
    sys.exit()

print

####################################################

txtTitle("B. Testing the OH's presence")

glib.set("oh_trigger_source", 1)

if (glib.get("oh_trigger_source") == 1): print "   Test", Passed, "could access the OptoHybrid"
else:
    print "   Test", Failed, "couldn't access the OptoHybrid"
    sys.exit()

print

####################################################

txtTitle("C. Detecting the VFAT2s over I2C")

presentVFAT2sSingle = []
presentVFAT2sFifo = []

glib.set("ei2c_reset", 0)
glib.get("vfat2_all_chipid0")
chipIDs = glib.fifoRead("ei2c_data", 24)

for i in range(0, 24):
    if ((glib.get("vfat2_" + str(i) + "_chipid0") & 0xff) != 0): presentVFAT2sSingle.append(i)
    if ((chipIDs[i] & 0xff) != 0): presentVFAT2sFifo.append(i)

if (presentVFAT2sSingle == presentVFAT2sFifo): print "   VFAT2s detection", Passed
else: print "   VFAT2s detection", Failed

print "   Detected", str(len(presentVFAT2sSingle)), "VFAT2s:", str(presentVFAT2sSingle)
print

####################################################

txtTitle("D. Testing the rate")

glib.set("ei2c_reset", 0)
glib.set("vfat2_all_ctrl0", 0)
glib.set("vfat2_" + str(presentVFAT2sSingle[0]) + "_ctrl0", 55)

f = open('out.log', 'w')

values = [
          100, 200, 300, 400, 500, 600, 700, 800, 900, 
          1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 
          10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000,
          100000, 125000, 150000, 175000, 200000
        ]

for i in values:
    isFull = False

    glib.set("t1_reset", 1)
    glib.set("tk_data_rd", 1)
    glib.set("t1_mode", 0)
    glib.set("t1_type", 0)
    glib.set("t1_n", 0)
    glib.set("t1_interval", 40000000 / i)
    glib.set("t1_toggle", 1)

    if (testType == "0"):
        for j in range(0, 1000):
            depth = glib.get("tk_data_cnt")
            glib.fifoRead("tk_data_rd", depth)
            if (glib.get("tk_data_full") == 1): isFull = True
    elif (testType == "1"):
        for j in range(0, 1000):
            depth = glib.get("tk_data_cnt")
            if (depth > 0):
                data = glib.fifoRead("tk_data_rd", depth)
                for d in data: f.write(str("%08x"%d))
            if (glib.get("tk_data_full") == 1): isFull = True

    if (isFull): 
        print "  ", str(i), "Hz", Failed
        sys.exit()
    else: print "  ", str(i), "Hz", Passed
    time.sleep(0.01)

f.close()

glib.set("t1_reset", 1)
glib.set("vfat2_" + str(presentVFAT2sSingle[0]) + "_ctrl0", 0)
