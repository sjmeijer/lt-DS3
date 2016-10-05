#!/usr/bin/python

# from sys import argv
# from sys import exit
import sys
import time
# import subprocess
import os

# os.system('source /common/majorana/scripts/setupMajorana.csh');
# os.system('cd $TMPDIR')

inputNum = int(sys.argv[1]);
print "Looking for run list number %03d..." % (inputNum)

# filename = "/global/u2/s/sjmeijer/P3KJR_silver_runlist/runlist_%02d" % (inputNum)
# filename = "/global/u2/s/sjmeijer/DS0_silver_clint/DS0_silver_%03d" % (inputNum)
# filename = '/global/project/projectdirs/majorana/users/sjmeijer/livetime/hiLo/DS0_runlist_split/DS0_runlist_%03d' % (inputNum)
filename = '/global/project/projectdirs/majorana/users/sjmeijer/livetime/DS3/runlist_m1/runlist_m1_%03d' % (inputNum)

if (os.path.isfile(filename) == False):
    print "No file exists at: %s" % (filename)
    print "Quitting..."
    sys.exit(0)

print "Found run list number %03d. Beginning to read..." % (inputNum)
f = open(filename)
line = f.readline()

candidates = []    # all runs to consider running

while(line != ""):
    value = int(line);
    candidates.append(value);
    line = f.readline();

print 'Found %d candidate runs in the file...' % (len(candidates))

toRun = []
for aRun in candidates:
    # filepath = '/global/u2/s/sjmeijer/mjd/analysis/livetime/hg-lg-missing/out/lt_hiLo_%d.root' % (aRun)
    # filepath = '/global/u2/s/sjmeijer/mjd/analysis/livetime/hg-lg-missing/out-DS0/lt_hiLo_%d.root' % (aRun)
    # if (os.path.isfile(filename) == False):
    toRun.append(aRun);
    print 'Added run %d to run list...' % (aRun)
    # else:
        # print 'Run %d already processed' % (aRun)

#expected number to read is the length of the file in lines
# exp = int(os.popen('wc -l ' + filename).read().split(' /g')[0]); # don't ask...
exp = len(toRun)
print "Will execute over %d runs" % (exp)


count = 0;
for aRun in toRun:
    cmd  = '/global/u2/s/sjmeijer/run_LT.csh %d ' % (aRun)
    print "\n"
    print cmd
    os.system(cmd)
    count = count + 1
    print count, " / ", exp
    # print "Moving root file out..."
    # os.system('mv *.root /global/u2/s/sjmeijer/mjd/analysis/livetime/hg-lg-missing/out/')
