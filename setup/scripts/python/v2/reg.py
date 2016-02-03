#!/bin/env python

import sys, os
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + "/kernel")
from ipbus import *

glib = GLIB()

if (sys.argv[1] == 'w' and len(sys.argv) == 4):
    print "Write to ", sys.argv[2]
    glib.set(sys.argv[2], int(sys.argv[3]))
elif (sys.argv[1] == 'w' and len(sys.argv) == 5):
    print "Write to ", sys.argv[4], " times in ", sys.argv[2]
    for i in range(0, int(sys.argv[4])):
        glib.set(sys.argv[2], int(sys.argv[3]))
elif (sys.argv[1] == 'r' and len(sys.argv) == 3):
    print "Read from ", sys.argv[2], " : ", hex(glib.get(sys.argv[2]))
elif (sys.argv[1] == 'r' and len(sys.argv) == 4):
    for i in range(0, int(sys.argv[3])):
        print "Read from ", sys.argv[2], " : ", hex(glib.get(sys.argv[2]))
else:
    print "Nothing to do..."




